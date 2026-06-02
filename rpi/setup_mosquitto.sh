#!/bin/bash
# ================================================================
# setup_mosquitto.sh — Instalación y configuración de Mosquitto
# Ejecutar en la Raspberry Pi con:  bash setup_mosquitto.sh
# ================================================================

set -e

echo "=== Instalando Mosquitto MQTT Broker ==="
sudo apt update
sudo apt install -y mosquitto mosquitto-clients

echo "=== Copiando configuración ==="
sudo cp mosquitto.conf /etc/mosquitto/mosquitto.conf

echo "=== Habilitando y arrancando servicio ==="
sudo systemctl enable mosquitto
sudo systemctl restart mosquitto

echo "=== Verificando estado ==="
sudo systemctl status mosquitto --no-pager

echo ""
echo "=== Prueba rápida ==="
echo "Abrí otra terminal y ejecutá:"
echo "  mosquitto_sub -h localhost -t 'test' -v"
echo "Luego en esta terminal:"
echo "  mosquitto_pub -h localhost -t 'test' -m 'ok'"
