# Guía de instalación y configuración

## Requisitos previos

- Raspberry Pi con RPi OS Lite 64-bit
- 2x ESP32 (WROOM-32 o similar)
- PC/Laptop con Python 3.10+
- ESP-IDF v5.0+ instalado
- Cables Dupont, BTS7960, relé 5V, LCD 16x2 I2C

## Paso 1 — Configurar la Raspberry Pi

Ver guía completa: [docs/05_Instalacion_RPi.md](docs/05_Instalacion_RPi.md)

Resumen:
```bash
# SSH a la RPi
ssh raspi@192.168.1.100

# Instalar Mosquitto
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto

# Configurar Mosquitto para conexiones externas
echo "listener 1883\nallow_anonymous true" | sudo tee -a /etc/mosquitto/mosquitto.conf
sudo systemctl restart mosquitto
```

## Paso 2 — Configurar Access Point en la RPi

Ver guía completa: [docs/08_AccessPoint_RPi.md](docs/08_AccessPoint_RPi.md)

```bash
sudo nmcli con add type wifi ifname wlan0 con-name CintaAP autoconnect yes ssid CintaTransportadora
sudo nmcli con modify CintaAP 802-11-wireless.mode ap ipv4.method shared ipv4.addresses 192.168.4.1/24
sudo nmcli con modify CintaAP wifi-sec.key-mgmt wpa-psk wifi-sec.psk "cinta2026"
sudo nmcli con up CintaAP
```

Red resultante:
- **SSID:** `CintaTransportadora`
- **Contraseña:** `cinta2026`
- **IP del broker MQTT:** `192.168.4.1:1883`

## Paso 3 — Flashear ESP32 Motor

```bash
cd esp32/motor_controller
idf.py set-target esp32
idf.py build
idf.py -p COM3 flash monitor   # Windows: COM3 | Linux: /dev/ttyUSB0
```

Verificar en monitor serial:
```
I (XXXX) WIFI: IP obtenida: 192.168.4.XX
I (XXXX) MQTT: Conectado al broker
```

## Paso 4 — Flashear ESP32 Consola

```bash
cd esp32/console_esp32
idf.py set-target esp32
idf.py build
idf.py -p COM4 flash monitor   # Windows: COM4 | Linux: /dev/ttyUSB1
```

La LCD debe mostrar:
```
OK:0     Rej:0
Estado: esperando
```

## Paso 5 — Instalar dependencias Python

```bash
pip install -r requirements/rpi_requirements.txt
pip install -r requirements/ml_requirements.txt
```

## Paso 6 — Probar con simulador (sin OpenCV)

```bash
# Conectar PC a la red CintaTransportadora
python scripts/simulador_vision.py
```

## Paso 7 — Módulo de visión artificial

Ver [docs/09_Vision_Artificial.md](docs/09_Vision_Artificial.md) (en desarrollo)

```bash
# Próximamente — Semana 4
python rpi/src/vision/clasificador.py
```
