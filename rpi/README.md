# Raspberry Pi — Servidor central

## Funciones de la RPi en el sistema

1. **Broker MQTT (Mosquitto):** Distribuye mensajes entre PC, ESP32 motor y ESP32 consola.
2. **Access Point WiFi:** Crea la red `CintaTransportadora` para que todo el sistema sea portátil.

## Configuración completada

- OS: Raspberry Pi OS Lite 64-bit
- Hostname: `raspi`
- IP fija Ethernet: `192.168.1.100`
- IP Access Point: `192.168.4.1`
- Mosquitto: activo en puerto 1883
- Red AP: `CintaTransportadora` / contraseña: `cinta2026`

## Módulo de visión (Semana 4 — en desarrollo)

```
rpi/src/vision/
├── clasificador.py      ← script principal (pendiente)
├── capture.py           ← captura de cámaras (pendiente)
└── analisis_color.py    ← análisis HSV (pendiente)
```

## Acceso SSH

```bash
# Por Ethernet (configuración)
ssh raspi@192.168.1.100

# Por WiFi AP (uso normal del sistema)
ssh raspi@192.168.4.1
```

## Comandos útiles

```bash
# Estado del broker
sudo systemctl status mosquitto

# Escuchar todos los tópicos MQTT
mosquitto_sub -h localhost -t "#" -v

# Ver red activa
nmcli con show --active
```
