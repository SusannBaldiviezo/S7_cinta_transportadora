/* ================================================================
 * ESP32 CONSOLA — Sistema Clasificador de Frutas
 * Hardware: LCD 16x2 I2C + 4 pulsadores + LED
 * Comunicación: WiFi + MQTT (suscriptor + publicador de comandos)
 * ================================================================ */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "mqtt_client.h"
#include "lcd_i2c.h"

/* ───── RED ───── */
#define WIFI_SSID      "CintaTransportadora"
#define WIFI_PASSWORD  "cinta2026"
#define MQTT_BROKER    "mqtt://192.168.4.1:1883"

/* ───── PINES ───── */
#define PIN_SDA        21
#define PIN_SCL        22
#define BTN_INICIAR    GPIO_NUM_14
#define BTN_DETENER    GPIO_NUM_27
#define BTN_PANTALLA   GPIO_NUM_26
#define BTN_RESET      GPIO_NUM_25
#define PIN_LED        GPIO_NUM_2

/* ───── TÓPICOS MQTT ───── */
#define TOPIC_FRUTA    "fruta/estado"
#define TOPIC_COMANDO  "operario/comando"
#define TOPIC_MOTOR    "sistema/motor"
#define TOPIC_VALVULA  "sistema/valvula"

static const char *TAG = "CONSOLA";
static esp_mqtt_client_handle_t cliente_mqtt = NULL;
static SemaphoreHandle_t lcd_mutex = NULL;

/* ───── ESTADO COMPARTIDO ───── */
typedef struct {
    int aceptadas;
    int rechazadas;
    int rechazos_consecutivos;
    char estado_motor[16];   // "activo" / "detenido" / "esperando"
    int pantalla_actual;     // 0 = contadores, 1 = diagnóstico, 2 = alerta
    bool alerta_activa;
    bool wifi_ok;
} estado_t;

static estado_t S = {
    .aceptadas = 0,
    .rechazadas = 0,
    .rechazos_consecutivos = 0,
    .estado_motor = "esperando",
    .pantalla_actual = 0,
    .alerta_activa = false,
    .wifi_ok = false,
};

/* ════════════════════════════════════════════════════════════════
 * RENDERIZAR PANTALLA SEGÚN VISTA ACTUAL
 * ════════════════════════════════════════════════════════════════ */
static void render_pantalla(void)
{
    char linea1[64];
    char linea2[64];

    switch (S.pantalla_actual) {
        case 0: // Contadores
            snprintf(linea1, sizeof(linea1), "OK:%-4d  Rej:%-3d", S.aceptadas, S.rechazadas);
            snprintf(linea2, sizeof(linea2), "Estado: %-8s", S.estado_motor);
            break;
        case 1: // Diagnóstico
            snprintf(linea1, sizeof(linea1), "WiFi: %-10s", S.wifi_ok ? "OK" : "OFFLINE");
            snprintf(linea2, sizeof(linea2), "Total: %-9d", S.aceptadas + S.rechazadas);
            break;
        case 2: // Alerta
            snprintf(linea1, sizeof(linea1), "!!! ALERTA !!!  ");
            snprintf(linea2, sizeof(linea2), "Rechazos: %-6d", S.rechazos_consecutivos);
            break;
        default:
            snprintf(linea1, sizeof(linea1), "N/A             ");
            snprintf(linea2, sizeof(linea2), "N/A             ");
            break;
    }

    if (lcd_mutex != NULL && xSemaphoreTake(lcd_mutex, portMAX_DELAY) == pdTRUE) {
        lcd_set_cursor(0, 0);
        lcd_print(linea1);
        lcd_set_cursor(0, 1);
        lcd_print(linea2);
        xSemaphoreGive(lcd_mutex);
    }
}

/* ════════════════════════════════════════════════════════════════
 * TAREA: leer botones (debounce simple)
 * ════════════════════════════════════════════════════════════════ */
static void tarea_botones(void *param)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << BTN_INICIAR) | (1ULL << BTN_DETENER) |
                        (1ULL << BTN_PANTALLA) | (1ULL << BTN_RESET),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);

    int last1 = 1, last2 = 1, last3 = 1, last4 = 1;

    while (1) {
        int b1 = gpio_get_level(BTN_INICIAR);
        int b2 = gpio_get_level(BTN_DETENER);
        int b3 = gpio_get_level(BTN_PANTALLA);
        int b4 = gpio_get_level(BTN_RESET);

        // Detecta flanco descendente (botón presionado)
        if (last1 == 1 && b1 == 0) {
            ESP_LOGI(TAG, "Botón INICIAR presionado");
            if (cliente_mqtt) {
                esp_mqtt_client_publish(cliente_mqtt, TOPIC_COMANDO, "iniciar", 0, 1, 0);
            }
        }
        if (last2 == 1 && b2 == 0) {
            ESP_LOGI(TAG, "Botón DETENER presionado");
            if (cliente_mqtt) {
                esp_mqtt_client_publish(cliente_mqtt, TOPIC_COMANDO, "detener", 0, 1, 0);
            }
        }
        if (last3 == 1 && b3 == 0) {
            S.pantalla_actual = (S.pantalla_actual + 1) % 3;
            ESP_LOGI(TAG, "Pantalla cambiada a %d", S.pantalla_actual);
            render_pantalla();
        }
        if (last4 == 1 && b4 == 0) {
            ESP_LOGI(TAG, "Contadores reseteados");
            S.aceptadas = 0;
            S.rechazadas = 0;
            S.rechazos_consecutivos = 0;
            S.alerta_activa = false;
            render_pantalla();
        }

        last1 = b1; last2 = b2; last3 = b3; last4 = b4;
        vTaskDelay(pdMS_TO_TICKS(50));  // antirrebote
    }
}

/* ════════════════════════════════════════════════════════════════
 * TAREA: parpadear LED si hay alerta, encendido fijo si WiFi OK
 * ════════════════════════════════════════════════════════════════ */
static void tarea_led(void *param)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << PIN_LED),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);

    bool led_on = false;
    while (1) {
        if (S.alerta_activa) {
            led_on = !led_on;
            gpio_set_level(PIN_LED, led_on);
            vTaskDelay(pdMS_TO_TICKS(300));
        } else {
            gpio_set_level(PIN_LED, S.wifi_ok ? 1 : 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

/* ════════════════════════════════════════════════════════════════
 * HANDLER MQTT
 * ════════════════════════════════════════════════════════════════ */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    char topic[64], data[64];

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT conectado");
        esp_mqtt_client_subscribe(client, TOPIC_FRUTA, 1);
        esp_mqtt_client_subscribe(client, TOPIC_MOTOR, 1);
        esp_mqtt_client_subscribe(client, TOPIC_VALVULA, 1);
        S.wifi_ok = true;
        render_pantalla();
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT desconectado");
        S.wifi_ok = false;
        render_pantalla();
        break;

    case MQTT_EVENT_DATA:
        snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);
        snprintf(data,  sizeof(data),  "%.*s", event->data_len,  event->data);
        ESP_LOGI(TAG, "MQTT [%s] = %s", topic, data);

        // ── ESTADO DEL MOTOR ──
        if (strcmp(topic, TOPIC_MOTOR) == 0) {
            strncpy(S.estado_motor, data, sizeof(S.estado_motor) - 1);
        }

        // ── CLASIFICACIÓN DE FRUTA ──
        else if (strcmp(topic, TOPIC_FRUTA) == 0) {
            if (strcmp(data, "aceptada") == 0) {
                S.aceptadas++;
                S.rechazos_consecutivos = 0;
                S.alerta_activa = false;
            }
            else if (strcmp(data, "rechazada") == 0) {
                S.rechazadas++;
                S.rechazos_consecutivos++;
                if (S.rechazos_consecutivos >= 3) {
                    S.alerta_activa = true;
                    S.pantalla_actual = 2;  // cambia a pantalla de alerta
                }
            }
        }

        render_pantalla();
        break;

    default:
        break;
    }
}

/* ════════════════════════════════════════════════════════════════
 * HANDLER WiFi
 * ════════════════════════════════════════════════════════════════ */
static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        S.wifi_ok = false;
        render_pantalla();
        esp_wifi_connect();
    }
    else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "WiFi conectado. IP: " IPSTR, IP2STR(&event->ip_info.ip));

        esp_mqtt_client_config_t mqtt_cfg = {
            .broker.address.uri = MQTT_BROKER,
        };
        cliente_mqtt = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(cliente_mqtt, ESP_EVENT_ANY_ID,
                                       mqtt_event_handler, NULL);
        esp_mqtt_client_start(cliente_mqtt);
    }
}

static void wifi_init(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

/* ════════════════════════════════════════════════════════════════
 * APP MAIN
 * ════════════════════════════════════════════════════════════════ */
void app_main(void)
{
    ESP_LOGI(TAG, "════════════════════════════════════════");
    ESP_LOGI(TAG, "  ESP32 CONSOLA — Clasificador de Frutas");
    ESP_LOGI(TAG, "════════════════════════════════════════");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Inicializar Mutex para el LCD
    lcd_mutex = xSemaphoreCreateMutex();

    // Iniciar LCD
    lcd_init(PIN_SDA, PIN_SCL);
    if (lcd_mutex != NULL && xSemaphoreTake(lcd_mutex, portMAX_DELAY) == pdTRUE) {
        lcd_print("Iniciando...");
        xSemaphoreGive(lcd_mutex);
    }

    // Lanzar tareas
    xTaskCreate(tarea_botones, "botones", 4096, NULL, 5, NULL);
    xTaskCreate(tarea_led, "led", 2048, NULL, 4, NULL);

    // WiFi + MQTT
    wifi_init();
}
