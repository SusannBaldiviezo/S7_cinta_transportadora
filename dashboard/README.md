# Sistema Clasificador de Frutas — Dashboard

Panel de control web en tiempo real para el sistema de clasificación de tomates con cinta transportadora.  
Desarrollado para UCB San Pablo · Sistemas Embebidos II · 2026.

---

## Arquitectura general

```
[ESP32 Cinta]  ──────────┐
                         ├─── WiFi ───► [Raspberry Pi AP]
[ESP32 Consola] ─────────┘               192.168.4.1
    (botones + OLED)                      │
                                          ├─ Mosquitto MQTT :1883
                                          ├─ FastAPI backend :8080
                                          └─ React frontend (servido por FastAPI)
```

La Raspberry Pi actúa como **punto de acceso WiFi** con SSID `CintaTransportadora`. Cualquier dispositivo conectado a esa red puede abrir el dashboard en `http://192.168.4.1:8080`.

---

## Estructura del proyecto

```
dashboard/
├── main.py            # Backend FastAPI + bridge MQTT (paho-mqtt)
├── data/
│   └── frutas.csv     # Historial persistente de clasificaciones
├── static/            # Build de React (generado con npm run build)
│   ├── index.html
│   └── assets/
└── frontend/          # Código fuente React + Vite
    ├── src/
    │   ├── App.jsx
    │   ├── index.css          # Sistema de diseño (variables CSS, tipografías)
    │   ├── api.js             # Llamadas HTTP al backend
    │   ├── hooks/
    │   │   └── useDashboard.js  # Polling cada 1500ms
    │   └── components/
    │       ├── Header.jsx
    │       ├── StatusCards.jsx
    │       ├── ControlPanel.jsx
    │       ├── AdminPanel.jsx
    │       ├── ConveyorPanel.jsx   # Animación cinta + bins de clasificación
    │       ├── CameraPanel.jsx     # Vista cámara + resultado de clasificación
    │       ├── OledMirror.jsx      # Espejo de la OLED del ESP32 Consola
    │       ├── CountersPanel.jsx
    │       ├── ChartsRow.jsx
    │       └── HistoryTable.jsx
    └── package.json
```

---

## MQTT — Tópicos

| Tópico              | Dirección                   | Contenido                              |
|---------------------|-----------------------------|----------------------------------------|
| `fruta/estado`      | ESP32 Cinta → todos         | `aceptada` / `podrida` / `verde`       |
| `sistema/motor`     | ESP32 Cinta → todos         | `activo` / `detenido`                  |
| `operario/comando`  | Dashboard/Consola → Cinta   | `iniciar` / `detener`                  |
| `operario/pantalla` | Dashboard → Consola         | Número de pantalla `0`, `1`, `2`       |
| `operario/reset`    | Dashboard → Consola         | `reset`                                |
| `consola/pantalla`  | ESP32 Consola → Dashboard   | Número de pantalla al presionar BTN    |
| `consola/reset`     | ESP32 Consola → Dashboard   | `reset` al presionar BTN_RESET         |

### Sincronización bidireccional

- **Botones ESP32 → Dashboard**: el ESP32 publica en `operario/comando`, `consola/pantalla` y `consola/reset`. El backend escucha y actualiza el estado. El frontend hace polling cada 1,5 s.
- **Dashboard → ESP32**: el backend publica en `operario/comando`, `operario/pantalla` y `operario/reset`. El ESP32 Consola está suscrito a todos estos.

---

## OLED Consola (SSD1306 128×64)

Tres pantallas, se ciclan con BTN_PANTALLA (GPIO 16) o desde el botón "Cambiar Pantalla" del dashboard:

| Pantalla | Contenido |
|----------|-----------|
| 0 — CONTADORES FRUTA | Aceptadas / Podridas / Verdes + estado motor |
| 1 — DIAGNOSTICO SIST | Total clasificadas + rechazos consecutivos   |
| 2 — SISTEMA ALERTAS  | Alerta parpadeante si rechazos consecutivos ≥ 3 |

El panel **Consola OLED** en el dashboard es un espejo fiel de lo que muestra el ESP32. Desde ahí también se puede:

- **Cambiar Pantalla** → publica `operario/pantalla` vía MQTT (el ESP32 cambia su OLED físico)
- **Reiniciar Contadores** → publica `operario/reset` vía MQTT (ambos dispositivos resetean sus contadores)

---

## Pines ESP32 Consola

| Pin      | Función                                          |
|----------|--------------------------------------------------|
| GPIO 18  | BTN_INICIAR — arranca la cinta                   |
| GPIO 17  | BTN_DETENER — para la cinta                      |
| GPIO 16  | BTN_PANTALLA — cicla la pantalla OLED            |
| GPIO 4   | BTN_RESET — reinicia contadores de sesión        |
| GPIO 2   | LED estado (parpadeo rápido = sin WiFi; lento = alerta) |
| GPIO 21  | SDA OLED I2C                                     |
| GPIO 22  | SCL OLED I2C                                     |

---

## Instalación y despliegue

### En la Raspberry Pi

```bash
# Instalar dependencias Python (una sola vez)
pip install fastapi uvicorn paho-mqtt

# Ejecutar el backend (desde la carpeta dashboard/)
python main.py
```

El servidor queda escuchando en `0.0.0.0:8080` y se reconecta automáticamente a MQTT si pierde la conexión.

### Compilar el frontend (desde Windows)

```bash
cd frontend
npm install        # solo la primera vez
npm run build      # genera ../static/
```

### Subir cambios a la Raspberry Pi

```bash
# Copiar el build y el backend actualizado
scp -r static/ main.py pi@192.168.4.1:~/dashboard/

# Reiniciar el backend en la Pi
ssh pi@192.168.4.1 "pkill -f main.py; cd ~/dashboard && python main.py &"
```

### Acceder al dashboard

Conectarse a la red WiFi `CintaTransportadora` y abrir:

```
http://192.168.4.1:8080
```

Múltiples dispositivos pueden conectarse simultáneamente; todos ven el mismo estado en tiempo real.

---

## Conexión de red

| Parámetro        | Valor                      |
|------------------|----------------------------|
| SSID             | `CintaTransportadora`      |
| Contraseña WiFi  | `cinta2026`                |
| IP Raspberry Pi  | `192.168.4.1`              |
| Dashboard        | `http://192.168.4.1:8080`  |
| MQTT broker      | `192.168.4.1:1883`         |

El rango DHCP asigna IPs en `192.168.4.2 – 192.168.4.20`. Pueden conectarse múltiples clientes simultáneamente.

---

## API del backend

| Método | Ruta                        | Descripción                             |
|--------|-----------------------------|-----------------------------------------|
| GET    | `/api/estado`               | Estado actual del sistema               |
| GET    | `/api/stats`                | Estadísticas globales + por hora        |
| GET    | `/api/frutas?limit=N`       | Últimas N clasificaciones               |
| POST   | `/api/control/start`        | Arrancar motor                          |
| POST   | `/api/control/stop`         | Detener motor                           |
| POST   | `/api/control/reset`        | Reiniciar contadores de sesión          |
| POST   | `/api/control/pantalla`     | Ciclar pantalla OLED al siguiente índice |
| POST   | `/api/admin/fruta/{tipo}`   | Simular clasificación (modo admin)      |
| POST   | `/api/admin/motor/{cmd}`    | Simular comando motor (modo admin)      |

---

## Diseño visual

| Variable           | Valor                                      |
|--------------------|--------------------------------------------|
| Fondo base         | `#131c25`                                  |
| Panel              | Gradiente `#1f2d39 → #192530 → #152130`   |
| Acento             | `#2d80b8` (azul SCADA)                     |
| OK / Mala / Verde  | `#3a9650` / `#b03830` / `#b88820`          |
| Tipografía display | Barlow Semi Condensed                       |
| Tipografía body    | Barlow                                      |
| Tipografía mono    | JetBrains Mono                              |
| Border radius      | 7px                                         |
