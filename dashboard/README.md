# Dashboard

> **Estado:** No implementado — componente futuro opcional

## Descripción

Interfaz web para visualizar el estado del sistema en tiempo real desde un navegador.

## Funcionalidades planificadas

- Gráfica de frutas aceptadas vs rechazadas en tiempo real
- Control de inicio/detención desde el navegador
- Historial de sesiones
- Estado de conexión de los dispositivos

## Alternativas de implementación

### Opción A — Node-RED (más rápido, sin código)
```bash
# Instalar en RPi
npm install -g --unsafe-perm node-red
node-red
# Acceder desde: http://192.168.4.1:1880
```

### Opción B — React + MQTT.js (más flexible)
```bash
npm install
npm start
```

## Nota

Para la demostración del proyecto, la consola portátil (ESP32 + LCD) cumple el rol de interfaz de operario. El dashboard es un complemento opcional para visualización en PC.
