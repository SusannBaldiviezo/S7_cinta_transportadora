# 10_inferencia_episodios.py
# Paso A: logica de episodios + decision por tomate, imprimiendo por consola.
# Sin MQTT todavia (eso es Paso B).
#
# Maquina de estados por estacion:
#   VACIA -> OCUPADA: bbox detectada N_ESTABILIDAD frames seguidos
#   OCUPADA -> VACIA: sin bbox N_HUECO frames seguidos
#   Durante OCUPADA: acumula detecciones (clase, conf, cam)
#
# Asociacion E1 -> E2:
#   E1 cierra -> push a cola FIFO
#   E2 abre   -> pop oldest de cola (mismo tomate)
#   E2 cierra -> decidir clase final
#   Si una entrada queda en cola mas de VIAJE_MAX -> "PERDIDO"
#
# Decision:
#   Por camara, voto = clase con mayor suma de confianzas (requiere >= N_MIN_VOTO frames)
#   Override: si CUALQUIER camara vota podrido -> podrido
#   Si no: mayoria entre las camaras

import cv2
import time
from collections import defaultdict, deque
from ultralytics import YOLO
import base64
import requests
import threading
import queue

# ════════════════════════════════════════════════════════
# INTEGRACIÓN MQTT — agregar después de tus imports
# ════════════════════════════════════════════════════════
import paho.mqtt.client as mqtt

MQTT_BROKER = "192.168.4.1"
MQTT_PORT   = 1883

# Inicializar cliente compatible con cualquier versión de paho
try:
    if hasattr(mqtt, "CallbackAPIVersion"):
        _mqtt = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id="vision_ia")
    else:
        _mqtt = mqtt.Client(client_id="vision_ia")

    _mqtt.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
    _mqtt.loop_start()
    print("✅ MQTT conectado al broker")
except Exception as e:
    print(f"⚠️  MQTT no disponible: {e}")
    _mqtt = None
# ════════════════════════════════════════════════════════
# INTEGRACIÓN HTTP (API) — Envío de Frames en vivo
# ════════════════════════════════════════════════════════
API_FRAMES = "http://localhost:8080/api/camaras/frame"
cola_frames = queue.Queue(maxsize=10)  # Cola para no saturar la red

def worker_enviar_frames():
    """Hilo que toma frames de la cola y los manda a FastAPI sin bloquear a YOLO"""
    while True:
        datos = cola_frames.get()
        if datos is None: 
            break
        try:
            # Enviamos el POST con un timeout corto para que no se congele
            requests.post(API_FRAMES, json=datos, timeout=0.3)
        except:
            pass # Si el backend no responde, ignoramos para no saturar la consola

# Iniciar el hilo en segundo plano
hilo_frames = threading.Thread(target=worker_enviar_frames, daemon=True)
hilo_frames.start()
print("✅ Stream de video HTTP activado")
# ════════════════════════════════════════════════════════
# Mapeo de estados YOLO → backend
_MAPA = {
    "maduro":       "aceptada",
    "podrido":      "podrida",
    "pinton":       "pinton",
    "verde":        "verde",
    "perdido":      "aceptada",
    "desconocido":  "aceptada",
}

def publicar_veredicto(resultado_yolo: str):
    """Mapea el resultado de YOLO y lo publica en MQTT."""
    if _mqtt is None:
        return
    estado = _MAPA.get(resultado_yolo.lower().strip(), "aceptada")
    try:
        _mqtt.publish("fruta/estado", estado, qos=1)
        print(f"  📡 MQTT → fruta/estado = {estado}  (yolo: {resultado_yolo})")
    except Exception as e:
        print(f"  ❌ Error MQTT: {e}")
# ════════════════════════════════════════════════════════


PESOS = "runs/detect/tomate_real/weights/best.pt"
BACKEND = cv2.CAP_DSHOW
ANCHO, ALTO = 640, 480
CONF_MIN = 0.30

CAMARAS = {
    "cam1_cenital":   2,
    "cam2_lateral":   0,
    "cam3_posterior": 1,
}

# --- Parametros tuneables ---
N_ESTABILIDAD = 3      # frames con bbox para abrir
N_HUECO       = 5      # frames sin bbox para cerrar
VIAJE_MIN     = 1.0    # s, delta minimo E1cerro -> E2abre
VIAJE_MAX     = 8.0    # s, mas alla -> tomate perdido
N_MIN_VOTO    = 3      # detecciones minimas para que una cam vote

COLORES = {
    "maduro":  (0, 0, 255),
    "verde":   (0, 255, 0),
    "pinton":  (0, 165, 255),
    "podrido": (128, 0, 128),
}


class Estacion:
    def __init__(self, nombre):
        self.nombre = nombre
        self.estado = "VACIA"
        self.cont_presencia = 0
        self.cont_ausencia = 0
        self.detecciones = []   # (clase, conf, cam) acumuladas durante OCUPADA

    def actualizar(self, hay_bbox, dets_nuevas):
        if hay_bbox:
            self.cont_ausencia = 0
            self.cont_presencia += 1
            if self.estado == "VACIA" and self.cont_presencia >= N_ESTABILIDAD:
                self.estado = "OCUPADA"
                self.detecciones = list(dets_nuevas)
                return "abrio"
            elif self.estado == "OCUPADA":
                self.detecciones.extend(dets_nuevas)
        else:
            self.cont_presencia = 0
            self.cont_ausencia += 1
            if self.estado == "OCUPADA" and self.cont_ausencia >= N_HUECO:
                self.estado = "VACIA"
                return "cerro"
        return None


def decidir(detecciones):
    """Override podrido + voto mayoritario por camara."""
    por_cam = defaultdict(list)
    for clase, conf, cam in detecciones:
        por_cam[cam].append((clase, conf))

    votos = {}
    for cam, dets in por_cam.items():
        conteo = defaultdict(lambda: [0, 0.0])    # clase -> [frames, suma_conf]
        for clase, conf in dets:
            conteo[clase][0] += 1
            conteo[clase][1] += conf
        validas = [(c, f, s) for c, (f, s) in conteo.items() if f >= N_MIN_VOTO]
        if validas:
            votos[cam] = max(validas, key=lambda x: x[2])[0]

    if not votos:
        return {"clase": "desconocido", "razon": "sin detecciones suficientes", "votos": votos}

    if "podrido" in votos.values():
        return {"clase": "podrido", "razon": "override podrido", "votos": votos}

    conteo_clases = defaultdict(int)
    for c in votos.values():
        conteo_clases[c] += 1
    clase_final = max(conteo_clases.items(), key=lambda x: x[1])[0]
    return {"clase": clase_final, "razon": "mayoria", "votos": votos}


def abrir(idx):
    cap = cv2.VideoCapture(idx, BACKEND)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, ANCHO)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, ALTO)
# --- NUEVO: Limitar FPS por hardware para no saturar el USB ---
    cap.set(cv2.CAP_PROP_FPS, 15)

    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    return cap


def leer(cap):
    ok, frame = cap.read()
    if not ok:
        return None
    return cv2.resize(frame, (ANCHO, ALTO))


def extraer_detecciones(resultado, nombre_cam, nombres):
    dets = []
    for box in resultado.boxes:
        conf = float(box.conf[0])
        if conf < CONF_MIN:
            continue
        clase = nombres[int(box.cls[0])]
        dets.append((clase, conf, nombre_cam))
    return dets


def dibujar(frame, resultado, nombres):
    for box in resultado.boxes:
        conf = float(box.conf[0])
        if conf < CONF_MIN:
            continue
        clase = nombres[int(box.cls[0])]
        x1, y1, x2, y2 = map(int, box.xyxy[0])
        color = COLORES.get(clase, (255, 255, 255))
        cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
        cv2.putText(frame, f"{clase} {conf:.2f}", (x1, max(y1 - 8, 15)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
    return frame


def main():
    modelo = YOLO(PESOS)
    nombres = modelo.names

    caps = {}
    for nombre, idx in CAMARAS.items():
        cap = abrir(idx)
        if leer(cap) is None:
            print(f"No se pudo abrir {nombre} (idx {idx})")
            for c in caps.values():
                c.release()
            cap.release()
            return
        caps[nombre] = cap

    e1 = Estacion("E1")
    e2 = Estacion("E2")
    cola = deque()                # episodios E1 cerrados esperando E2
    tomate_en_e2 = None
    tomate_global_id = 0
    ultima_decision = None
    ts_ultima = 0

    print(f"Inferencia con episodios. Pesos: {PESOS}")
    print("[q] sale\n")

    while True:
        ahora = time.time()

        frames = {}
        falla = False
        for nombre, cap in caps.items():
            f = leer(cap)
            if f is None:
                falla = True
                break
            frames[nombre] = f
        if falla:
            continue

        resultados = modelo.predict(list(frames.values()), conf=CONF_MIN, verbose=False)
        det_por_cam = {}
        for (nombre, _), res in zip(frames.items(), resultados):
            det_por_cam[nombre] = extraer_detecciones(res, nombre, nombres)

        # E1 = cam1 + cam2 (instante simultaneo, mismo tomate fisico)
        dets_e1 = det_por_cam["cam1_cenital"] + det_por_cam["cam2_lateral"]
        accion_e1 = e1.actualizar(len(dets_e1) > 0, dets_e1)

        # E2 = cam3
        dets_e2 = det_por_cam["cam3_posterior"]
        accion_e2 = e2.actualizar(len(dets_e2) > 0, dets_e2)

        # E1 cerro -> a la cola
        if accion_e1 == "cerro":
            tomate_global_id += 1
            cola.append({
                "id": tomate_global_id,
                "ts_cierre_e1": ahora,
                "detecciones_e1": list(e1.detecciones),
            })
            print(f"[E1 cerro] tomate {tomate_global_id:03d}, "
                  f"{len(e1.detecciones)} detecciones, cola={len(cola)}")

        # Limpiar perdidos
        while cola and ahora - cola[0]["ts_cierre_e1"] > VIAJE_MAX:
            perdido = cola.popleft()
            print(f"[PERDIDO] tomate {perdido['id']:03d} - E2 nunca lo vio")
            publicar_veredicto("perdido")  # ← AGREGAR ESTA LÍNEA

        # E2 abre -> asociar
        if accion_e2 == "abrio":
            if cola:
                tomate_en_e2 = cola.popleft()
                delta = ahora - tomate_en_e2["ts_cierre_e1"]
                marca = " (rapido!)" if delta < VIAJE_MIN else ""
                print(f"[E2 abrio] tomate {tomate_en_e2['id']:03d}, delta {delta:.1f}s{marca}")
            else:
                tomate_global_id += 1
                tomate_en_e2 = {"id": tomate_global_id, "detecciones_e1": []}
                print(f"[E2 abrio] tomate {tomate_global_id:03d} HUERFANO (no hubo E1)")

# E2 cierra -> decision
        if accion_e2 == "cerro" and tomate_en_e2 is not None:
            todas = tomate_en_e2["detecciones_e1"] + e2.detecciones
            decision = decidir(todas)
            print(f"\n>>> TOMATE {tomate_en_e2['id']:03d}: {decision['clase'].upper()} "
                  f"({decision['razon']})")
            
            print(f"    votos por camara: {decision['votos']}")
            print(f"    total detecciones: {len(todas)}\n")
            publicar_veredicto(decision['clase'])  # ← AGREGAR ESTA LÍNEA
            ultima_decision = (tomate_en_e2["id"], decision["clase"])
            ts_ultima = ahora
            tomate_en_e2 = None

       # --- visualizacion ---
        anotados = []
        for (nombre, frame), res in zip(frames.items(), resultados):
            # 1. Dibuja las cajas de YOLO en el frame
            frame = dibujar(frame, res, nombres)
            cv2.putText(frame, nombre, (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            anotados.append(frame)

            # --- NUEVO: Enviar este frame al Dashboard ---
            try:
                # Reducimos la resolución un poco para que vuele por la red
                frame_web = cv2.resize(frame, (320, 240))
                # Comprimimos a JPG con calidad media (50%)
                _, buffer = cv2.imencode('.jpg', frame_web, [cv2.IMWRITE_JPEG_QUALITY, 50])
                # Convertimos a Base64
                b64_str = "data:image/jpeg;base64," + base64.b64encode(buffer).decode('utf-8')
                
                # Lo metemos a la cola si hay espacio
                if not cola_frames.full():
                    cola_frames.put({
                        "nombre": nombre, 
                        "frame": b64_str, 
                        "latencia_ms": int((time.time() - ahora) * 1000)
                    })
            except Exception as e:
                pass

        # --- MANTENEMOS TU VENTANA LOCAL Y LOS CONTROLES ---
        vista = cv2.hconcat(anotados)

        estado_txt = (f"E1:{e1.estado}  E2:{e2.estado}  cola:{len(cola)}  "
                      f"total:{tomate_global_id}")
        cv2.putText(vista, estado_txt, (10, ALTO - 15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)

        if ultima_decision and ahora - ts_ultima < 5.0:
            txt = f"Ultimo: tomate {ultima_decision[0]:03d} = {ultima_decision[1].upper()}"
            color = COLORES.get(ultima_decision[1], (255, 255, 255))
            cv2.putText(vista, txt, (10, 55),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.9, color, 2)

        vista = cv2.resize(vista, None, fx=0.6, fy=0.6)
        cv2.imshow("inferencia con episodios", vista)

        # Si presionas 'q' en la ventana local, se rompe el ciclo
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # --- CIERRE SEGURO (Fuera del ciclo while) ---
    for cap in caps.values():
        cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()