# Raspberry Pi — Configuración del servidor

## Rol en el sistema

La RPi actúa como el **núcleo de comunicación** del proyecto:

- **Broker MQTT (Mosquitto):** distribuye mensajes entre la PC, el ESP32 motor y el ESP32 consola
- **Access Point WiFi:** crea la red `CintaTransportadora` para que el sistema sea completamente portátil
- **IP fija:** `192.168.4.1` (AP) / `192.168.1.100` (Ethernet)

---

## Archivos de configuración

| Archivo | Descripción |
|---|---|
| `mosquitto.conf` | Configuración del broker MQTT |
| `setup_ap.sh` | Script para crear el Access Point con nmcli |
| `setup_mosquitto.sh` | Script de instalación y configuración de Mosquitto |
| `systemd/cinta-mqtt.service` | Servicio systemd para Mosquitto (arranque automático) |

---

## Acceso SSH

```bash
# Por Ethernet (configuración y mantenimiento)
ssh raspi@192.168.1.100

# Por WiFi AP (uso normal en demo)
ssh raspi@192.168.4.1
```

Si dice "REMOTE HOST IDENTIFICATION HAS CHANGED":
```bash
ssh-keygen -R 192.168.4.1
ssh raspi@192.168.4.1
```

---

## Comandos de uso frecuente

```bash
# Estado del broker MQTT
sudo systemctl status mosquitto

# Reiniciar Mosquitto
sudo systemctl restart mosquitto

# Escuchar todos los tópicos en tiempo real
mosquitto_sub -h localhost -t "#" -v

# Ver IPs activas (debe mostrar 192.168.4.1 y/o 192.168.1.100)
hostname -I

# Ver redes activas
nmcli con show --active

# Cambiar a modo AP (para demo)
sudo nmcli con down "netplan-wlan0-Alvaro y Susann_Ext"
sudo nmcli con up CintaAP

# Cambiar a modo cliente WiFi (para internet)
sudo nmcli con down CintaAP
sudo nmcli con up "netplan-wlan0-Alvaro y Susann_Ext"
```
