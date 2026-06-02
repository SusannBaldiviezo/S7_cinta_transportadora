# Tests

## Pruebas de integración

### Test 1 — MQTT end-to-end

```bash
# Terminal 1: suscribir a todos los tópicos
mosquitto_sub -h 192.168.4.1 -t "#" -v

# Terminal 2: simular veredictos
python ../scripts/simulador_vision.py
```

Verificar que:
- ESP32 Motor arranca al recibir `operario/comando = "iniciar"`
- Válvula se activa al recibir `fruta/estado = "rechazada"`
- LCD consola muestra contadores actualizados

### Test 2 — Consola portátil

1. Presionar Botón INICIAR → motor debe arrancar
2. Presionar Botón DETENER → motor debe parar
3. Enviar 3 rechazos consecutivos → LED debe parpadear, pantalla cambia a ALERTA
4. Presionar Botón RESET → alerta se limpia, contadores a cero

### Test 3 — Portabilidad sin internet

1. Desconectar el cable Ethernet de la RPi del router
2. Verificar que la red `CintaTransportadora` sigue activa
3. Confirmar que MQTT sigue funcionando entre todos los dispositivos
4. El sistema debe funcionar completamente sin internet

## Prueba rápida desde la RPi

```bash
ssh raspi@192.168.4.1

# Iniciar sistema
mosquitto_pub -h localhost -t "operario/comando" -m "iniciar"

# Simular frutas
mosquitto_pub -h localhost -t "fruta/estado" -m "aceptada"
mosquitto_pub -h localhost -t "fruta/estado" -m "rechazada"

# Detener
mosquitto_pub -h localhost -t "operario/comando" -m "detener"
```
