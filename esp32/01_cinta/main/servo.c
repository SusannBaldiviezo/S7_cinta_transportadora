/* ================================================================
 * servo.c — Control de 2 servomotores sincronizados (GPIO 19 y 17)
 *
 * ESTADOS DEL MECANISMO DE DESVÍO:
 *   maduro   → 45°  (derecha)   — fruta madura
 *   pintón/verde → 90° (centro) — fruta pintona o verde (reposo)
 *   podrido  → 135° (izquierda) — fruta podrida
 *
 * LÓGICA DE TIMING:
 *   1. Llega señal MQTT con el tipo de fruta.
 *   2. Si la posición objetivo es DISTINTA a la actual → mover inmediatamente.
 *      Si ya está en la posición correcta → no hacer nada (evita volver de ida y vuelta).
 *   3. Esperar FRUTA_TRANSITO_MS (6 s) para que la fruta pase el mecanismo.
 *   4. Volver al centro (SERVO_ANG_CENTRO = 90°) listo para la siguiente fruta.
 *
 * Los servos se manejan en una tarea FreeRTOS propia para que el
 * movimiento NUNCA bloquee el hilo del broker MQTT.
 * ================================================================ */

#include <math.h>
#include <limits.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "config.h"
#include "servo.h"

static const char *TAG = "SERVO";

/* Handle de la tarea — necesario para enviarle notificaciones */
static TaskHandle_t tarea_servo_handle = NULL;

/* Posicion actual (en grados) de ambos servos (pos_a = referencia) */
static int pos_a = SERVO_ANG_CENTRO;
static int pos_b = SERVO_ANG_CENTRO;

/* Ultima accion ejecutada — para evitar mover si ya estamos en posicion */
static uint32_t ultimo_estado = 0;   /* 0 = ninguno aun */

/* Cliente MQTT compartido (se asigna desde mqtt_handler) */
extern esp_mqtt_client_handle_t cliente_mqtt;

/* ── Codigos de accion ── */
#define ACCION_MADURO   1   /* 45°  derecha */
#define ACCION_PINTON   2   /* 90°  centro  */
#define ACCION_PODRIDO  3   /* 135° izquierda */

/* ────────────────────────────────────────────────────────────────
 * Convierte grados (0–180) a duty cycle para LEDC_TIMER_13_BIT
 * Periodo servo = 20 000 µs (50 Hz)
 * 8192 counts = 20 000 µs
 * duty = counts * (pulse_us / 20000)
 * ──────────────────────────────────────────────────────────────── */
static uint32_t angulo_a_duty(int grados)
{
    if (grados < 0)   grados = 0;
    if (grados > 180) grados = 180;

    uint32_t pulse_us = SERVO_PULSE_MIN_US +
                        (uint32_t)(grados) * (SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US) / 180;

    return (pulse_us * 8192U) / 20000U;
}

/* Angulo del servo B segun modo espejo */
static int angulo_b(int ang)
{
    return SERVO_B_ESPEJO ? (180 - ang) : ang;
}

/* ────────────────────────────────────────────────────────────────
 * Escribe el angulo directamente (sin suavizado) en ambos servos
 * ──────────────────────────────────────────────────────────────── */
static void servo_set_raw(int a, int b)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, SERVO_CANAL_A, angulo_a_duty(a));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, SERVO_CANAL_A);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, SERVO_CANAL_B, angulo_a_duty(b));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, SERVO_CANAL_B);
    pos_a = a;
    pos_b = b;
}

/* ────────────────────────────────────────────────────────────────
 * Mueve AMBOS servos de forma sincronizada y suave
 * ──────────────────────────────────────────────────────────────── */
static void mover_sync(int objetivo_a, int objetivo_b)
{
    int inicio_a = pos_a;
    int inicio_b = pos_b;

    int pasos = abs(objetivo_a - inicio_a);
    if (abs(objetivo_b - inicio_b) > pasos)
        pasos = abs(objetivo_b - inicio_b);

    if (pasos == 0) {
        servo_set_raw(objetivo_a, objetivo_b);
        return;
    }

    for (int i = 1; i <= pasos; i++) {
        int a = inicio_a + (objetivo_a - inicio_a) * i / pasos;
        int b = inicio_b + (objetivo_b - inicio_b) * i / pasos;
        servo_set_raw(a, b);
        vTaskDelay(pdMS_TO_TICKS(12)); /* ~12ms por paso: suave y preciso */
    }
}

/* ────────────────────────────────────────────────────────────────
 * Tarea privada: espera notificacion, actua y espera transito
 *
 * FLUJO por señal recibida:
 *   1. Recibir accion por xTaskNotifyWait
 *   2. Si el objetivo es IGUAL al estado actual → no mover nada
 *   3. Si es diferente → mover servos AL INSTANTE al angulo objetivo
 *   4. Esperar FRUTA_TRANSITO_MS para que la fruta cruce el mecanismo
 *   5. Volver al centro (pintón/reposo)
 *   6. Actualizar ultimo_estado → listo para la siguiente señal
 * ──────────────────────────────────────────────────────────────── */
static void tarea_servo(void *arg)
{
    uint32_t accion = 0;

    /* Posicion inicial: centro */
    servo_set_raw(SERVO_ANG_CENTRO, angulo_b(SERVO_ANG_CENTRO));
    ultimo_estado = ACCION_PINTON;

    while (1) {
        xTaskNotifyWait(0x00, ULONG_MAX, &accion, portMAX_DELAY);

        /* ── Determinar angulo objetivo ── */
        int objetivo = SERVO_ANG_CENTRO;
        const char *mqtt_payload = "centro";
        const char *nombre = "pintón/verde";

        if (accion == ACCION_MADURO) {
            objetivo      = SERVO_ANG_DERECHA;
            mqtt_payload  = "desvio_der";
            nombre        = "MADURO (derecha 45°)";
        } else if (accion == ACCION_PINTON) {
            objetivo      = SERVO_ANG_CENTRO;
            mqtt_payload  = "centro";
            nombre        = "PINTÓN/VERDE (centro 90°)";
        } else if (accion == ACCION_PODRIDO) {
            objetivo      = SERVO_ANG_IZQUIERDA;
            mqtt_payload  = "desvio_izq";
            nombre        = "PODRIDO (izquierda 135°)";
        } else {
            /* accion desconocida, ignorar */
            continue;
        }

        /* ── Si ya estamos en la posicion correcta, no hacer nada ── */
        if (accion == ultimo_estado && pos_a == objetivo) {
            ESP_LOGI(TAG, ">>> Ya en posicion %s — sin movimiento innecesario", nombre);
            continue;   /* esperar la siguiente señal directamente */
        }

        /* ── Mover inmediatamente al angulo objetivo ── */
        ESP_LOGI(TAG, ">>> Moviendo a %s", nombre);
        mover_sync(objetivo, angulo_b(objetivo));
        ultimo_estado = accion;

        /* Publicar confirmacion de movimiento */
        if (cliente_mqtt) {
            esp_mqtt_client_publish(cliente_mqtt, TOPIC_VALVULA, mqtt_payload, 0, 1, 0);
        }

        /* ── Esperar a que la fruta cruce el mecanismo ── */
        ESP_LOGI(TAG, ">>> Esperando %d ms para que pase la fruta...", FRUTA_TRANSITO_MS);
        vTaskDelay(pdMS_TO_TICKS(FRUTA_TRANSITO_MS));

        /* ── Volver al centro listo para la siguiente fruta ── */
        ESP_LOGI(TAG, ">>> Volviendo al centro (reposo)");
        mover_sync(SERVO_ANG_CENTRO, angulo_b(SERVO_ANG_CENTRO));
        ultimo_estado = ACCION_PINTON;   /* ahora estamos en el centro */
    }
}

/* ────────────────────────────────────────────────────────────────
 * API pública
 * ──────────────────────────────────────────────────────────────── */
void servo_init(void)
{
    /* ── Configurar timer LEDC para 50 Hz ── */
    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = SERVO_TIMER,
        .duty_resolution = SERVO_RESOL,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);

    /* ── Canal Servo A (GPIO 19) ── */
    ledc_channel_config_t ch_a = {
        .gpio_num   = PIN_SERVO_A,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = SERVO_CANAL_A,
        .timer_sel  = SERVO_TIMER,
        .duty       = angulo_a_duty(SERVO_ANG_CENTRO),
        .hpoint     = 0,
    };
    ledc_channel_config(&ch_a);

    /* ── Canal Servo B (GPIO 17) ── */
    ledc_channel_config_t ch_b = {
        .gpio_num   = PIN_SERVO_B,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = SERVO_CANAL_B,
        .timer_sel  = SERVO_TIMER,
        .duty       = angulo_a_duty(angulo_b(SERVO_ANG_CENTRO)),
        .hpoint     = 0,
    };
    ledc_channel_config(&ch_b);

    pos_a = SERVO_ANG_CENTRO;
    pos_b = angulo_b(SERVO_ANG_CENTRO);

    /* ── Crear tarea dedicada (stack 4KB, prioridad 5) ── */
    xTaskCreate(tarea_servo, "servo_task", 4096, NULL, 5, &tarea_servo_handle);
    ESP_LOGI(TAG, "Servos inicializados (A=GPIO%d, B=GPIO%d) — reposo en 90°",
             PIN_SERVO_A, PIN_SERVO_B);
}

void servo_disparar_maduro(void)
{
    ESP_LOGI(TAG, "servo_disparar_maduro() llamado");
    if (tarea_servo_handle != NULL) {
        xTaskNotify(tarea_servo_handle, ACCION_MADURO, eSetValueWithOverwrite);
    } else {
        ESP_LOGE(TAG, "ERROR: tarea_servo_handle es NULL!");
    }
}

void servo_disparar_pinton(void)
{
    ESP_LOGI(TAG, "servo_disparar_pinton() llamado");
    if (tarea_servo_handle != NULL) {
        xTaskNotify(tarea_servo_handle, ACCION_PINTON, eSetValueWithOverwrite);
    } else {
        ESP_LOGE(TAG, "ERROR: tarea_servo_handle es NULL!");
    }
}

void servo_disparar_podrido(void)
{
    ESP_LOGI(TAG, "servo_disparar_podrido() llamado");
    if (tarea_servo_handle != NULL) {
        xTaskNotify(tarea_servo_handle, ACCION_PODRIDO, eSetValueWithOverwrite);
    } else {
        ESP_LOGE(TAG, "ERROR: tarea_servo_handle es NULL!");
    }
}
