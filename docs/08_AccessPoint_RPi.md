# Access Point — Raspberry Pi como Router WiFi propio

## Objetivo

Convertir la RPi en un router WiFi propio para un sistema 100% portátil sin depender de WiFi externo.

## Red resultante

```
[Raspberry Pi]   ← funciona como router
    │   crea red "CintaTransportadora"
    │   IP fija: 192.168.4.1
    ├── ESP32 motor   (192.168.4.X)
    ├── ESP32 consola (192.168.4.Y)
    └── PC            (192.168.4.Z)
```

## Configuración

```bash
# 1. Desconectar WiFi cliente anterior
sudo nmcli con down "netplan-wlan0-Alvaro y Susann_Ext"

# 2. Crear conexión Access Point
sudo nmcli con add type wifi ifname wlan0 con-name CintaAP \
    autoconnect yes ssid CintaTransportadora

# 3. Configurar modo AP e IP
sudo nmcli con modify CintaAP \
    802-11-wireless.mode ap \
    802-11-wireless.band bg \
    802-11-wireless.channel 7 \
    ipv4.method shared \
    ipv4.addresses 192.168.4.1/24

# 4. Configurar contraseña WPA2 (mínimo 8 caracteres)
sudo nmcli con modify CintaAP \
    wifi-sec.key-mgmt wpa-psk \
    wifi-sec.psk "cinta2026"

# 5. Activar
sudo nmcli con up CintaAP
```

> ⚠️ La contraseña DEBE tener mínimo 8 caracteres. "123" o "abc" causan error y dejan la red abierta.

## Verificar

```bash
hostname -I
# 192.168.1.100 192.168.4.1 ...
```

## Cambiar entre modos

```bash
# Modo Access Point (demo)
sudo nmcli con down "netplan-wlan0-Alvaro y Susann_Ext"
sudo nmcli con up CintaAP

# Modo cliente (internet)
sudo nmcli con down CintaAP
sudo nmcli con up "netplan-wlan0-Alvaro y Susann_Ext"
```

## Credenciales del sistema con AP activo

```c
// En config.h (ESP32 motor) y esp32_consola.c (ESP32 consola)
#define WIFI_SSID      "CintaTransportadora"
#define WIFI_PASSWORD  "cinta2026"
#define MQTT_BROKER    "mqtt://192.168.4.1:1883"
```

## Solución de problemas

| Problema | Solución |
|---|---|
| `psk: property is invalid` | Contraseña muy corta — mínimo 8 caracteres |
| La red no pide contraseña | La clave no se guardó; repetir paso 4 |
| ESP32 no se conecta | Verificar SSID/clave exacta (mayúsculas importan) |
| SSH a 192.168.4.1 da timeout | RPi cambió de red — esperar 30s |
| MQTT no responde | `sudo systemctl restart mosquitto` |
