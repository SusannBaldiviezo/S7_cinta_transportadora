# 99_renombrar_ids.py
# Reasigna IDs de tomate: cada N fotos consecutivas (en orden por timestamp)
# = un tomate nuevo. Renombra .jpg y su .txt correspondiente.
#
# Uso:  python .\99_renombrar_ids.py --clase maduro --por-tomate 10

from pathlib import Path
import argparse
import re

RE_CAMPOS = re.compile(r"^(?P<clase>[^_]+)_t\d+_(?P<resto>.+)$")


def renombrar(clase, por_tomate, dry):
    carpeta = Path("dataset") / clase
    if not carpeta.exists():
        print(f"No existe {carpeta}")
        return

    # Ordenar por timestamp embebido en el nombre (parte final, antes de .jpg)
    jpgs = sorted(carpeta.glob("*.jpg"), key=lambda p: p.name.split("_")[-1])
    print(f"{clase}: {len(jpgs)} imagenes, agrupando de {por_tomate} en {por_tomate}")

    for i, jpg in enumerate(jpgs):
        nuevo_id = (i // por_tomate) + 1
        m = RE_CAMPOS.match(jpg.stem)
        if not m:
            print(f"  [skip] formato raro: {jpg.name}")
            continue
        nuevo_stem = f"{m['clase']}_t{nuevo_id:03d}_{m['resto']}"
        nuevo_jpg = jpg.with_name(nuevo_stem + ".jpg")
        nuevo_txt = jpg.with_name(nuevo_stem + ".txt")
        viejo_txt = jpg.with_suffix(".txt")

        if dry:
            print(f"  {jpg.name}  ->  {nuevo_jpg.name}")
        else:
            jpg.rename(nuevo_jpg)
            if viejo_txt.exists():
                viejo_txt.rename(nuevo_txt)

    if not dry:
        # Conteo final por ID
        ids = {}
        for jpg in carpeta.glob("*.jpg"):
            m = re.search(r"_t(\d+)_", jpg.name)
            if m:
                tid = int(m.group(1))
                ids[tid] = ids.get(tid, 0) + 1
        print(f"  resultado: {len(ids)} tomates -> {dict(sorted(ids.items()))}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--clase", required=True)
    ap.add_argument("--por-tomate", type=int, default=10)
    ap.add_argument("--dry", action="store_true", help="solo mostrar, no renombrar")
    args = ap.parse_args()
    renombrar(args.clase, args.por_tomate, args.dry)