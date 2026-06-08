# 07_split_por_tomate.py
# Split por ID de tomate. El mismo tomate fisico NUNCA cae en train Y val.
# Nombre esperado: clase_tNNN_camara_timestamp.jpg

from pathlib import Path
from collections import defaultdict
import re
import shutil

ORIGEN = Path("dataset")
DESTINO = Path("dataset_yolo")
CLASES = ["maduro", "verde", "pinton", "podrido"]
FRAC_VAL = 0.20

RE_TOMATE = re.compile(r"_t(\d+)_")

if DESTINO.exists():
    shutil.rmtree(DESTINO)
for split in ["train", "val"]:
    (DESTINO / split / "images").mkdir(parents=True)
    (DESTINO / split / "labels").mkdir(parents=True)

total = {"train": 0, "val": 0}

for clase in CLASES:
    carpeta = ORIGEN / clase
    if not carpeta.exists():
        continue

    # Agrupar imagenes por ID de tomate
    grupos = defaultdict(list)
    for jpg in carpeta.glob("*.jpg"):
        if not jpg.with_suffix(".txt").exists():
            continue
        m = RE_TOMATE.search(jpg.name)
        if not m:
            print(f"  [skip] sin ID: {jpg.name}")
            continue
        grupos[int(m.group(1))].append(jpg)

    ids = sorted(grupos.keys())
    n_val = max(1, int(round(len(ids) * FRAC_VAL)))
    ids_val = set(ids[-n_val:])
    ids_train = set(ids[:-n_val])

    n_train = sum(len(grupos[i]) for i in ids_train)
    n_val_img = sum(len(grupos[i]) for i in ids_val)

    print(f"{clase}: {len(ids)} tomates")
    print(f"  train: tomates {sorted(ids_train)} ({n_train} imgs)")
    print(f"  val:   tomates {sorted(ids_val)} ({n_val_img} imgs)")

    for tid, jpgs in grupos.items():
        split = "val" if tid in ids_val else "train"
        for jpg in jpgs:
            shutil.copy(jpg, DESTINO / split / "images" / jpg.name)
            shutil.copy(jpg.with_suffix(".txt"),
                       DESTINO / split / "labels" / (jpg.stem + ".txt"))
            total[split] += 1

yaml = f"""path: {DESTINO.resolve().as_posix()}
train: train/images
val: val/images

names:
  0: maduro
  1: verde
  2: pinton
  3: podrido
"""
(DESTINO / "data.yaml").write_text(yaml)
print(f"\nTotal: train={total['train']}, val={total['val']}")