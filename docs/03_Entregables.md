# 6. Entregables principales del proyecto

El entregable principal del proyecto es el prototipo físico funcional del sistema de clasificación de frutas.

El prototipo deberá demostrar en funcionamiento la cadena completa: captura de imagen → procesamiento con OpenCV → decisión → señal a ESP32 → acción del cilindro neumático, con frutas reales en condiciones de buen estado y mal estado. Adicionalmente, la consola portátil deberá mostrar en tiempo real los resultados recibidos vía WiFi/MQTT y permitir al operario controlar el proceso desde la misma.

## Tabla de entregables

| Entregable | Descripción | Estado |
|---|---|---|
| Prototipo físico | Banda transportadora con cámaras, cilindro neumático y ESP32 principal integrados y funcionando | Principal |
| Módulo de visión | Script Python con OpenCV que detecta fruta deteriorada en tiempo real desde 2 cámaras | En progreso |
| Firmware ESP32 Motor | Código embebido para control de banda, válvula y comunicación con PC (vía MQTT) | ✅ Completado |
| Firmware ESP32 Consola | Código embebido de la unidad portátil — LCD, botones, LED, MQTT | ✅ Completado |
| Consola portátil | Hardware (ESP32 + LCD + powerbank + botones) y firmware de la unidad de monitoreo | ✅ Completado |
| Access Point RPi | RPi configurada como router WiFi propio para portabilidad total del sistema | ✅ Completado |
| Demostración en vivo | Presentación del sistema funcionando con frutas reales ante el docente | Pendiente |

## Ubicación en el repositorio

| Entregable | Carpeta |
|---|---|
| Firmware ESP32 Motor | `esp32/motor_controller/` |
| Firmware ESP32 Consola | `esp32/console_esp32/` |
| Módulo de visión (OpenCV) | `rpi/src/vision/` |
| Scripts de prueba | `scripts/` |
| Documentación | `docs/` |
| Hardware y conexiones | `hardware/` |
