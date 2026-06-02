# Módulo de Visión Artificial

> **Estado:** En desarrollo — Semana 4

## Objetivo

Detectar frutas en estado de deterioro o descomposición mediante análisis de color y textura con OpenCV desde 2 cámaras USB.

## Pipeline de procesamiento

```
Cámara 1 + Cámara 2
       │
       ▼
capture.py (cv2.VideoCapture)
       │
       ▼
analisis_color.py
  - Convertir BGR → HSV
  - Detectar zonas oscuras/manchas
  - Calcular porcentaje de área dañada
  - Decisión: "aceptada" | "rechazada"
       │
       ▼
MQTT → fruta/estado
```

## Instalación

```bash
pip install -r ../requirements/ml_requirements.txt
```

## Estructura (pendiente)

```
ml/
├── README.md
├── notebooks/           ← análisis exploratorio
├── scripts/             ← entrenamiento y evaluación
└── models/              ← modelos entrenados (en .gitignore)
```

## Parámetros clave

- **Umbral de área dañada:** 15% de la superficie visible
- **Rango HSV de manchas:** [0,0,0] → [30,255,80] (zonas oscuras/café)
- **Tiempo máximo de procesamiento:** 2 segundos por fruta (RF-01)
