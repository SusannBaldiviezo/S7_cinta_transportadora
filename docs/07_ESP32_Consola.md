# Semana 3: ESP32 Consola Portátil

## Objetivo

Consola portátil totalmente inalámbrica con LCD 16x2 I2C, 4 pulsadores y LED. Permite al operario monitorear el estado del sistema y controlar el proceso sin estar frente a la PC.

## Hardware

| Componente | Conexión |
|---|---|
| LCD 16x2 I2C (PCF8574) | GPIO 21 (SDA), GPIO 22 (SCL) |
| Botón INICIAR | GPIO 14 → GND |
| Botón DETENER | GPIO 27 → GND |
| Botón PANTALLA | GPIO 26 → GND |
| Botón RESET | GPIO 25 → GND |
| LED indicador | GPIO 2 (con resistencia 220Ω) → GND |
| LCD VCC | 5V (VIN) |

## Código fuente

Ubicación: `esp32/console_esp32/main/`

| Archivo | Función |
|---|---|
| `esp32_consola.c` | Código principal: WiFi, MQTT, botones, LED, LCD |
| `lcd_i2c.c/.h` | Driver LCD HD44780 vía I2C (PCF8574) |

## Vistas de pantalla

### Pantalla 0 — Contadores (vista principal)
```
OK:0     Rej:0
Estado: esperando
```

### Pantalla 1 — Diagnóstico
```
WiFi: OK
Total: 0
```

### Pantalla 2 — Alerta (automática tras 3 rechazos seguidos)
```
!!! ALERTA !!!
Rechazos: 3
```

## Compilar y flashear

```bash
cd esp32/console_esp32
idf.py set-target esp32
idf.py build
idf.py -p COM4 flash monitor
```

## Prueba de contadores

```bash
# Desde la RPi:
mosquitto_pub -h localhost -t "fruta/estado" -m "aceptada"
mosquitto_pub -h localhost -t "fruta/estado" -m "rechazada"
```

## Encontrar dirección I2C del LCD

Si la LCD no muestra nada después de ajustar el contraste, puede ser que su dirección sea `0x3F` en lugar de `0x27`. El firmware incluye un escáner I2C en `lcd_init()` que lo reporta en el monitor serial.

Cambiar en `lcd_i2c.h`:
```c
#define LCD_I2C_ADDR  0x3F   // si 0x27 no funciona
```

## Solución de problemas

| Problema | Solución |
|---|---|
| LCD no muestra nada | Ajustar potenciómetro de contraste (atrás del módulo) |
| LCD parpadea texto raro | Dirección I2C incorrecta — ver monitor serial para la dirección real |
| Botón no responde | Verificar que esté entre GPIO y GND, no en VCC |
| LED no enciende | Verificar polaridad: ánodo (pata larga) → GPIO con resistencia |
| MQTT no conecta | Verificar IP RPi (`ping 192.168.4.1`) y que esté conectado a `CintaTransportadora` |
