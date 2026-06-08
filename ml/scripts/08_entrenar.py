# 08_entrenar.py
# Entrenamiento de prueba: pocas epochs, modelo nano.
# Mañana con datos reales: yolo11s, 120 epochs, patience=25.

from ultralytics import YOLO

modelo = YOLO("yolo11n.pt")

modelo.train(
    data="dataset_yolo/data.yaml",
    epochs=30,
    imgsz=640,
    batch=8,
    patience=10,
    name="tomate_real",
    # color aug bajita: el color ES la etiqueta, no la queremos difuminar
    hsv_h=0.005,
    hsv_s=0.3,
    hsv_v=0.2,
    # geometricas: si dejamos normales
    fliplr=0.5,
    flipud=0.5,
    scale=0.3,
    translate=0.1,
    close_mosaic=5,
)