# Proyecto: 02_consola (Clasificador de Frutas)

Este es el proyecto de la consola para el sistema clasificador de frutas utilizando ESP-IDF, MQTT y una pantalla LCD I2C.

## Comandos Útiles de ESP-IDF

Aquí tienes una lista de los comandos esenciales que necesitarás ejecutar en la terminal (PowerShell) para trabajar con este proyecto:

### 1. Activar el entorno de ESP-IDF
Este comando es **obligatorio** cada vez que abres una nueva ventana de PowerShell antes de poder usar `idf.py`:
```powershell
C:\esp\v6.0\esp-idf\export.ps1
```

### 2. Configurar el proyecto
Abre el menú de configuración visual para ajustar parámetros del hardware, WiFi, MQTT, etc.:
```powershell
idf.py menuconfig
```

### 3. Compilar el proyecto
Construye el código fuente (crea los ejecutables binarios). La primera vez puede tardar un poco:
```powershell
idf.py build
```

### 4. Subir el código al ESP32 (Flashear)
*Nota: Cambia `COMx` por el puerto COM donde esté conectado tu ESP32 (ej. `COM3` o en Linux `/dev/ttyUSB0`)*
```powershell
idf.py -p COMx flash
```

### 5. Ver el monitor serie
Abre el monitor para ver los mensajes (`ESP_LOGI`, `printf`, etc.) que el ESP32 imprime en tiempo real:
```powershell
idf.py -p COMx monitor
```
*(Para salir del monitor serie presiona `Ctrl` + `]`)*

### 6. Compilar, flashear y monitorear (Todo en uno)
El comando más usado durante el desarrollo. Hace todo el proceso de un solo golpe:
```powershell
idf.py -p COMx flash monitor
```

### 7. Limpiar la compilación (Clean)
Útil si el código hace cosas raras y quieres forzar una compilación desde cero borrando todos los archivos temporales generados:
```powershell
idf.py fullclean
```
## Pasos realizados para que el proyecto compile

1. **Activar el entorno ESP‑IDF**  
   Ejecuta el script de exportación de PowerShell (solo una vez por sesión):

   ```powershell
   C:\esp\v6.0\esp-idf\export.ps1
   ```

2. **Compilar el proyecto**  

   ```powershell
   idf.py build
   ```

   La compilación se completa sin el error `-Werror=format-truncation` porque los buffers del LCD fueron ampliados a 64 bytes en `esp32_consola.c`.

3. **Flashear y monitorizar** (sólo cuando el ESP‑32 esté conectado):

   ```powershell
   idf.py -p COMx flash monitor
   ```

Con estos pasos el proyecto se construye y puede ser cargado en el ESP‑32.  
Si abres una nueva ventana de PowerShell, repite el paso 1 antes de ejecutar los demás comandos.

## Pruebas de MQTT (Simulando la Raspberry Pi)

Para comprobar que el sistema recibe correctamente los mensajes del clasificador y actualiza los contadores, puedes enviar mensajes MQTT manualmente simulando ser la Raspberry Pi.

1. **Conéctate a tu Raspberry Pi** desde una nueva ventana de PowerShell:
   ```bash
   ssh raspi@192.168.1.100
   ```

2. **Aceptar frutas:**
   Envía este comando para simular que pasó una fruta "aceptada". Puedes enviarlo varias veces y ver cómo sube el contador "OK" en la LCD:
   ```bash
   mosquitto_pub -h localhost -t "fruta/estado" -m "aceptada"
   ```

3. **Rechazar frutas:**
   Envía este comando para simular una fruta "rechazada". El contador "Rej" debería subir:
   ```bash
   mosquitto_pub -h localhost -t "fruta/estado" -m "rechazada"
   ```

4. **Probar la Alerta:**
   Si mandas 3 rechazos consecutivos, el sistema debería entrar en modo `!!! ALERTA !!!` y hacer parpadear el LED de la consola:
   ```bash
   mosquitto_pub -h localhost -t "fruta/estado" -m "rechazada"
   mosquitto_pub -h localhost -t "fruta/estado" -m "rechazada"
   mosquitto_pub -h localhost -t "fruta/estado" -m "rechazada"
   ```
   *(Para salir del estado de alerta, simplemente presiona el Botón 4 "RESET" en tu placa).*

5. **Probar el Relé / Motor / Válvula:**
   La consola **no** tiene el relé conectado. Para probar que el relé se activa, asegúrate de que la otra ESP32 (proyecto `01_motor`) esté encendida, conectada al WiFi y escuchando a MQTT. Luego envía el comando directo para la válvula:
   ```bash
   mosquitto_pub -h localhost -t "sistema/valvula" -m "abrir"
   ```

## FASE 8: Pruebas de Portabilidad (Powerbank)

1. Desconecta el cable USB de la PC.
2. Conecta la ESP32 (Consola) a un **Powerbank**.
3. El sistema debería encender y reconectarse al WiFi / MQTT automáticamente.
4. Llévala caminando por la habitación para comprobar que el WiFi se mantiene en rango sin perder la conexión.

✅ **¡Listo! Consola portátil operativa y funcionando al 100%.**
