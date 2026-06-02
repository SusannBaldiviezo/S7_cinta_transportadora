/* ================================================================
 * motor.c — Implementación del control del motor BTS7960
 * ================================================================ */

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "config.h"
#include "motor.h"

static const char *TAG = "MOTOR";

void motor_init(void)
{
    // Pines de habilitación (R_EN y L_EN) como salida digital
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_R_EN) | (1ULL << PIN_L_EN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = 0,
        .pull_down_en = 0,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // Activar ambos lados del puente H
    gpio_set_level(PIN_R_EN, 1);
    gpio_set_level(PIN_L_EN, 1);

    // Configurar timer PWM
    ledc_timer_config_t pwm_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = PWM_TIMER,
        .duty_resolution = PWM_RESOL,
        .freq_hz         = PWM_FREQ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&pwm_timer);

    // Canal PWM giro adelante (RPWM → GPIO 25)
    ledc_channel_config_t pwm_R = {
        .gpio_num   = PIN_RPWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = PWM_CANAL_R,
        .timer_sel  = PWM_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&pwm_R);

    // Canal PWM giro atrás (LPWM → GPIO 26)
    ledc_channel_config_t pwm_L = {
        .gpio_num   = PIN_LPWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = PWM_CANAL_L,
        .timer_sel  = PWM_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&pwm_L);

    ESP_LOGI(TAG, "Motor inicializado");
}

void motor_avanzar(int velocidad)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CANAL_L, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CANAL_L);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CANAL_R, velocidad);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CANAL_R);
    ESP_LOGI(TAG, "Avanzando @ velocidad %d", velocidad);
}

void motor_detener(void)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CANAL_R, 0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CANAL_L, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CANAL_R);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CANAL_L);
    ESP_LOGI(TAG, "Motor detenido");
}
