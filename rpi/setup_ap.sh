#!/bin/bash
# ================================================================
# setup_ap.sh — Configurar Raspberry Pi como Access Point WiFi
#
# Red:        CintaTransportadora
# Contraseña: cinta2026
# IP RPi:     192.168.4.1
#
# IMPORTANTE:
#   - La contraseña WPA2 necesita mínimo 8 caracteres
#   - El cable Ethernet puede seguir conectado (doble red)
#   - Este script es idempotente: se puede ejecutar más de una vez
#
# Ejecutar con:  bash setup_ap.sh
# ================================================================

set -e

SSID="CintaTransportadora"
PASSWORD="cinta2026"
IP="192.168.4.1/24"
CON_NAME="CintaAP"
WIFI_CLIENTE="netplan-wlan0-Alvaro y Susann_Ext"

echo "=== Bajando conexión WiFi cliente (si está activa) ==="
sudo nmcli con down "$WIFI_CLIENTE" 2>/dev/null || echo "(no estaba activa, continuando)"

echo "=== Eliminando configuración AP anterior (si existe) ==="
sudo nmcli con delete "$CON_NAME" 2>/dev/null || echo "(no existía, continuando)"

echo "=== Creando Access Point: $SSID ==="
sudo nmcli con add type wifi ifname wlan0 con-name "$CON_NAME" \
    autoconnect yes ssid "$SSID"

echo "=== Configurando modo AP e IP ==="
sudo nmcli con modify "$CON_NAME" \
    802-11-wireless.mode ap \
    802-11-wireless.band bg \
    802-11-wireless.channel 7 \
    ipv4.method shared \
    ipv4.addresses "$IP"

echo "=== Configurando contraseña WPA2 ==="
sudo nmcli con modify "$CON_NAME" \
    wifi-sec.key-mgmt wpa-psk \
    wifi-sec.psk "$PASSWORD"

echo "=== Activando Access Point ==="
sudo nmcli con up "$CON_NAME"

echo ""
echo "=== Verificando IPs ==="
hostname -I

echo ""
echo "================================================================"
echo "  Access Point creado exitosamente"
echo "  SSID:     $SSID"
echo "  Clave:    $PASSWORD"
echo "  IP (RPi): 192.168.4.1"
echo "  MQTT:     mqtt://192.168.4.1:1883"
echo "================================================================"
echo ""
echo "Próximo paso: conectá tu PC a la red '$SSID'"
echo "Luego accedé por SSH: ssh raspi@192.168.4.1"
