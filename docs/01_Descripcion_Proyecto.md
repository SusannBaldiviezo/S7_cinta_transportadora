# 1. Descripción técnica-conceptual del proyecto

## Sistema Automático de Clasificación de Frutas mediante Visión Artificial y ESP32

El presente proyecto consiste en el diseño e implementación de un sistema automático de clasificación de frutas en general mediante técnicas de visión artificial y sistemas embebidos. El sistema integra dos cámaras posicionadas estratégicamente en la parte superior de una banda transportadora, desde ángulos distintos, con el objetivo de capturar la mayor superficie visible de cada fruta y detectar con mayor precisión zonas de deterioro o descomposición.

El procesamiento de imágenes se realiza en una computadora personal (PC o laptop) mediante el uso de la biblioteca OpenCV, aplicando algoritmos de análisis de color y textura para identificar frutas en estado de descomposición. Una vez que el sistema de visión artificial emite un veredicto, la señal de control es transmitida a una placa ESP32 que actúa como controlador embebido encargado de gestionar los actuadores físicos: la banda transportadora y el cilindro neumático de expulsión.

El cilindro neumático, accionado mediante una válvula solenoide controlada por la ESP32, se encarga de desviar físicamente las frutas clasificadas como deterioradas fuera de la línea principal, separándolas de aquellas en buen estado.

Como complemento al sistema central, se incorpora una **consola portátil** basada en una segunda ESP32 que permite al operario monitorear el estado del proceso en tiempo real desde cualquier punto de la planta, sin necesidad de permanecer frente a la PC principal. Desde esta consola el operario puede iniciar y detener el proceso completo de clasificación.

## Tabla 1: Arquitectura general del sistema

| Entradas | Procesamiento | Control Embebido | Salidas |
|---|---|---|---|
| 2 cámaras superiores USB/IP | PC con OpenCV (Python) | ESP32 principal | Fruta clasificada |
| Parámetros: calidad y especie | Análisis de color y textura | Controla banda y actuadores | Expulsión por cilindro |
| Sensor opcional de presencia | Decisión: buen/mal estado | Comunicación serial/WiFi | Registro de resultados |

---

# 2. Propósito del proyecto

El propósito principal del proyecto es automatizar el proceso de clasificación de frutas en general mediante la detección de deterioro o descomposición a través de visión artificial. Con ello se busca:

- Reducir la intervención humana en tareas repetitivas y de alto margen de error visual.
- Mejorar la eficiencia y velocidad del proceso de selección en líneas de producción agroindustrial.
- Minimizar las pérdidas económicas generadas por la comercialización de fruta en mal estado.
- Aplicar conceptos teórico-prácticos de sistemas embebidos y visión artificial en un prototipo funcional real.
- Demostrar la viabilidad técnica de integrar una ESP32 con un sistema de visión por computadora de bajo costo, incluyendo una consola portátil de monitoreo y control para el operario de planta.

---

# 3. Alcance del proyecto

El proyecto abarca el diseño, construcción y validación de un prototipo funcional de clasificación de frutas en general.

## 3.1. Lo que el sistema SÍ contempla

- Detección visual de fruta en estado de descomposición mediante dos cámaras superiores y OpenCV.
- Control de la banda transportadora a través de la ESP32.
- Activación del cilindro neumático para la expulsión física de fruta deteriorada.
- Comunicación entre la PC (sistema de visión) y la ESP32 (sistema embebido).
- Operación bajo condiciones de iluminación controlada y ambiente de laboratorio.
- Clasificación de fruta en dos categorías: buen estado y mal estado.
- Consola portátil basada en una segunda ESP32 para monitoreo inalámbrico en tiempo real y control del proceso por parte del operario de planta.

## 3.2. Lo que el sistema NO contempla

- Clasificación por tamaño, peso o madurez de la fruta (solo estado de descomposición).
- Operación en entornos industriales de alta velocidad o con múltiples tipos de fruta simultáneamente.
- Implementación de interfaz gráfica avanzada o panel de control remoto vía red (más allá de la consola portátil del operario).
- Integración con sistemas ERP o bases de datos de trazabilidad industrial.

---

# 4. Supuestos del proyecto

- **Iluminación controlada:** El prototipo operará bajo condiciones de iluminación artificial controlada y constante.
- **Fruta sobre banda transportadora:** Las frutas serán colocadas manualmente sobre la banda en posición individual y con separación suficiente.
- **Velocidad de banda constante:** La velocidad de desplazamiento será fija y calibrada previamente.
- **Comunicación confiable PC-ESP32:** Se asume disponibilidad de comunicación serial (USB) o inalámbrica estable entre la PC y la ESP32 principal.
- **Red WiFi local disponible:** Existe disponibilidad de red WiFi local (o Access Point generado por la RPi) para la comunicación con la consola portátil del operario.
- **Componentes disponibles:** El equipo cuenta con los componentes físicos necesarios para la construcción del prototipo (incluyendo la segunda ESP32 y pantalla para la consola portátil).
- **Uso en ambiente de laboratorio:** El prototipo es diseñado para operar en condiciones de laboratorio o taller académico.
