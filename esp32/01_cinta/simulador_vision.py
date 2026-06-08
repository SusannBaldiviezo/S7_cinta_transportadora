# ================================================================
# SIMULADOR DE VISIÓN ARTIFICIAL (Prueba MQTT)
# Envía veredictos aleatorios a la ESP32 para probar el motor y válvula
# ================================================================

import paho.mqtt.client as mqtt
import random
import time

# IP de tu Raspberry Pi (Broker MQTT)
BROKER = "192.168.1.100"
PORT = 1883

try:
    # Intentar inicializar con la API v2.0+ (versiones más recientes de paho-mqtt)
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id="simulador_pc")
except AttributeError:
    # Fallback para paho-mqtt v1.x (versiones anteriores)
    client = mqtt.Client(client_id="simulador_pc")

print(f"Conectando al Broker MQTT en {BROKER}...")
client.connect(BROKER, PORT, 60)

print("\n--- INICIANDO SISTEMA DESDE SIMULADOR ---")
# Manda el comando para encender la cinta y activar el sistema de la ESP32
client.publish("operario/comando", "iniciar")
print("  → Enviado: 'iniciar' a 'operario/comando' (Motor activado)")
time.sleep(2)

try:
    print("\nSimulando detección de frutas (Presiona Ctrl+C para detener)...")
    while True:
        # 66% de probabilidad de fruta aceptada, 33% de rechazada
        estado = random.choice(["aceptada", "aceptada", "rechazada"])
        client.publish("fruta/estado", estado)
        print(f"  → Fruta detectada: {estado.upper()}")
        time.sleep(3)
except KeyboardInterrupt:
    # Al presionar Ctrl+C en la terminal, detiene la cinta de manera segura
    client.publish("operario/comando", "detener")
    print("\n--- DETENIENDO SISTEMA DESDE SIMULADOR ---")
    print("  → Enviado: 'detener' a 'operario/comando' (Motor apagado)")
    client.disconnect()
