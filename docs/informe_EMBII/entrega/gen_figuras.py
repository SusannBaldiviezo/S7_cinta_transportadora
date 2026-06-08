# -*- coding: utf-8 -*-
"""Genera las graficas de respuesta del subsistema embebido (PNG @ 200 dpi)."""
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

plt.rcParams.update({
    "font.size": 11, "axes.grid": True, "grid.alpha": 0.3,
    "axes.spines.top": False, "axes.spines.right": False,
    "figure.dpi": 200, "savefig.bbox": "tight",
})
AZUL = "#1f4e79"; NARANJA = "#c55a11"; VERDE = "#2e7d32"; ROJO = "#c0392b"

# ----------------------------------------------------------------------
# 1) Senal PWM del LEDC a 10 kHz, duty = 150/255 (58.8 %)
# ----------------------------------------------------------------------
f = 10_000.0            # Hz (PWM_FREQ)
T = 1.0 / f             # 100 us
duty = 150 / 255        # MOTOR_VEL_NORMAL
t = np.linspace(0, 3 * T, 3000) * 1e6   # us
pwm = (np.mod(t * 1e-6, T) < duty * T).astype(float) * 3.3
fig, ax = plt.subplots(figsize=(7, 2.7))
ax.plot(t, pwm, color=AZUL, lw=1.8)
ax.set_xlabel(r"Tiempo [$\mu$s]"); ax.set_ylabel("Tensión GPIO [V]")
ax.set_yticks([0, 3.3]); ax.set_ylim(-0.3, 3.9)
ax.axhline(3.3, color="gray", ls=":", lw=0.8)
ax.annotate(r"$T=100\ \mu s\ (10\ kHz)$", xy=(T*1e6, 3.5), xytext=(T*1e6*1.05, 3.6))
ax.annotate(f"Duty = 150/255 = {duty*100:.1f} %", xy=(duty*T*1e6, 1.6),
            xytext=(duty*T*1e6+8, 1.0),
            arrowprops=dict(arrowstyle="->", color=NARANJA), color=NARANJA)
ax.set_title("Señal PWM en RPWM (GPIO25) — canal LEDC")
fig.savefig("Figuras/pwm_10khz.png"); plt.close(fig)

# ----------------------------------------------------------------------
# 2) Mapa duty -> velocidad (LEDC 8 bits) y punto de operacion
# ----------------------------------------------------------------------
duty8 = np.arange(0, 256)
fig, ax = plt.subplots(figsize=(6.4, 3.0))
ax.plot(duty8, duty8 / 255 * 100, color=AZUL, lw=2)
ax.axvline(150, color=NARANJA, ls="--", lw=1.5)
ax.axhline(150/255*100, color=NARANJA, ls="--", lw=1.5)
ax.plot(150, 150/255*100, "o", color=ROJO, ms=8)
ax.annotate("Punto de arranque\nMOTOR_VEL_NORMAL=150",
            xy=(150, 150/255*100), xytext=(40, 70),
            arrowprops=dict(arrowstyle="->", color=ROJO))
ax.set_xlabel("Valor de duty (0–255, resolución 8 bits)")
ax.set_ylabel("Ciclo de trabajo [%]")
ax.set_xlim(0, 255); ax.set_ylim(0, 100)
ax.set_title("Cuantización del PWM: duty de 8 bits → tensión media en el motor")
fig.savefig("Figuras/ledc_duty.png"); plt.close(fig)

# ----------------------------------------------------------------------
# 3) Cronograma del disparo de valvula (relé + cilindro)
#    Evento MQTT "rechazada" -> tarea FreeRTOS -> GPIO27 ON 500 ms -> publish
# ----------------------------------------------------------------------
fig, ax = plt.subplots(figsize=(7.4, 3.4))
def step(y, segs, color, label):
    for (x0, x1, lvl) in segs:
        ax.plot([x0, x1], [y + lvl*0.7, y + lvl*0.7], color=color, lw=2.2)
        ax.plot([x0, x0], [y, y + 0.7], color=color, lw=0.8, alpha=0.5)
        ax.plot([x1, x1], [y, y + 0.7], color=color, lw=0.8, alpha=0.5)
    ax.text(-60, y + 0.35, label, ha="right", va="center")

# t en ms
ax.axvline(0, color="gray", ls=":", lw=1)
ax.text(0, 3.95, 'MQTT: fruta/estado = "rechazada"', ha="center", fontsize=9, color=ROJO)
step(3.0, [(-200, 0, 0), (0, 0, 1)], ROJO, "Evento MQTT")     # impulso
ax.plot([0], [3.0+0.7], "v", color=ROJO)
step(2.0, [(-200, 5, 0), (5, 505, 1), (505, 700, 0)], VERDE, "GPIO27 (relé)")
step(1.0, [(-200, 25, 0), (25, 530, 1), (530, 700, 0)], AZUL, "Cilindro\n(neumático)")
ax.plot([520], [3.0+0.7], "^", color=NARANJA)
ax.text(520, 3.95, 'publish\n"disparada"', ha="center", fontsize=8, color=NARANJA)
ax.annotate("", xy=(505, 2.0+0.78), xytext=(5, 2.0+0.78),
            arrowprops=dict(arrowstyle="<->", color="black"))
ax.text(255, 2.0+0.95, "VALVULA_MS = 500 ms", ha="center", fontsize=9)
ax.set_xlim(-220, 720); ax.set_ylim(0.5, 4.2)
ax.set_yticks([]); ax.set_xlabel("Tiempo [ms]")
ax.set_title("Cronograma del disparo no bloqueante de la válvula (tarea FreeRTOS)")
ax.grid(False)
fig.savefig("Figuras/cronograma_valvula.png"); plt.close(fig)

# ----------------------------------------------------------------------
# 4) Presupuesto de latencia extremo-a-extremo (cámara -> expulsión)
# ----------------------------------------------------------------------
etapas = ["Captura\n+ inferencia\n(PC)", "publish\nMQTT (PC)", "Broker\nMosquitto\n(RPi)",
          "Reacción\nESP32", "Apertura\nrelé+válvula"]
vals = [1500, 5, 8, 10, 25]   # ms (tipicos/spec)
colores = [AZUL, NARANJA, "#7f7f7f", VERDE, ROJO]
fig, ax = plt.subplots(figsize=(7.2, 3.2))
left = 0
for e, v, c in zip(etapas, vals, colores):
    ax.barh(0, v, left=left, color=c, edgecolor="white")
    ax.text(left + v/2, 0, f"{v} ms", ha="center", va="center",
            color="white", fontsize=9, fontweight="bold")
    ax.text(left + v/2, 0.55, e, ha="center", va="bottom", fontsize=8)
    left += v
ax.set_xlim(0, left*1.02); ax.set_ylim(-0.6, 1.2)
ax.set_yticks([]); ax.set_xlabel("Tiempo acumulado [ms]")
ax.set_title(f"Presupuesto de latencia extremo a extremo  (total ≈ {left} ms < 2 s, RNF-01)")
ax.grid(axis="x", alpha=0.3)
fig.savefig("Figuras/latencia_e2e.png"); plt.close(fig)

print("OK figuras:", *__import__("os").listdir("Figuras"))
