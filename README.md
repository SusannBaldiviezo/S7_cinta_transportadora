# Sistema Automático de Clasificación de Frutas

**Universidad:** Universidad Católica Boliviana "San Pablo"
**Materia:** Sistemas Embebidos II
**Docente:** Msc. Ing. Alan Cornejo Q.
**Semestre:** 1/2026

**Estudiantes:**
- Susann Baldiviezo C.
- Alejandro Bejarano R.
- Florencia Frigerio A.
- Benjamín Soruco G.

---

## Descripción

Sistema automático de clasificación de frutas mediante visión artificial y ESP32. Detecta frutas en estado de descomposición a través de dos cámaras USB y OpenCV, activa un cilindro neumático para expulsarlas de la banda transportadora, y permite al operario monitorear y controlar el proceso desde una consola portátil inalámbrica.

## Arquitectura general

```
[2 Cámaras USB]
      │
      ▼
[PC — Python + OpenCV]  ──────────────────────────────────────────┐
      │  publica fruta/estado                                      │
      ▼                                                            │
[Raspberry Pi — Mosquitto MQTT]  (AP: CintaTransportadora)        │
      │  distribuye                                                │
      ├────────────────────────┐                                   │
      ▼                        ▼                                   │
[ESP32 Motor]          [ESP32 Consola]   ◄── operario             │
   BTS7960                LCD 16x2                                  │
   Banda DC               4 botones                                │
   Válvula solenoide      LED indicador                            │
      │                        │                                   │
      ▼                        ▼                                   │
[Motor 12V]          [Pantalla de estado]    ──────────────────────┘
[Cilindro neumático]
```

## Estructura del repositorio

```
git/
├── README.md              ← este archivo
├── ARCHITECTURE.md        ← diagrama detallado del sistema
├── SETUP.md               ← cómo levantar el proyecto de cero
├── .gitignore
│
├── docs/                  ← documentación completa
├── esp32/                 ← firmware embebido (motor + consola)
├── hardware/              ← conexiones, pinouts, datasheets
├── ml/                    ← módulo de visión artificial (OpenCV/YOLO)
├── rpi/                   ← scripts Python para la Raspberry Pi
├── scripts/               ← herramientas y simuladores
├── config/                ← configuración de servicios (Mosquitto, etc.)
├── requirements/          ← dependencias por módulo
├── tests/                 ← pruebas de integración
└── dashboard/             ← interfaz web (futuro)
```

## Inicio rápido

```bash
# 1. Clonar el repositorio
git clone <url-del-repo>
cd git

# 2. Ver guía de instalación
cat SETUP.md

# 3. Instalar dependencias Python
pip install -r requirements/rpi_requirements.txt

# 4. Compilar y flashear ESP32 motor (desde esp32/motor_controller/)
idf.py build
idf.py -p COM3 flash monitor

# 5. Compilar y flashear ESP32 consola (desde esp32/console_esp32/)
idf.py build
idf.py -p COM4 flash monitor

# 6. Probar con simulador (sin OpenCV)
python scripts/simulador_vision.py
```

## Estado del proyecto

| Componente | Estado |
|---|---|
| RPi + Mosquitto MQTT | ✅ Semana 1 completada |
| ESP32 Motor (BTS7960 + válvula) | ✅ Semana 2 completada |
| RPi como Access Point | ✅ Configurado |
| ESP32 Consola (LCD + botones) | ✅ Semana 3 completada |
| Módulo OpenCV (visión artificial) | 🔄 Semana 4 — en progreso |
| Dashboard web | ⬜ Futuro |
