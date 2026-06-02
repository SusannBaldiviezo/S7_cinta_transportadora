# Módulo de Visión Artificial — OpenCV

> **Estado:** En desarrollo — Semana 4

## Objetivo

Script Python con OpenCV que:
- Captura imágenes desde 2 cámaras USB
- Analiza color y textura para detectar deterioro en frutas
- Publica el veredicto en MQTT (`fruta/estado = "aceptada" | "rechazada"`)

## Arquitectura

```
[Cámara 1 — ángulo frontal]    [Cámara 2 — ángulo lateral]
          │                              │
          └──────────────┬───────────────┘
                         │ frames simultáneos
                         ▼
               [capture.py — cv2.VideoCapture]
                         │
                         ▼
               [clasificador.py — análisis HSV]
               ┌─────────────────────────────┐
               │  1. Convertir BGR → HSV      │
               │  2. Detectar zonas oscuras   │
               │  3. Calcular porcentaje area │
               │  4. Umbral → decisión        │
               └─────────────────────────────┘
                         │
                         ▼
               [mqtt_publisher.py]
               fruta/estado = "aceptada" | "rechazada"
```

## Instalación

```bash
pip install -r requirements/ml_requirements.txt
```

## Uso

```bash
# Modo demo (2 cámaras reales)
python rpi/src/vision/clasificador.py

# Modo simulador (sin cámaras)
python scripts/simulador_vision.py
```

## Parámetros de clasificación

Los parámetros se configuran en `rpi/src/vision/config_vision.yaml` (pendiente):

```yaml
camaras:
  cam1_id: 0
  cam2_id: 1
  resolucion: [640, 480]

clasificacion:
  umbral_area_dañada: 0.15    # 15% del área total
  min_area_mancha: 500        # píxeles mínimos para considerar mancha
  rango_hsv_dañado:
    lower: [0, 0, 0]          # negro/café
    upper: [30, 255, 80]

mqtt:
  broker: "192.168.4.1"
  port: 1883
  topic_veredicto: "fruta/estado"
```

## Archivos (pendiente de implementación)

```
rpi/src/vision/
├── clasificador.py      ← script principal
├── capture.py           ← captura de cámaras
├── analisis_color.py    ← análisis HSV + detección de zonas
└── config_vision.yaml   ← parámetros
```
