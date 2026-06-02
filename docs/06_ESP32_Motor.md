# Semana 2: ESP32 Motor — BTS7960 + Válvula + MQTT

## Objetivo

ESP32 conectada al WiFi `CintaTransportadora`, hablando con Mosquitto en la RPi (`192.168.4.1`), controlando motor DC 12V a través del BTS7960 y activando válvula solenoide vía relé cuando llega "rechazada".

## Hardware — Driver BTS7960

El driver de motor utilizado es el **BTS7960 (IBT-2)**, NO un L298N.

| Característica | L298N | BTS7960 |
|---|---|---|
| Corriente máxima | 2A | 43A |
| Voltaje | 5–35V | 5.5–27V |
| Pines de control | 3 (IN1, IN2, ENA) | 4 (RPWM, LPWM, R_EN, L_EN) |

## Pines de conexión

### ESP32 ↔ BTS7960
```
ESP32 GPIO 25  →  BTS7960 RPWM   (PWM giro adelante)
ESP32 GPIO 26  →  BTS7960 LPWM   (PWM giro atrás)
ESP32 GPIO 32  →  BTS7960 R_EN   (habilita lado derecho)
ESP32 GPIO 33  →  BTS7960 L_EN   (habilita lado izquierdo)
ESP32 5V (VIN) →  BTS7960 VCC
ESP32 GND      →  BTS7960 GND
```

### BTS7960 ↔ Fuente 12V ↔ Motor
```
Fuente 12V (+) →  BTS7960 B+
Fuente 12V (-) →  BTS7960 B-
BTS7960 M+     →  Motor (+)
BTS7960 M-     →  Motor (-)
```

### ESP32 ↔ Relé ↔ Válvula solenoide
```
ESP32 GPIO 27   →  Relé IN
ESP32 5V (VIN)  →  Relé VCC
ESP32 GND       →  Relé GND
Fuente 12V (+)  →  Relé COM
Relé NO         →  Válvula (+)
Válvula (-)     →  Fuente 12V (-)
```

> ⚠️ TIERRA COMÚN: ESP32 GND + BTS7960 GND + BTS7960 B- + Fuente 12V (-) + Relé GND deben estar todos unidos.

## Código fuente

Ubicación: `esp32/motor_controller/main/`

| Archivo | Función |
|---|---|
| `config.h` | Pines, red WiFi, tópicos MQTT, velocidad motor |
| `esp32_cinta.c` | Punto de entrada (`app_main`) |
| `motor.c/.h` | Control PWM del BTS7960 |
| `valvula.c/.h` | Control del relé con tarea FreeRTOS no bloqueante |
| `mqtt_handler.c/.h` | Procesamiento de mensajes MQTT |
| `wifi_manager.c/.h` | Inicialización WiFi y arranque MQTT |

## Compilar y flashear

```bash
cd esp32/motor_controller
idf.py set-target esp32
idf.py build
idf.py -p COM3 flash monitor
```

## Tópicos MQTT

| Tópico | Acción |
|---|---|
| `operario/comando = "iniciar"` | Arranca motor a velocidad 150/255 |
| `operario/comando = "detener"` | Para el motor |
| `fruta/estado = "rechazada"` | Dispara válvula 500ms (no bloqueante) |
| `sistema/motor` (publica) | Estado actual del motor |
| `sistema/valvula` (publica) | Confirmación de disparo |

## Prueba manual desde RPi

```bash
ssh raspi@192.168.4.1
mosquitto_pub -h localhost -t "operario/comando" -m "iniciar"
mosquitto_pub -h localhost -t "fruta/estado" -m "rechazada"
mosquitto_pub -h localhost -t "operario/comando" -m "detener"
```

## Solución de problemas

| Problema | Solución |
|---|---|
| Motor no gira | Verificar B+, B- conectados a fuente 12V |
| Motor gira al revés | Intercambiar cables M+ y M- |
| Motor "zumba" sin girar | Velocidad muy baja, aumentar `MOTOR_VEL_NORMAL` en config.h |
| Relé no hace clic | Confirmar tierra común con ESP32 GND |
| ESP32 se resetea | Fuente USB insuficiente, usar fuente 2A |
