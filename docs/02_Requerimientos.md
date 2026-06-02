# 5. Requerimientos

## 5.1. Requerimientos de Hardware

| ID | Componente | Descripción | Cant. |
|---|---|---|---|
| H-01 | ESP32 (o similar) | Microcontrolador para control de actuadores y comunicación con la PC | 1 |
| H-02 | Cámaras USB/IP (o similar) | 2 cámaras superiores en ángulos distintos para captura de imágenes | 2 |
| H-03 | Cilindro neumático (o similar) | Actuador para expulsión física de fruta deteriorada | 1 |
| H-04 | Válvula solenoide (o similar) | Controla el paso de aire al cilindro neumático (GPIO) | 1 |
| H-05 | Banda transportadora (o similar) | Motor + cinta para desplazamiento de la fruta | 1 |
| H-06 | Compresor de aire (o similar) | Suministra aire comprimido al cilindro neumático | 1 |
| H-07 | PC / Laptop (o similar) | Equipo de cómputo para procesamiento de imágenes con OpenCV | 1 |
| H-08 | Fuente de alimentación (o similar) | Suministro eléctrico para PC, cámaras y motor | 1 |
| H-09 | Iluminación artificial | Fuente de luz controlada sobre la zona de inspección | 1 |
| H-10 | ESP32 consola (o similar) | Segunda placa dedicada a la unidad portátil de monitoreo y control | 1 |
| H-11 | Pantalla TFT SPI ILI9341 (o similar) | Display a color 2.4"–3.5" para visualización de datos en tiempo real | 1 |
| H-12 | Batería LiPo 3.7V + TP4056 (o similar) | Sistema de alimentación autónoma recargable para la consola portátil | 1 |
| H-13 | Botones táctiles x4 (o similar) | Interfaz física para navegación y control por parte del operario | 4 |
| H-14 | Buzzer + LED RGB (o similar) | Alertas sonoras y visuales para notificaciones del sistema | 1 c/u |

> **Nota:** La implementación actual usa LCD 16x2 I2C (en lugar de TFT ILI9341) y LED simple (en lugar de LED RGB + buzzer). El diseño es compatible con los upgrades indicados en el documento.

## 5.2. Requerimientos de Software

| ID | Software | Descripción | Versión |
|---|---|---|---|
| S-01 | Python 3.x | Lenguaje de programación para el módulo de visión artificial | 3.10+ |
| S-02 | OpenCV (o similar) | Biblioteca de visión por computadora para análisis de color/textura | 4.x |
| S-03 | NumPy (o similar) | Manejo de arrays y operaciones matemáticas sobre imágenes | Latest |
| S-04 | Arduino IDE / ESP-IDF (o similar) | Entorno de desarrollo para programar la ESP32 | Latest |
| S-05 | PySerial (o similar) | Comunicación serial entre Python y la ESP32 | Latest |
| S-06 | TFT_eSPI (o similar) | Driver para control eficiente de pantallas SPI en ESP32 | Latest |
| S-07 | PubSubClient (o similar) | Cliente MQTT ligero para ESP32; suscripción y publicación de mensajes | Latest |
| S-08 | Mosquitto MQTT Broker (o similar) | Broker de mensajería liviano instalado en la PC/RPi central | 2.x |

## 5.3. Requerimientos Funcionales

### Sistema principal de clasificación:

- **RF-01** El sistema debe capturar imágenes de cada fruta desde dos cámaras superiores en tiempo real.
- **RF-02** El algoritmo debe analizar color y textura para detectar zonas de descomposición en la superficie de la fruta.
- **RF-03** El sistema debe clasificar cada fruta en "buen estado" o "mal estado" de forma automática.
- **RF-04** La ESP32 debe recibir la señal de clasificación y activar o no el cilindro neumático en consecuencia.
- **RF-05** La banda transportadora debe detenerse o ajustar su velocidad cuando una fruta esté siendo analizada.

### Consola portátil del operario:

- **RF-06** La consola debe recibir y mostrar en pantalla los datos de clasificación con una latencia máxima de 1 segundo desde que la PC emite el veredicto.
- **RF-07** El operario debe poder iniciar, pausar y reanudar tanto la banda como el proceso completo de clasificación desde la consola portátil mediante botones dedicados.
- **RF-08** La consola debe emitir una alerta (buzzer o LED) cuando se detecte un volumen elevado de fruta rechazada de forma consecutiva.
- **RF-09** La consola debe indicar visualmente (LED o mensaje en pantalla) cuando se pierda la comunicación WiFi con el sistema central.
- **RF-10** La consola debe funcionar de forma autónoma (sin cable) gracias a la batería integrada.
- **RF-11** El operario debe poder iniciar y detener el proceso completo (banda + sistema de visión) directamente desde la consola portátil, sin necesidad de acceder a la PC principal.

## 5.4. Requerimientos No Funcionales

- **RNF-01** El tiempo de procesamiento por fruta no debe superar los 2 segundos.
- **RNF-02** El sistema debe operar de manera continua durante al menos 30 minutos sin fallos.
- **RNF-03** El código debe estar estructurado de forma modular y documentado.
- **RNF-04** El prototipo debe ser reproducible con componentes de bajo costo disponibles en el mercado local.
