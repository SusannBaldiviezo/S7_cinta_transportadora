/* ================================================================
 * valvula.c — Control de válvula solenoide vía relé (GPIO 27)
 *
 * La válvula se maneja en una tarea FreeRTOS propia para que el
 * vTaskDelay de 500 ms NUNCA bloquee el hilo del broker MQTT.
 * ================================================================ */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "config.h"
#include "valvula.h"

static const char *TAG = "VALVULA";

/* Handle de la tarea — necesario para enviarle notificaciones */
static TaskHandle_t tarea_valvula_handle = NULL;

/* Cliente MQTT compartido (se asigna desde mqtt_handler) */
extern esp_mqtt_client_handle_t cliente_mqtt;

/* ────────────────────────────────────────────────────────────────
 * Tarea privada: espera una notificación, activa el relé 500 ms
 * y publica confirmación por MQTT.
 * ──────────────────────────────────────────────────────────────── */
static void tarea_valvula(void *arg)
{
    while (1) {
        /* Bloquea aquí hasta recibir vTaskNotifyGive */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, ">>> Disparando cilindro");
        gpio_set_level(PIN_VALVULA, 1);
        vTaskDelay(pdMS_TO_TICKS(VALVULA_MS));
        gpio_set_level(PIN_VALVULA, 0);
        ESP_LOGI(TAG, ">>> Cilindro retraído");

        if (cliente_mqtt) {
            esp_mqtt_client_publish(cliente_mqtt, TOPIC_VALVULA, "disparada", 0, 1, 0);
        }
    }
}

/* ────────────────────────────────────────────────────────────────
 * API pública
 * ──────────────────────────────────────────────────────────────── */
void valvula_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_VALVULA),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = 0,
        .pull_down_en = 0,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(PIN_VALVULA, 0);

    /* Crear la tarea dedicada (stack 4 KB, prioridad 5) */
    xTaskCreate(tarea_valvula, "valvula_task", 4096, NULL, 5, &tarea_valvula_handle);
    ESP_LOGI(TAG, "Válvula inicializada");
}

void valvula_disparar(void)
{
    ESP_LOGI(TAG, "valvula_disparar() llamado — notificando tarea");
    if (tarea_valvula_handle != NULL) {
        xTaskNotifyGive(tarea_valvula_handle);
    } else {
        ESP_LOGE(TAG, "ERROR: tarea_valvula_handle es NULL!");
    }
}
