# Semana 1: Configuración Raspberry Pi

## Resultado final

| Componente | Estado | Valor |
|---|---|---|
| RPi con OS (Lite 64-bit) | ✅ | Linux raspi 6.18.29 aarch64 |
| Acceso SSH | ✅ | `ssh raspi@192.168.1.100` |
| IP fija RPi (Ethernet) | ✅ | `192.168.1.100` |
| Red WiFi conectada | ✅ | `Alvaro y Susann_Ext` |
| Mosquitto MQTT | ✅ | Puerto 1883, activo |

## Infraestructura de red

```
[Internet]
    |
[Router principal]
    |
[Mini Router KP-3069]  ← Red: "Alvaro y Susann_Ext"
    |
  Puerto LAN
    |
[Cable Ethernet]
    |
[Raspberry Pi]  ← IP fija: 192.168.1.100
    |
[Mosquitto MQTT]  ← Puerto 1883
```

## Pasos de configuración

### Fase 1 — Grabar OS con Raspberry Pi Imager
- OS: Raspberry Pi OS Lite (64-bit)
- Hostname: `raspi`
- Usuario: `raspi`
- SSH habilitado desde el Imager

### Fase 2 — Primer acceso SSH
```cmd
ssh raspi@raspi.local
```

### Fase 3 — Instalar Mosquitto
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

### Fase 4 — Permitir conexiones externas en Mosquitto
```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Agregar al final:
```
listener 1883
allow_anonymous true
```

```bash
sudo systemctl restart mosquitto
```

### Fase 5 — Asignar IP fija (con NetworkManager)

> Esta versión de RPi OS NO usa dhcpcd. Usa NetworkManager.

```bash
sudo nmcli con mod "netplan-eth0" ipv4.addresses 192.168.1.100/24
sudo nmcli con mod "netplan-eth0" ipv4.gateway 192.168.1.254
sudo nmcli con mod "netplan-eth0" ipv4.dns "8.8.8.8"
sudo nmcli con mod "netplan-eth0" ipv4.method manual
sudo nmcli con up "netplan-eth0"
```

Verificar:
```bash
hostname -I
# 192.168.1.100
```

## Prueba MQTT

```bash
# Ventana 1 (suscriptor)
mosquitto_sub -h localhost -t "prueba/cinta" -v

# Ventana 2 (publicador)
mosquitto_pub -h localhost -t "prueba/cinta" -m "sistema_ok"
```

## Comandos de referencia rápida

```bash
ssh raspi@192.168.1.100
sudo systemctl status mosquitto
sudo systemctl restart mosquitto
hostname -I
mosquitto_sub -h localhost -t "#" -v
```

## Solución de problemas

| Problema | Solución |
|---|---|
| SSH no conecta | `ssh-keygen -R 192.168.1.100` y reconectar |
| Mosquitto no arranca | `sudo systemctl restart mosquitto` |
| IP volvió a dinmica | Repetir comandos nmcli de Fase 5 |
| ESP32 no conecta al MQTT | `sudo ufw allow 1883` en la RPi |
