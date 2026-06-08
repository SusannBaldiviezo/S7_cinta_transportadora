"""
Sistema Clasificador de Frutas — Backend v3
Sirve el frontend React desde /static
"""
import csv, json, threading, time
from collections import Counter
from datetime import datetime, timedelta
from pathlib import Path

import paho.mqtt.client as mqtt
import uvicorn
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse, JSONResponse
from fastapi.staticfiles import StaticFiles

# ── Config
MQTT_BROKER = "192.168.4.1"
MQTT_PORT   = 1883
HTTP_PORT   = 8080
STATIC_DIR  = Path("static")
DATA_DIR    = Path("data")
CSV_FILE    = DATA_DIR / "frutas.csv"
CSV_HEADERS = ["timestamp", "resultado", "motor"]

# ── Estado
estado = {
    "motor":        "detenido",
    "ultima":       None,
    "ts":           None,
    "mqtt_ok":      False,
    # Contadores de sesión — 4 estados (se resetean con /api/control/reset)
    "sesion":       {"aceptadas": 0, "verdes": 0, "pintones": 0, "podridas": 0},
    # Consecutivos no-aceptada → alerta si >= 3
    "consecutivos": 0,
    "alerta":       False,
    # Pantalla activa en la consola (0=contadores, 1=diagnóstico, 2=alerta)
    "pantalla":     0,
}

# ── Cámaras en vivo (volatile memory)
camaras_frames = {}  # {nombre: base64_jpeg}
camaras_stats = {
    "fps_global": 0,
    "camaras": {
        "cam1_cenital": {"fps": 0, "latencia_ms": 0},
        "cam2_lateral": {"fps": 0, "latencia_ms": 0},
        "cam3_posterior": {"fps": 0, "latencia_ms": 0},
    },
    "timestamp": 0,
}

csv_lock      = threading.Lock()
estado_lock   = threading.Lock()
camaras_lock  = threading.Lock()

DATA_DIR.mkdir(exist_ok=True)
if not CSV_FILE.exists():
    with open(CSV_FILE, "w", newline="", encoding="utf-8") as f:
        csv.writer(f).writerow(CSV_HEADERS)

# ── CSV
def append_row(resultado, motor):
    with csv_lock:
        with open(CSV_FILE, "a", newline="", encoding="utf-8") as f:
            csv.writer(f).writerow([
                datetime.now().strftime("%Y-%m-%d %H:%M:%S"), resultado, motor
            ])

def read_all():
    with csv_lock:
        if not CSV_FILE.exists():
            return []
        with open(CSV_FILE, "r", encoding="utf-8") as f:
            return list(csv.DictReader(f))

# ── MQTT
_V2 = hasattr(mqtt, "CallbackAPIVersion")
if _V2:
    _client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id="dashboard_pc")
else:
    _client = mqtt.Client(client_id="dashboard_pc")

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        with estado_lock: estado["mqtt_ok"] = True
        client.subscribe([
            ("fruta/estado",    1),
            ("sistema/motor",   1),
            ("operario/comando",0),
            ("operario/pantalla",0),
            ("operario/reset",  0),
            ("consola/pantalla",0),  # BTN_PANTALLA del ESP32
            ("consola/reset",   0),  # BTN_RESET del ESP32
        ])
        print("✅ MQTT conectado")

def on_disconnect(client, userdata, rc):
    with estado_lock: estado["mqtt_ok"] = False
    print("⚠️  MQTT desconectado")

def on_message(client, userdata, msg):
    topic   = msg.topic
    payload = msg.payload.decode(errors="replace").strip()

    if topic == "fruta/estado":
        with estado_lock:
            estado["ultima"] = payload
            estado["ts"]     = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            snap             = estado["motor"]
            # Contador de sesión
            if payload in estado["sesion"]:
                estado["sesion"][payload] += 1
            # Consecutivos: aceptada resetea, verde/pinton/podrida acumulan
            if payload == "aceptada":
                estado["consecutivos"] = 0
                estado["alerta"]       = False
            else:  # verde, pinton, podrida son rechazos
                estado["consecutivos"] += 1
                if estado["consecutivos"] >= 3:
                    estado["alerta"] = True
        append_row(payload, snap)
        print(f"  🍎 {payload}  (consec={estado['consecutivos']}, alerta={estado['alerta']})")

    elif topic == "sistema/motor":
        with estado_lock: estado["motor"] = payload
        print(f"  ⚙️  motor/{payload}")

    elif topic == "operario/comando":
        # Botón físico del ESP32 o cualquier cliente → actualizar estado inmediatamente
        with estado_lock:
            if payload == "iniciar":
                estado["motor"] = "iniciando"
            elif payload == "detener":
                estado["motor"] = "deteniendo"
        print(f"  🎮  comando/{payload}")

    elif topic == "operario/pantalla":
        try:
            p = int(payload) % 3
            with estado_lock: estado["pantalla"] = p
            print(f"  📺  pantalla/{p}")
        except: pass

    elif topic == "operario/reset":
        with estado_lock:
            estado["sesion"]       = {"aceptadas": 0, "verdes": 0, "pintones": 0, "podridas": 0}
            estado["consecutivos"] = 0
            estado["alerta"]       = False
        print("  🔄 Reset por MQTT")

    # ── Mensajes del ESP32 Consola (botones físicos) ──────────────
    elif topic == "consola/pantalla":
        try:
            p = int(payload) % 3
            with estado_lock: estado["pantalla"] = p
            print(f"  📺  [ESP32→Dashboard] pantalla/{p}")
        except: pass

    elif topic == "consola/reset":
        with estado_lock:
            estado["sesion"]       = {"aceptadas": 0, "verdes": 0, "pintones": 0, "podridas": 0}
            estado["consecutivos"] = 0
            estado["alerta"]       = False
        print("  🔄  [ESP32→Dashboard] Reset físico recibido")

_client.on_connect    = on_connect
_client.on_disconnect = on_disconnect
_client.on_message    = on_message

def run_mqtt():
    while True:
        try:
            _client.connect(MQTT_BROKER, MQTT_PORT, keepalive=30)
            _client.loop_forever()
        except Exception as e:
            print(f"MQTT: {e} — reintentando 5s")
            time.sleep(5)

# ── FastAPI
app = FastAPI(title="Cinta API v3")
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])

@app.get("/api/estado")
async def api_estado():
    with estado_lock: return JSONResponse(dict(estado))

@app.get("/api/frutas")
async def api_frutas(limit: int = 100):
    rows = read_all()
    return list(reversed(rows[-limit:]))

@app.get("/api/stats")
async def api_stats():
    rows   = read_all()
    total  = len(rows)
    counts = Counter(r["resultado"] for r in rows)
    now    = datetime.now()
    horas  = [(now - timedelta(hours=i)).strftime("%H:00") for i in range(11,-1,-1)]
    ph     = {h: {"aceptada":0,"verde":0,"pinton":0,"podrida":0} for h in horas}
    for r in rows:
        try:
            ts = datetime.strptime(r["timestamp"], "%Y-%m-%d %H:%M:%S")
            if (now - ts).total_seconds() <= 43200:
                hk = ts.strftime("%H:00")
                if hk in ph and r["resultado"] in ph[hk]:
                    ph[hk][r["resultado"]] += 1
        except: pass
    return {
        "total":     total,
        "aceptadas": counts.get("aceptada",0),
        "verdes":    counts.get("verde",0),
        "pintones":  counts.get("pinton",0),
        "podridas":  counts.get("podrida",0),
        "por_hora":  [{"hora":h,**v} for h,v in ph.items()],
    }

@app.post("/api/control/{action}")
async def api_control(action: str):
    if action == "start":
        _client.publish("operario/comando", "iniciar")
        # Actualización optimista: no esperar el roundtrip MQTT
        with estado_lock:
            estado["motor"] = "activo"
    elif action == "stop":
        _client.publish("operario/comando", "detener")
        with estado_lock:
            estado["motor"] = "detenido"
    elif action == "reset":
        with estado_lock:
            estado["sesion"]       = {"aceptadas": 0, "verdes": 0, "pintones": 0, "podridas": 0}
            estado["consecutivos"] = 0
            estado["alerta"]       = False
        # Notificar al ESP32 para que también resetee sus contadores locales
        _client.publish("operario/reset", "reset", qos=1)
        print("  🔄 Contadores de sesión reseteados")
    elif action == "pantalla":
        with estado_lock:
            estado["pantalla"] = (estado["pantalla"] + 1) % 3
            p = estado["pantalla"]
        _client.publish("operario/pantalla", str(p), qos=1)
        print(f"  📺  pantalla → {p}")
    return {"ok": True}

@app.post("/api/admin/fruta/{resultado}")
async def admin_fruta(resultado: str):
    if resultado not in ("aceptada","verde","pinton","podrida"):
        return JSONResponse({"ok":False},status_code=400)
    _client.publish("fruta/estado", resultado)
    with estado_lock:
        estado["ultima"] = resultado
        estado["ts"]     = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        snap             = estado["motor"]
        if resultado in estado["sesion"]:
            estado["sesion"][resultado] += 1
        if resultado == "aceptada":
            estado["consecutivos"] = 0
            estado["alerta"]       = False
        else:
            estado["consecutivos"] += 1
            if estado["consecutivos"] >= 3:
                estado["alerta"] = True
    append_row(resultado, snap)
    print(f"  🧪 [TEST] fruta/{resultado}")
    return {"ok": True}

@app.post("/api/admin/motor/{cmd}")
async def admin_motor(cmd: str):
    if cmd not in ("iniciar","detener"):
        return JSONResponse({"ok":False},status_code=400)
    _client.publish("operario/comando", cmd)
    with estado_lock:
        estado["motor"] = "activo" if cmd=="iniciar" else "detenido"
    print(f"  🧪 [TEST] motor/{cmd}")
    return {"ok": True}

# ── Cámaras (endpoints para frames en vivo)
@app.post("/api/camaras/frame")
async def camaras_frame(data: dict):
    """Recibe un frame base64 del script de inferencia."""
    try:
        nombre = data.get("nombre", "desconocido")
        frame_b64 = data.get("frame", "")
        latencia_ms = int(data.get("latencia_ms", 0))

        with camaras_lock:
            camaras_frames[nombre] = frame_b64
            if nombre in camaras_stats["camaras"]:
                camaras_stats["camaras"][nombre]["latencia_ms"] = latencia_ms
    except Exception as e:
        print(f"  ⚠️  Error recibiendo frame: {e}")

    return {"ok": True}

@app.post("/api/camaras/stats")
async def recibir_camaras_stats(data: dict):  # <-- Nombre cambiado aquí
    """Recibe estadísticas de FPS y latencia del script de inferencia."""
    try:
        with camaras_lock:
            camaras_stats.update(data)
    except Exception as e:
        print(f"  ⚠️  Error recibiendo stats: {e}")

    return {"ok": True}

@app.get("/api/camaras/frames")
async def camaras_get_frames():
    """Devuelve los últimos frames base64 de todas las cámaras."""
    with camaras_lock:
        return JSONResponse(dict(camaras_frames))

@app.get("/api/camaras/stats")
async def camaras_get_stats():
    """Devuelve estadísticas de FPS y latencia."""
    with camaras_lock:
        return JSONResponse(camaras_stats)

# ── Static (React build)
if STATIC_DIR.exists():
    app.mount("/assets", StaticFiles(directory=STATIC_DIR/"assets"), name="assets")

@app.get("/{full_path:path}")
async def spa_fallback(full_path: str):
    index = STATIC_DIR / "index.html"
    if index.exists():
        return FileResponse(index)
    return JSONResponse({"error":"Frontend no compilado. Ejecutá: cd frontend && npm install && npm run build"},status_code=503)

if __name__ == "__main__":
    print("="*52)
    print("  🍎  Clasificador de Frutas  v3")
    print(f"  Dashboard  →  http://localhost:{HTTP_PORT}")
    print(f"  MQTT       →  {MQTT_BROKER}:{MQTT_PORT}")
    print("="*52)
    threading.Thread(target=run_mqtt, daemon=True).start()
    uvicorn.run(app, host="0.0.0.0", port=HTTP_PORT, log_level="warning")