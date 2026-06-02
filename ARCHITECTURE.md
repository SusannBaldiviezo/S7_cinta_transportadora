# Arquitectura del Sistema

## Diagrama de capas

```
┌─────────────────────────────────────────────────────────┐
│                   CAPA DE ENTRADA                        │
│   [Cámara USB 1]    [Cámara USB 2]   [Sensor presencia] │
│        ↓ ángulo frontal  ↓ ángulo lateral   ↓ opcional  │
└──────────────────────┬──────────────────────────────────┘
                       │ frames de video
┌──────────────────────▼──────────────────────────────────┐
│                 CAPA DE PROCESAMIENTO                    │
│              PC / Laptop — Python 3.x                   │
│   ┌─────────────────────────────────────────────────┐   │
│   │  capture.py  →  OpenCV analysis  →  decision   │   │
│   │  (color + textura → "aceptada" | "rechazada")  │   │
│   └─────────────────┬───────────────────────────────┘   │
│                     │ publica a MQTT                     │
└─────────────────────┼───────────────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────────────┐
│               CAPA DE MENSAJERÍA (MQTT)                 │
│         Raspberry Pi — Mosquitto Broker v2.x            │
│         Red WiFi: CintaTransportadora (AP)              │
│         IP: 192.168.4.1  Puerto: 1883                   │
│                                                         │
│  Tópicos:                                               │
│    fruta/estado      → "aceptada" | "rechazada"         │
│    operario/comando  → "iniciar" | "detener"            │
│    sistema/motor     → "activo" | "detenido" | "online" │
│    sistema/valvula   → "disparada"                      │
└───────────────┬───────────────────────┬─────────────────┘
                │                       │
┌───────────────▼───────────┐ ┌─────────▼───────────────┐
│    ESP32 MOTOR            │ │   ESP32 CONSOLA          │
│    (motor_controller)     │ │   (console_esp32)        │
│                           │ │                          │
│  BTS7960 (GPIO 25,26,32,33│ │  LCD 16x2 I2C            │
│  Válvula (GPIO 27 → relé) │ │  4 botones (GPIO 14,25,  │
│  Motor DC 12V             │ │            26,27)        │
│  Cilindro neumático       │ │  LED indicador (GPIO 2)  │
│                           │ │  Alimentado por powerbank│
└───────────────────────────┘ └──────────────────────────┘
```

## Flujo de datos

```
1. Fruta entra a la banda transportadora
2. Cámaras capturan imagen desde 2 ángulos
3. PC analiza color y textura con OpenCV
4. PC publica veredicto en MQTT (fruta/estado)
5. Mosquitto distribuye el mensaje a todos los suscriptores
6. ESP32 Motor recibe "rechazada" → dispara válvula → cilindro expulsa fruta
7. ESP32 Consola recibe el mismo mensaje → actualiza LCD y contadores
8. Operario puede iniciar/detener desde consola → ESP32 Consola publica en operario/comando
9. ESP32 Motor escucha operario/comando → arranca o para el motor
```

## Topología de red

```
[Raspberry Pi]
    wlan0: 192.168.4.1  ← Access Point "CintaTransportadora"
    eth0:  192.168.1.100 ← Ethernet (opcional, para configuración)
    
Clientes WiFi:
    ESP32 Motor:   192.168.4.X (DHCP)
    ESP32 Consola: 192.168.4.Y (DHCP)
    PC/Laptop:     192.168.4.Z (DHCP)
```

## Tópicos MQTT

| Tópico | Publicado por | Suscrito por | Valores |
|---|---|---|---|
| `fruta/estado` | PC (OpenCV) | ESP32 Motor, ESP32 Consola | `aceptada`, `rechazada` |
| `operario/comando` | ESP32 Consola | ESP32 Motor | `iniciar`, `detener` |
| `sistema/motor` | ESP32 Motor | ESP32 Consola | `activo`, `detenido`, `esp32_online` |
| `sistema/valvula` | ESP32 Motor | ESP32 Consola | `disparada` |
