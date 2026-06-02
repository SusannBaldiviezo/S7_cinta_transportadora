/* ================================================================
 * mqtt_handler.c — Manejador de eventos del cliente MQTT
 * ================================================================ */

#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "config.h"
#include "motor.h"
#include "valvula.h"
#include "mqtt_handler.h"

static const char *TAG = "MQTT";

/* Variables globales compartidas con wifi_manager y valvula */
esp_mqtt_client_handle_t cliente_mqtt = NULL;
bool sistema_activo = false;

void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                        int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event  = event_data;
    esp_mqtt_client_handle_t client = event->client;
    char topic[64];
    char data[64];

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Conectado al broker");
        esp_mqtt_client_subscribe(client, TOPIC_FRUTA,   1);
        esp_mqtt_client_subscribe(client, TOPIC_COMANDO, 1);
        esp_mqtt_client_publish(client, TOPIC_MOTOR, "esp32_online", 0, 1, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "Desconectado del broker");
        break;

    case MQTT_EVENT_DATA:
        snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);
        snprintf(data,  sizeof(data),  "%.*s", event->data_len,  event->data);
        ESP_LOGI(TAG, "[%s] = %s", topic, data);

        /* ── COMANDOS DEL OPERARIO ── */
        if (strcmp(topic, TOPIC_COMANDO) == 0) {
            if (strcmp(data, "iniciar") == 0) {
                sistema_activo = true;
                motor_avanzar(MOTOR_VEL_NORMAL);
                esp_mqtt_client_publish(client, TOPIC_MOTOR, "activo", 0, 1, 0);
            }
            else if (strcmp(data, "detener") == 0) {
                sistema_activo = false;
                motor_detener();
                esp_mqtt_client_publish(client, TOPIC_MOTOR, "detenido", 0, 1, 0);
            }
        }

        /* ── VEREDICTO DE FRUTA ──
           Solo notificamos la tarea de la válvula — NO bloqueamos el hilo MQTT */
        else if (strcmp(topic, TOPIC_FRUTA) == 0 && sistema_activo) {
            if (strcmp(data, "rechazada") == 0) {
                valvula_disparar();
            }
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "Error en cliente MQTT");
        break;

    default:
        break;
    }
}
