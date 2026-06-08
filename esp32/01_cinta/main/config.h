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

/* ───── SERVOS → CLASIFICACIÓN (MECANISMO DESVÍO) ───── */
#define PIN_SERVO_A      GPIO_NUM_19  // Servo A (izquierda = podrida)
#define PIN_SERVO_B      GPIO_NUM_17  // Servo B (derecha   = verde)

/* ───── ÁNGULOS DE SERVO (° grados) ───── */
/*   maduro   → 45°  (derecha)                              */
/*   pintón/verde → 90° (centro, reposo)                    */
/*   podrido  → 135° (izquierda)                            */
#define SERVO_ANG_CENTRO      90   // pintón / verde / reposo
#define SERVO_ANG_DERECHA     45   // maduro  (derecha)
#define SERVO_ANG_IZQUIERDA  135   // podrido (izquierda)
#define SERVO_B_ESPEJO     false   // true si los servos estan montados enfrentados

/* ───── CANALES LEDC PARA SERVOS ───── */
#define SERVO_TIMER        LEDC_TIMER_1
#define SERVO_CANAL_A      LEDC_CHANNEL_2
#define SERVO_CANAL_B      LEDC_CHANNEL_3
#define SERVO_FREQ_HZ      50           // 50 Hz estándar para servos
#define SERVO_RESOL        LEDC_TIMER_13_BIT  // 8192 counts -> resolucion fina
#define SERVO_PULSE_MIN_US 500           // us a 0 grados
#define SERVO_PULSE_MAX_US 2400          // us a 180 grados

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

/* ───── TIEMPOS ───── */
/* Tiempo que tarda la fruta en pasar por el mecanismo      */
/* después de que el servo se movió.                        */
/* Ajustar según velocidad real de la cinta.                */
#define FRUTA_TRANSITO_MS   6000   // 6 s hasta que la fruta sale del mecanismo
#define VALVULA_MS           500   // tiempo en ms que el servo mantiene la posicion
