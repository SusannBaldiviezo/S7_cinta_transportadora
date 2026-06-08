# 06_auto_etiquetar.py  (v2)
# YOLO-World + fallback por contorno HSV (rescata verdes y casos raros).

from ultralytics import YOLOWorld
from pathlib import Path
import cv2

DIR_DATASET = Path("dataset")
CLASES = ["maduro", "verde", "pinton", "podrido"]
PROMPTS = ["tomato", "green tomato", "unripe tomato", "red fruit", "round fruit"]
CONF_MIN = 0.05

# Fallback HSV: saturacion > umbral = no es fondo mate
SAT_MIN = 40
AREA_MIN_FRAC = 0.02   # el blob debe ocupar al menos 2% de la imagen


def bbox_por_contorno(img):
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv[:, :, 1], SAT_MIN, 255)
    k = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (7, 7))
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, k)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, k)
    cnts, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not cnts:
        return None
    cnt = max(cnts, key=cv2.contourArea)
    h, w = img.shape[:2]
    if cv2.contourArea(cnt) < AREA_MIN_FRAC * h * w:
        return None
    x, y, bw, bh = cv2.boundingRect(cnt)
    return (x, y, x + bw, y + bh)


modelo = YOLOWorld("yolov8s-world.pt")
modelo.set_classes(PROMPTS)

total_yolo, total_fb, total_sin = 0, 0, 0

for clase in CLASES:
    carpeta = DIR_DATASET / clase
    if not carpeta.exists():
        print(f"[skip] {carpeta} no existe")
        continue
    idx_clase = CLASES.index(clase)
    imgs = list(carpeta.glob("*.jpg"))
    print(f"\n=== {clase} ({len(imgs)} imagenes, clase id {idx_clase}) ===")

    for img_path in imgs:
        img = cv2.imread(str(img_path))
        if img is None:
            continue
        h, w = img.shape[:2]

        res = modelo.predict(img, conf=CONF_MIN, verbose=False)[0]
        bbox, origen = None, None
        if len(res.boxes) > 0:
            best = res.boxes.conf.cpu().numpy().argmax()
            bbox = tuple(res.boxes.xyxy[best].cpu().numpy())
            origen = "yolo"
        else:
            bbox = bbox_por_contorno(img)
            origen = "fallback"

        if bbox is None:
            total_sin += 1
            print(f"  [sin bbox] {img_path.name}")
            continue

        x1, y1, x2, y2 = bbox
        cx, cy = ((x1 + x2) / 2) / w, ((y1 + y2) / 2) / h
        bw, bh = (x2 - x1) / w, (y2 - y1) / h
        with open(img_path.with_suffix(".txt"), "w") as f:
            f.write(f"{idx_clase} {cx:.6f} {cy:.6f} {bw:.6f} {bh:.6f}\n")

        if origen == "yolo":
            total_yolo += 1
        else:
            total_fb += 1
            print(f"  [fallback] {img_path.name}")

print(f"\nResumen: YOLO-World {total_yolo}, fallback {total_fb}, sin bbox {total_sin}")