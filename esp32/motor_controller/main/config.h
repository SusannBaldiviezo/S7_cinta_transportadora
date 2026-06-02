#pragma once

/* ================================================================
 * config.h — Configuración central del sistema
 * ================================================================ */

/* ───── RED ───── */
#define WIFI_SSID      "CintaTransportadora"
#define WIFI_PASSWORD  "cinta2026"
#define MQTT_BROKER    "mqtt://192.168.4.1:1883"

/* ───── PINES BTS7960 ───── */
#define PIN_RPWM       GPIO_NUM_25   // PWM giro adelante
#define PIN_LPWM       GPIO_NUM_26   // PWM giro atrás
#define PIN_R_EN       GPIO_NUM_32   // Habilita lado derecho
#define PIN_L_EN       GPIO_NUM_33   // Habilita lado izquierdo

/* ───── RELÉ → VÁLVULA SOLENOIDE ───── */
#define PIN_VALVULA    GPIO_NUM_27

/* ───── CONFIGURACIÓN PWM ───── */
#define PWM_CANAL_R    LEDC_CHANNEL_0
#define PWM_CANAL_L    LEDC_CHANNEL_1
#define PWM_TIMER      LEDC_TIMER_0
#define PWM_FREQ       10000             // 10 kHz — suave para BTS7960
#define PWM_RESOL      LEDC_TIMER_8_BIT // 0–255

/* ───── VELOCIDAD DEL MOTOR ───── */
#define MOTOR_VEL_NORMAL  150   // duty cycle de arranque (0–255)

/* ───── TÓPICOS MQTT ───── */
#define TOPIC_FRUTA    "fruta/estado"
#define TOPIC_COMANDO  "operario/comando"
#define TOPIC_MOTOR    "sistema/motor"
#define TOPIC_VALVULA  "sistema/valvula"

/* ───── TIEMPO VÁLVULA ───── */
#define VALVULA_MS     500   // tiempo en ms que permanece abierta
