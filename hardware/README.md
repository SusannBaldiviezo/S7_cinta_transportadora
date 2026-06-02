# Hardware

## Lista de componentes

| ID | Componente | Descripción | Estado |
|---|---|---|---|
| H-01 | ESP32 WROOM-32 | Microcontrolador principal (motor) | ✅ |
| H-02 | 2x Cámaras USB | Captura de imágenes de frutas | ⬜ pendiente |
| H-03 | Cilindro neumático | Expulsión física de fruta rechazada | ✅ |
| H-04 | Válvula solenoide 12V | Controla aire al cilindro (via relé) | ✅ |
| H-05 | Motor DC 12V + banda | Transporte de fruta | ✅ |
| H-06 | Compresor de aire | Suministro neumático | ✅ |
| H-07 | PC/Laptop | Procesamiento OpenCV | ✅ |
| H-08 | Fuente 12V | Alimenta motor y válvula | ✅ |
| H-09 | Iluminación LED | Luz controlada sobre zona de inspección | ⬜ pendiente |
| H-10 | ESP32 WROOM-32 | Microcontrolador consola portátil | ✅ |
| H-11 | LCD 16x2 I2C | Display consola (módulo PCF8574) | ✅ |
| H-12 | Powerbank 5V | Alimentación autónoma consola | ✅ |
| H-13 | 4x pulsadores | Botones de control operario | ✅ |
| H-14 | LED + resistencia 220Ω | Indicador de alerta | ✅ |
| H-15 | BTS7960 (IBT-2) | Driver puente H para motor DC | ✅ |
| H-16 | Relé 5V | Control de válvula solenoide | ✅ |

## Conexiones ESP32 Motor ↔ BTS7960

```
ESP32 GPIO 25  →  BTS7960 RPWM   (PWM giro adelante)
ESP32 GPIO 26  →  BTS7960 LPWM   (PWM giro atrás)
ESP32 GPIO 32  →  BTS7960 R_EN   (habilita lado derecho)
ESP32 GPIO 33  →  BTS7960 L_EN   (habilita lado izquierdo)
ESP32 5V (VIN) →  BTS7960 VCC
ESP32 GND      →  BTS7960 GND    ← tierra común obligatoria

Fuente 12V (+) →  BTS7960 B+
Fuente 12V (-) →  BTS7960 B-     ← misma tierra común
BTS7960 M+     →  Motor (+)
BTS7960 M-     →  Motor (-)
```

## Conexiones ESP32 Motor ↔ Relé ↔ Válvula

```
ESP32 GPIO 27   →  Relé IN
ESP32 5V (VIN)  →  Relé VCC
ESP32 GND       →  Relé GND      ← tierra común
Fuente 12V (+)  →  Relé COM
Relé NO         →  Válvula (+)
Válvula (-)     →  Fuente 12V (-) ← tierra común
```

## Conexiones ESP32 Consola ↔ LCD I2C

```
ESP32 GPIO 21  →  LCD SDA
ESP32 GPIO 22  →  LCD SCL
ESP32 5V (VIN) →  LCD VCC
ESP32 GND      →  LCD GND

Dirección I2C por defecto: 0x27
(Si no funciona, probar: 0x3F)
```

## Conexiones Botones y LED (consola)

```
GPIO 14 → Botón INICIAR → GND   (pull-up interno activo)
GPIO 27 → Botón DETENER → GND
GPIO 26 → Botón PANTALLA → GND
GPIO 25 → Botón RESET → GND
GPIO 2  → Resistencia 220Ω → LED ánodo → LED cátodo → GND
```

## Tierra común — crítica

Todos los GND deben estar unidos:
- ESP32 GND
- BTS7960 GND (señales)
- BTS7960 B- (potencia)
- Fuente 12V (-)
- Relé GND

Sin tierra común, el sistema se comporta de forma errática.
