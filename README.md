Markdown# Sistema Automático de Clasificación de Frutas

**Universidad:** Universidad Católica Boliviana "San Pablo"
**Materia:** Sistemas Embebidos II
**Docente:** Msc. Ing. Alan Cornejo Q.

**Universidad:** Universidad Católica Boliviana "San Pablo"  
**Materia:** Sistemas Embebidos II  
**Docente:** Msc. Ing. Alan Cornejo Q.  

**Semestre:** 1/2026
=======
**Semestre:** 1/2026  


**Estudiantes:** - Susann Baldiviezo C.  
- Alejandro Bejarano R.  
- Florencia Frigerio A.  
- Benjamín Soruco G.  

---

## Descripción

El objetivo principal de este proyecto es automatizar, mediante una cinta transportadora, la selección y clasificación de tomates en las zonas productoras del departamento de Tarija. Para dar solución a esta necesidad, se presenta este prototipo a escala como una alternativa tecnológica eficiente.

El sistema realiza la clasificación automática mediante visión artificial y un microcontrolador ESP32. Detecta tomates en estado de descomposición a través de dos cámaras USB y OpenCV, activa un mecanismo de desviación controlado por **dos servomotores** para expulsarlos de la banda transportadora, y permite al operario monitorear y controlar todo el proceso desde una consola portátil inalámbrica.

## Arquitectura general

[2 Cámaras USB]│▼[PC — Python + OpenCV]  ──────────────────────────────────────────┐│  publica fruta/estado                                     │▼                                                           │[Raspberry Pi — Mosquitto MQTT]  (AP: CintaTransportadora)        ││  distribuye                                               │├────────────────────────┐                                  │▼                        ▼                                  │[ESP32 Motor]          [ESP32 Consola]  ◄── operario              │BTS7960                Pantalla OLED                           │Banda DC               4 botones                               │2 Servomotores                                                 ││                        │                                  │▼                        ▼                                  │[Motor 12V]          [Pantalla de estado]  ───────────────────────┘[Mecanismo Desviador]
## Estructura del repositorio

├── README.md              ← este archivo├── ARCHITECTURE.md        ← diagrama detallado del sistema├── SETUP.md               ← cómo levantar el proyecto de cero├── .gitignore│├── docs/                  ← documentación completa├── esp32/                 ← firmware embebido (motor + consola)├── hardware/              ← conexiones, pinouts, datasheets├── ml/                    ← módulo de visión artificial (OpenCV/YOLO)├── rpi/                   ← scripts Python para la Raspberry Pi├── scripts/               ← herramientas y simuladores├── config/                ← configuración de servicios (Mosquitto, etc.)├── requirements/          ← dependencias por módulo├── tests/                 ← pruebas de integración└── dashboard/             ← interfaz web 
## Inicio rápido

```bash
# 1. Clonar el repositorio
git clone <url-del-repo>
cd S7_cinta_transportadora

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

# 6. Ejecutar el sistema principal
python ml/main_vision.py
Estado del proyectoComponenteEstadoRPi + Mosquitto MQTT✅ CompletadoESP32 Motor (BTS7960 + 2 Servomotores)✅ CompletadoRPi como Access Point✅ CompletadoESP32 Consola (Pantalla OLED + botones)✅ CompletadoMódulo OpenCV (Visión artificial para tomates)✅ CompletadoIntegración y Pruebas en Banda✅ CompletadoDashboard web✅ Completado