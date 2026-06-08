# 05_captura_dataset.py  (v2)
# Captura de fotos para el dataset de deteccion (yolo detect).
# Una clase por sesion: el auto-etiquetado saca la clase del nombre de carpeta.
# El nombre de archivo lleva el ID de tomate -> permite split POR TOMATE despues.
#
# Uso:
#   1) Descubrir indices de camara (primera vez o si cambiaron):
#        python 05_captura_dataset.py --probe
#
#   2) Capturar (cargar SOLO tomates de esa clase):
#        python 05_captura_dataset.py --clase verde
#        python 05_captura_dataset.py --clase verde --inicio 41   # seguir numerando
#
# Teclas en captura:
#   n  -> siguiente tomate (sube el ID; hacelo ANTES de cada tomate nuevo)
#   1  -> guarda solo cam1_cenital
#   2  -> guarda solo cam2_lateral
#   3  -> guarda solo cam3_posterior
#   ESPACIO -> guarda las 3 (para pasada con cinta en movimiento)
#   q  -> salir

import cv2
import os
import time
import argparse

# --- CONFIG ---------------------------------------------------------------
BACKEND = cv2.CAP_MSMF          # decisiones.md: DSHOW solo veia 1 camara
ANCHO, ALTO = 640, 480          # subir a 1280,720 si las camaras lo soportan (mejor detalle de podrido)
DIR_DATASET = "dataset"

# Indices fisicos. AJUSTAR tras correr --probe. MSMF baraja indices al reconectar USB.
CAMARAS = {
    "cam1_cenital":   1,
    "cam2_lateral":   0,
    "cam3_posterior": 2,
}

CLASES_VALIDAS = {"maduro", "verde", "pinton", "podrido"}   # ASCII a proposito
# --------------------------------------------------------------------------

# mapeo tecla -> nombre de camara
TECLA_CAM = {ord('1'): "cam1_cenital", ord('2'): "cam2_lateral", ord('3'): "cam3_posterior"}


def abrir_camara(idx):
    cap = cv2.VideoCapture(idx, BACKEND)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, ANCHO)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, ALTO)
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    return cap


def leer_frame(cap):
    ok, frame = cap.read()
    if not ok:
        return None
    return cv2.resize(frame, (ANCHO, ALTO))   # forzar uniforme siempre


def probe(max_idx=6):
    print("Escaneando indices 0..{} ...".format(max_idx - 1))
    abiertas = []
    for idx in range(max_idx):
        cap = abrir_camara(idx)
        if leer_frame(cap) is not None:
            print("  indice {}: OK".format(idx))
            abiertas.append((idx, cap))
        else:
            cap.release()
    if not abiertas:
        print("No se abrio ninguna camara. Revisa conexiones USB.")
        return
    print("\nIdentifica cada camara y escribe los indices en CAMARAS. [q] sale.")
    while True:
        for idx, cap in abiertas:
            frame = leer_frame(cap)
            if frame is None:
                continue
            cv2.putText(frame, "indice {}".format(idx), (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 0, 255), 2)
            cv2.imshow("probe idx {}".format(idx), frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    for _, cap in abiertas:
        cap.release()
    cv2.destroyAllWindows()


def guardar(carpeta, clase, tomate_id, nombre_cam, frame, conteos):
    ts = time.strftime("%Y%m%d_%H%M%S")
    fname = "{}_t{:03d}_{}_{}.jpg".format(clase, tomate_id, nombre_cam, ts)
    cv2.imwrite(os.path.join(carpeta, fname), frame)
    conteos[nombre_cam] = conteos.get(nombre_cam, 0) + 1
    print("  [tomate {:03d}] {} guardada (total {})".format(tomate_id, nombre_cam, conteos[nombre_cam]))


def capturar(clase, inicio):
    if clase not in CLASES_VALIDAS:
        print("Clase invalida. Usar: {}".format(sorted(CLASES_VALIDAS)))
        return

    carpeta = os.path.join(DIR_DATASET, clase)
    os.makedirs(carpeta, exist_ok=True)

    caps = {}
    for nombre, idx in CAMARAS.items():
        cap = abrir_camara(idx)
        ok, raw = cap.read()
        if not ok:
            print("No se pudo abrir {} (indice {}). Corre --probe.".format(nombre, idx))
            for c in caps.values():
                c.release()
            cap.release()
            return
        print("  {} (idx {}): resolucion nativa {}x{}".format(nombre, idx, raw.shape[1], raw.shape[0]))
        caps[nombre] = cap

    tomate_id = inicio
    conteos = {n: 0 for n in CAMARAS}

    print("Clase: {} | carpeta: {} | tomate inicial: {:03d}".format(clase, carpeta, tomate_id))
    print("[n] sig.tomate  [1/2/3] guardar cam  [ESPACIO] las 3  [q] salir")

    while True:
        frames = {}
        falla = False
        for nombre, cap in caps.items():
            f = leer_frame(cap)
            if f is None:
                falla = True
                break
            frames[nombre] = f
        if falla:
            continue

        vista = cv2.hconcat(list(frames.values()))
        # solo para mostrar; lo guardado sigue siendo 640x480
        vista = cv2.resize(vista, None, fx=0.6, fy=0.6)
        for i, nombre in enumerate(frames.keys()):
            cv2.putText(vista, nombre, (i * ANCHO + 10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        hud = "{} | tomate {:03d} | c1={} c2={} c3={}".format(
            clase, tomate_id, conteos["cam1_cenital"], conteos["cam2_lateral"], conteos["cam3_posterior"])
        cv2.putText(vista, hud, (10, ALTO - 15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
        cv2.imshow("captura dataset", vista)

        k = cv2.waitKey(1) & 0xFF
        if k == ord('q'):
            break
        elif k == ord('n'):
            tomate_id += 1
            print("--> tomate {:03d}".format(tomate_id))
        elif k in TECLA_CAM:
            nombre_cam = TECLA_CAM[k]
            guardar(carpeta, clase, tomate_id, nombre_cam, frames[nombre_cam], conteos)
        elif k == ord(' '):
            for nombre_cam, frame in frames.items():
                guardar(carpeta, clase, tomate_id, nombre_cam, frame, conteos)

    for cap in caps.values():
        cap.release()
    cv2.destroyAllWindows()
    total = sum(conteos.values())
    print("Fin. Total imagenes sesion: {} | ultimo tomate: {:03d}".format(total, tomate_id))


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Captura de dataset 3 camaras (v2)")
    ap.add_argument("--clase", help="maduro / verde / pinton / podrido")
    ap.add_argument("--inicio", type=int, default=1, help="ID del primer tomate de la sesion")
    ap.add_argument("--probe", action="store_true", help="solo descubrir indices de camara")
    args = ap.parse_args()

    if args.probe:
        probe()
    elif args.clase:
        capturar(args.clase, args.inicio)
    else:
        ap.print_help()