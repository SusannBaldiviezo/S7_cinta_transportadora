# Firmware ESP32

## Proyectos

| Carpeta | Descripción | Puerto |
|---|---|---|
| `motor_controller/` | Control de banda (BTS7960) + válvula solenoide + MQTT | COM3 |
| `console_esp32/` | Consola portátil: LCD 16x2 + 4 botones + LED + MQTT | COM4 |

## Compilar y flashear

```bash
# ESP32 Motor
cd motor_controller
idf.py set-target esp32
idf.py build
idf.py -p COM3 flash monitor

# ESP32 Consola
cd ../console_esp32
idf.py set-target esp32
idf.py build
idf.py -p COM4 flash monitor
```

> En Linux usar `/dev/ttyUSB0` y `/dev/ttyUSB1` en lugar de COM3/COM4.  
> Para salir del monitor: `Ctrl + ]`

## Credenciales de red (ambos proyectos)

```c
#define WIFI_SSID      "CintaTransportadora"
#define WIFI_PASSWORD  "cinta2026"
#define MQTT_BROKER    "mqtt://192.168.4.1:1883"
```

Estas están definidas en:
- `motor_controller/main/config.h`
- `console_esp32/main/esp32_consola.c` (líneas 20-22)
