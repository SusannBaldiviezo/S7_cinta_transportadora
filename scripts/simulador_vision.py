# ================================================================
# SIMULADOR DE VISIÓN ARTIFICIAL (Prueba MQTT)
# Envía veredictos aleatorios a la ESP32 para probar el motor y válvula
# sin necesidad de tener las cámaras o el módulo OpenCV configurado.
#
# USO:
#   1. Conectar PC a la red "CintaTransportadora"
#   2. Ejecutar: python simulador_vision.py
#
# BROKER: Raspberry Pi actuando como Access Point
# ================================================================

import paho.mqtt.client as mqtt
import random
import time

# IP del broker MQTT (Raspberry Pi como Access Point)
BROKER = "192.168.4.1"
PORT = 1883

try:
    # API v2.0+ (paho-mqtt >= 2.0)
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id="simulador_pc")
except AttributeError:
    # Fallback para paho-mqtt v1.x
    client = mqtt.Client(client_id="simulador_pc")

print(f"Conectando al Broker MQTT en {BROKER}:{PORT}...")
client.connect(BROKER, PORT, 60)

print("\n--- INICIANDO SISTEMA DESDE SIMULADOR ---")
client.publish("operario/comando", "iniciar")
print("  → Enviado: 'iniciar' a 'operario/comando' (Motor activado)")
time.sleep(2)

try:
    print("\nSimulando detección de frutas (Presiona Ctrl+C para detener)...")
    print("  Probabilidad: 66% aceptada, 33% rechazada\n")
    while True:
        estado = random.choice(["aceptada", "aceptada", "rechazada"])
        client.publish("fruta/estado", estado)
        print(f"  → Fruta detectada: {estado.upper()}")
        time.sleep(3)
except KeyboardInterrupt:
    client.publish("operario/comando", "detener")
    print("\n--- DETENIENDO SISTEMA DESDE SIMULADOR ---")
    print("  → Enviado: 'detener' a 'operario/comando' (Motor apagado)")
    client.disconnect()
