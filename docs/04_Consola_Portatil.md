# 7. Consola portátil para el operario (Unidad de Monitoreo y Control Móvil)

Como complemento al sistema central de clasificación, se incorpora una consola portátil que permite al operario de planta monitorear en tiempo real el estado del proceso e **iniciar o detener el proceso completo** sin necesidad de permanecer frente a la PC principal. Esta unidad móvil está basada en una segunda placa ESP32 y actúa como terminal de visualización y control inalámbrico, recibiendo y enviando datos al sistema central mediante comunicación WiFi (protocolo MQTT).

## 7.1. Descripción del dispositivo

La consola portátil está diseñada para ser compacta, robusta y de fácil uso en entornos de planta. Su arquitectura integra los siguientes componentes principales:

- **ESP32 (módulo WROOM-32, WROVER o similar):** Microcontrolador principal de la consola. Gestiona la comunicación WiFi, el control de la pantalla y la lectura de botones.

- **Pantalla TFT LCD (2.4"–3.5", resolución 240x320, o similar):** Interfaz visual para el operario. Muestra contadores de frutas clasificadas, alertas de error, estado de la banda y resultados del proceso en tiempo real.
  > **Implementación actual:** LCD 16x2 I2C (HD44780 + PCF8574). Funcional y de bajo costo. Puede actualizarse a TFT sin cambiar la lógica MQTT.

- **Botones físicos (3–4 pulsadores táctiles o similar):** Permiten al operario iniciar el proceso, detenerlo, navegar entre pantallas de información, confirmar alertas y reiniciar contadores.

- **LED indicador RGB o buzzer (o similar):** Alerta visual/sonora cuando se detecta un volumen elevado de fruta rechazada o cuando el sistema entra en estado de error.

- **Batería LiPo 3.7V + módulo cargador TP4056 (o similar):** Alimentación autónoma recargable.
  > **Implementación actual:** Powerbank USB 5V. Funcional para la demostración.

- **Carcasa impresa en 3D (o similar):** Estructura compacta tipo gamepad.

## 7.2. Información mostrada y controles disponibles

### Pantalla 0 — Estado General (vista principal)
```
OK:0     Rej:0
Estado: esperando
```
- Total de frutas aceptadas / rechazadas en la sesión.
- Estado operativo del motor.

### Pantalla 1 — Diagnóstico
```
WiFi: OK
Total: 0
```
- Estado de la comunicación WiFi (conectado/desconectado).
- Total acumulado de frutas procesadas.

### Pantalla 2 — Alertas
```
!!! ALERTA !!!
Rechazos: 3
```
- Se activa automáticamente cuando hay 3 rechazos consecutivos.
- El LED parpadea a 300ms mientras la alerta está activa.

### Botones

| Botón | GPIO | Función |
|---|---|---|
| INICIAR | GPIO 14 | Publica `operario/comando = "iniciar"` → arranca motor |
| DETENER | GPIO 27 | Publica `operario/comando = "detener"` → para motor |
| PANTALLA | GPIO 26 | Cambia entre las 3 vistas del LCD |
| RESET | GPIO 25 | Resetea contadores y desactiva alerta |

## 7.3. Arquitectura de comunicación

La consola portátil se integra al sistema mediante una red WiFi local (Access Point generado por la RPi).

1. La PC central ejecuta el script Python de visión artificial y, tras cada clasificación, publica el resultado vía MQTT al broker local (Mosquitto, corriendo en la RPi).

2. El ESP32 de control de actuadores también publica su estado (banda activa, cilindro disparado, contadores) al mismo broker MQTT.

3. La consola portátil (ESP32 secundario) se suscribe a los tópicos MQTT relevantes y actualiza su pantalla LCD en tiempo real con cada mensaje recibido.

4. Los comandos del operario (iniciar proceso, detener proceso, resetear contador) se envían desde la consola al broker MQTT y son ejecutados por el ESP32 de control al recibirlos.

### Tópicos MQTT utilizados por la consola

| Tópico | Acción |
|---|---|
| `fruta/estado` (suscripción) | Actualiza contadores OK/Rej en LCD |
| `sistema/motor` (suscripción) | Muestra estado del motor en LCD |
| `sistema/valvula` (suscripción) | Registra activación del cilindro |
| `operario/comando` (publicación) | Envía comandos iniciar/detener al ESP32 motor |

## 7.4. Pines de conexión (implementación actual)

```
ESP32 GPIO 21  →  LCD SDA (I2C)
ESP32 GPIO 22  →  LCD SCL (I2C)
ESP32 GPIO 14  →  Botón INICIAR → GND
ESP32 GPIO 27  →  Botón DETENER → GND
ESP32 GPIO 26  →  Botón PANTALLA → GND
ESP32 GPIO 25  →  Botón RESET → GND
ESP32 GPIO 2   →  LED (con resistencia 220Ω) → GND
ESP32 5V (VIN) →  LCD VCC
ESP32 GND      →  LCD GND, botones, LED (tierra común)
```
