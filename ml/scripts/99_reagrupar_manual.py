# 99_reagrupar_manual.py
# Revisa fotos de una clase en orden y reasigna IDs de tomate manualmente.
# Teclas:
#   s -> mismo tomate que la anterior
#   n -> nuevo tomate (incrementa ID)
#   b -> volver una foto atras (corrige error)
#   q -> salir y aplicar renombres

from pathlib import Path
import argparse
import cv2
import re

RE_CAMPOS = re.compile(r"^(?P<clase>[^_]+)_t\d+_(?P<resto>.+)$")


def reagrupar(clase):
    carpeta = Path("dataset") / clase
    jpgs = sorted(carpeta.glob("*.jpg"), key=lambda p: p.name.split("_")[-1])
    print(f"{clase}: {len(jpgs)} imagenes")
    print("[s] mismo tomate  [n] nuevo tomate  [b] atras  [q] salir y aplicar")

    asignacion = []   # lista de (jpg_path, tomate_id)
    i = 0
    tomate_actual = 1

    while i < len(jpgs):
        jpg = jpgs[i]
        img = cv2.imread(str(jpg))
        if img is None:
            i += 1
            continue
        img = cv2.resize(img, (640, 480))
        info = f"[{i+1}/{len(jpgs)}] tomate actual: {tomate_actual}  | {jpg.name}"
        cv2.putText(img, info, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
        cv2.imshow("reagrupar", img)

        k = cv2.waitKey(0) & 0xFF
        if k == ord('q'):
            break
        elif k == ord('s'):
            asignacion.append((jpg, tomate_actual))
            i += 1
        elif k == ord('n'):
            tomate_actual += 1
            asignacion.append((jpg, tomate_actual))
            i += 1
        elif k == ord('b'):
            if i > 0:
                i -= 1
                # quitar la ultima asignacion y revisar de nuevo
                if asignacion:
                    _, prev_id = asignacion.pop()
                    tomate_actual = prev_id   # mantener el id que tenia

    cv2.destroyAllWindows()

    if not asignacion:
        print("Nada que aplicar.")
        return

    # Aplicar renombres
    print(f"\nAplicando {len(asignacion)} renombres...")
    for jpg, tid in asignacion:
        m = RE_CAMPOS.match(jpg.stem)
        if not m:
            continue
        nuevo_stem = f"{m['clase']}_t{tid:03d}_{m['resto']}"
        nuevo_jpg = jpg.with_name(nuevo_stem + ".jpg")
        nuevo_txt = jpg.with_name(nuevo_stem + ".txt")
        viejo_txt = jpg.with_suffix(".txt")
        if jpg != nuevo_jpg:
            jpg.rename(nuevo_jpg)
            if viejo_txt.exists():
                viejo_txt.rename(nuevo_txt)

    # Conteo
    ids = {}
    for jpg in carpeta.glob("*.jpg"):
        m = re.search(r"_t(\d+)_", jpg.name)
        if m:
            tid = int(m.group(1))
            ids[tid] = ids.get(tid, 0) + 1
    print(f"Resultado: {len(ids)} tomates -> {dict(sorted(ids.items()))}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--clase", required=True)
    args = ap.parse_args()
    reagrupar(args.clase)