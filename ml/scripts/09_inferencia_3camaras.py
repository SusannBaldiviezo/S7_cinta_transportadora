# 09_inferencia_3camaras.py
# Inferencia en vivo con las 3 camaras + dibujo de bbox y clase.
# Prueba de pipeline: ver detecciones en tiempo real. La logica de episodios
# por tomate / override de podrido / voto mayoritario se agrega despues.

import cv2
from ultralytics import YOLO

PESOS = "runs/detect/tomate_prueba/weights/best.pt"
BACKEND = cv2.CAP_MSMF
ANCHO, ALTO = 640, 480
CONF_MIN = 0.30

# Mismo dict que el script de captura
CAMARAS = {
    "cam1_cenital":   2,
    "cam2_lateral":   0,
    "cam3_posterior": 1,
}

# Colores BGR por clase
COLORES = {
    "maduro":  (0, 0, 255),     # rojo
    "verde":   (0, 255, 0),     # verde
    "pinton":  (0, 165, 255),   # naranja
    "podrido": (128, 0, 128),   # purpura
}


def abrir(idx):
    cap = cv2.VideoCapture(idx, BACKEND)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, ANCHO)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, ALTO)
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    return cap


def leer(cap):
    ok, frame = cap.read()
    if not ok:
        return None
    return cv2.resize(frame, (ANCHO, ALTO))


def dibujar(frame, resultado, nombres):
    for box in resultado.boxes:
        cls_id = int(box.cls[0])
        conf = float(box.conf[0])
        if conf < CONF_MIN:
            continue
        clase = nombres[cls_id]
        x1, y1, x2, y2 = map(int, box.xyxy[0])
        color = COLORES.get(clase, (255, 255, 255))
        cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
        etiqueta = f"{clase} {conf:.2f}"
        cv2.putText(frame, etiqueta, (x1, max(y1 - 8, 15)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
    return frame


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
        raise SystemExit
    caps[nombre] = cap

print("Inferencia 3 camaras. [q] sale.")

while True:
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

    # Inferencia batch: las 3 a la vez (mas rapido que una por una)
    lista = list(frames.values())
    resultados = modelo.predict(lista, conf=CONF_MIN, verbose=False)

    anotados = []
    for (nombre, frame), res in zip(frames.items(), resultados):
        frame = dibujar(frame, res, nombres)
        cv2.putText(frame, nombre, (10, 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        anotados.append(frame)

    vista = cv2.hconcat(anotados)
    vista = cv2.resize(vista, None, fx=0.6, fy=0.6)
    cv2.imshow("inferencia 3 camaras", vista)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

for cap in caps.values():
    cap.release()
cv2.destroyAllWindows()