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
// #include "lcd_i2c.h"
#include "oled_u8g2.h"
/* ───── RED ───── */
#define WIFI_SSID      "CintaTransportadora"
#define WIFI_PASSWORD  "cinta2026"
#define MQTT_BROKER    "mqtt://192.168.4.1:1883"

/* ───── PINES ───── */
#define PIN_SDA        21
#define PIN_SCL        22
#define BTN_INICIAR    GPIO_NUM_18
#define BTN_DETENER    GPIO_NUM_17
#define BTN_PANTALLA   GPIO_NUM_16
#define BTN_RESET      GPIO_NUM_4
#define PIN_LED        GPIO_NUM_2

/* ───── TÓPICOS MQTT ───── */
#define TOPIC_FRUTA    "fruta/estado"
#define TOPIC_COMANDO  "operario/comando"
#define TOPIC_MOTOR    "sistema/motor"
#define TOPIC_VALVULA  "sistema/valvula"

static const char *TAG = "CONSOLA";
static esp_mqtt_client_handle_t cliente_mqtt = NULL;

/* ───── ESTADO COMPARTIDO ───── */
typedef struct {
    int aceptadas;
    int podridas;
    int verdes;
    int rechazos_consecutivos;
    char estado_motor[16];   // "activo" / "detenido" / "esperando"
    int pantalla_actual;     // 0 = contadores, 1 = diagnóstico, 2 = alerta
    bool alerta_activa;
    bool wifi_ok;
    volatile bool lcd_needs_update;
} estado_t;

static estado_t S = {
    .aceptadas = 0,
    .podridas = 0,
    .verdes = 0,
    .rechazos_consecutivos = 0,
    .estado_motor = "esperando",
    .pantalla_actual = 0,
    .alerta_activa = false,
    .wifi_ok = false,
    .lcd_needs_update = true,
};

/* ════════════════════════════════════════════════════════════════
 * RENDERIZAR PANTALLA SEGÚN VISTA ACTUAL
 * ════════════════════════════════════════════════════════════════ */
static void render_pantalla(void)
{
    oled_render_pantalla(S.pantalla_actual, S.aceptadas, S.podridas, S.verdes, S.estado_motor, S.wifi_ok, S.rechazos_consecutivos);
}

/* ════════════════════════════════════════════════════════════════
 * TAREA: BOTONES
 * ════════════════════════════════════════════════════════════════ */
static void tarea_botones(void *param)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << BTN_INICIAR) | (1ULL << BTN_DETENER) |
                        (1ULL << BTN_PANTALLA) | (1ULL << BTN_RESET),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);

    int last1 = 1, last2 = 1, last3 = 1, last4 = 1;

    while (1) {
        int b1 = gpio_get_level(BTN_INICIAR);
        int b2 = gpio_get_level(BTN_DETENER);
        int b3 = gpio_get_level(BTN_PANTALLA);
        int b4 = gpio_get_level(BTN_RESET);

        if (last1 == 1 && b1 == 0) {
            // Solo permite iniciar si no esta "iniciando" ni "activo"
            if (strcmp(S.estado_motor, "iniciando") != 0 && strcmp(S.estado_motor, "activo") != 0) {
                ESP_LOGI(TAG, "Botón INICIAR presionado");
                if (cliente_mqtt) {
                    esp_mqtt_client_publish(cliente_mqtt, TOPIC_COMANDO, "iniciar", 0, 1, 0);
                    strncpy(S.estado_motor, "iniciando", sizeof(S.estado_motor) - 1);
                    S.lcd_needs_update = true;
                } else {
                    ESP_LOGW(TAG, "INICIAR ignorado: MQTT no conectado aún");
                }
            } else {
                ESP_LOGI(TAG, "Ignorado: Ya está iniciado (%s)", S.estado_motor);
            }
        }
        if (last2 == 1 && b2 == 0) {
            // Solo permite detener si esta "iniciando" o "activo"
            if (strcmp(S.estado_motor, "iniciando") == 0 || strcmp(S.estado_motor, "activo") == 0) {
                ESP_LOGI(TAG, "Botón DETENER presionado");
                if (cliente_mqtt) {
                    esp_mqtt_client_publish(cliente_mqtt, TOPIC_COMANDO, "detener", 0, 1, 0);
                    strncpy(S.estado_motor, "deteniendo", sizeof(S.estado_motor) - 1);
                    S.lcd_needs_update = true;
                } else {
                    ESP_LOGW(TAG, "DETENER ignorado: MQTT no conectado aún");
                }
            } else {
                ESP_LOGI(TAG, "Ignorado: Ya está detenido (%s)", S.estado_motor);
            }
        }
        if (last3 == 1 && b3 == 0) {
            S.pantalla_actual = (S.pantalla_actual + 1) % 3;
            ESP_LOGI(TAG, "Pantalla cambiada a %d", S.pantalla_actual);
            // Publicar pantalla al dashboard
            if (cliente_mqtt) {
                char buf[4];
                snprintf(buf, sizeof(buf), "%d", S.pantalla_actual);
                esp_mqtt_client_publish(cliente_mqtt, "consola/pantalla", buf, 0, 0, 0);
            }
            S.lcd_needs_update = true;
        }
        if (last4 == 1 && b4 == 0) {
            ESP_LOGI(TAG, "Contadores reseteados");
            S.aceptadas = 0;
            S.podridas = 0;
            S.verdes = 0;
            S.rechazos_consecutivos = 0;
            S.alerta_activa = false;
            // Notificar reset al dashboard
            if (cliente_mqtt) {
                esp_mqtt_client_publish(cliente_mqtt, "consola/reset", "reset", 0, 0, 0);
            }
            S.lcd_needs_update = true;
        }

        last1 = b1; last2 = b2; last3 = b3; last4 = b4;
        vTaskDelay(pdMS_TO_TICKS(30));  // Antirrebote 30ms (más estable)
    }
}

/* ════════════════════════════════════════════════════════════════
 * TAREA: LED DE ESTADO
 * ════════════════════════════════════════════════════════════════ */
static void tarea_led(void *param)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << PIN_LED,
        .mode         = GPIO_MODE_OUTPUT,
    };
    gpio_config(&cfg);

    while (1) {
        if (!S.wifi_ok) {
            // Parpadeo rápido = sin WiFi
            gpio_set_level(PIN_LED, 1); vTaskDelay(pdMS_TO_TICKS(150));
            gpio_set_level(PIN_LED, 0); vTaskDelay(pdMS_TO_TICKS(150));
        } else if (S.alerta_activa) {
            // Parpadeo lento = alerta
            gpio_set_level(PIN_LED, 1); vTaskDelay(pdMS_TO_TICKS(500));
            gpio_set_level(PIN_LED, 0); vTaskDelay(pdMS_TO_TICKS(500));
        } else {
            gpio_set_level(PIN_LED, S.wifi_ok ? 1 : 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

/* ════════════════════════════════════════════════════════════════
 * TAREA: ACTUALIZAR LCD (separada para no bloquear botones)
 * ════════════════════════════════════════════════════════════════ */
static void tarea_lcd(void *param)
{
    while (1) {
        if (S.lcd_needs_update) {
            S.lcd_needs_update = false;
            render_pantalla();
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Refresco instantáneo
    }
}

/* ════════════════════════════════════════════════════════════════
 * HANDLER MQTT
 * ════════════════════════════════════════════════════════════════ */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event  = event_data;
    esp_mqtt_client_handle_t client = event->client;
    char topic[64];
    char data[64];

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT conectado");
        esp_mqtt_client_subscribe(client, TOPIC_FRUTA, 1);
        esp_mqtt_client_subscribe(client, TOPIC_MOTOR, 1);
        esp_mqtt_client_subscribe(client, TOPIC_VALVULA, 1);
        esp_mqtt_client_subscribe(client, TOPIC_COMANDO, 1);
        S.wifi_ok = true;
        S.lcd_needs_update = true;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT desconectado");
        S.wifi_ok = false;
        S.lcd_needs_update = true;
        break;

    case MQTT_EVENT_DATA:
        snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);
        snprintf(data,  sizeof(data),  "%.*s", event->data_len,  event->data);
        ESP_LOGI(TAG, "[%s] = %s", topic, data);

        /* ── ESTADO DEL MOTOR (enviado por la cinta) ── */
        if (strcmp(topic, TOPIC_MOTOR) == 0) {
            strncpy(S.estado_motor, data, sizeof(S.estado_motor) - 1);
            S.estado_motor[sizeof(S.estado_motor) - 1] = '\0';
            S.lcd_needs_update = true;
        }

        /* ── COMANDO OPERARIO (desde RPi/PC/botones remotos) ── */
        else if (strcmp(topic, TOPIC_COMANDO) == 0) {
            if (strcmp(data, "iniciar") == 0) {
                strncpy(S.estado_motor, "iniciando", sizeof(S.estado_motor) - 1);
            } else if (strcmp(data, "detener") == 0) {
                strncpy(S.estado_motor, "deteniendo", sizeof(S.estado_motor) - 1);
            }
            S.estado_motor[sizeof(S.estado_motor) - 1] = '\0';
            S.lcd_needs_update = true;
        }

        /* ── VEREDICTO DE FRUTA ── */
        else if (strcmp(topic, TOPIC_FRUTA) == 0) {
            if (strcmp(data, "aceptada") == 0) {
                S.aceptadas++;
                S.rechazos_consecutivos = 0;
                S.alerta_activa = false;
            } else if (strcmp(data, "podrida") == 0) {
                S.podridas++;
                S.rechazos_consecutivos++;
                if (S.rechazos_consecutivos >= 3) {
                    S.alerta_activa = true;
                    S.pantalla_actual = 2;
                }
            } else if (strcmp(data, "verde") == 0) {
                S.verdes++;
                S.rechazos_consecutivos++;
                if (S.rechazos_consecutivos >= 3) {
                    S.alerta_activa = true;
                    S.pantalla_actual = 2;
                }
            }
            S.lcd_needs_update = true;
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "Error en cliente MQTT");
        break;

    default:
        break;
    }
}

/* ════════════════════════════════════════════════════════════════
 * HANDLER WIFI
 * ════════════════════════════════════════════════════════════════ */
static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        S.wifi_ok = false;
        S.lcd_needs_update = true;
        esp_wifi_connect();
    }
    else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "WiFi conectado. IP: " IPSTR, IP2STR(&event->ip_info.ip));

        // Iniciar cliente MQTT
        esp_mqtt_client_config_t mqtt_cfg = {
            .broker.address.uri = MQTT_BROKER,
        };
        esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
        esp_mqtt_client_start(client);
        cliente_mqtt = client;
    }
}

/* ════════════════════════════════════════════════════════════════
 * INICIALIZACIÓN WIFI
 * ════════════════════════════════════════════════════════════════ */
static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    ESP_ERROR_CHECK(esp_wifi_start());

    // Reducir potencia TX para evitar caídas de voltaje en la PCB
    esp_wifi_set_max_tx_power(40); // 10 dBm (~150mA pico vs 450mA default)
}

/* ════════════════════════════════════════════════════════════════
 * APP MAIN
 * ════════════════════════════════════════════════════════════════ */
void app_main(void)
{
    ESP_LOGI(TAG, "════════════════════════════════════════");
    ESP_LOGI(TAG, "  ESP32 CONSOLA — Clasificador de Frutas");
    ESP_LOGI(TAG, "════════════════════════════════════════");

    /* NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    /* LCD / OLED */
    // lcd_init(PIN_SDA, PIN_SCL);
    // lcd_clear();
    // lcd_set_cursor(0, 0); lcd_print("Iniciando...");
    // lcd_set_cursor(0, 1); lcd_print("Conectando WiFi");
    oled_init(PIN_SDA, PIN_SCL);

    /* Lanzar tareas */
    xTaskCreate(tarea_botones, "botones", 4096, NULL, 5, NULL);
    xTaskCreate(tarea_led,     "led",     2048, NULL, 4, NULL);
    xTaskCreate(tarea_lcd,     "lcd",     4096, NULL, 4, NULL);

    /* WiFi + MQTT */
    wifi_init();

    ESP_LOGI(TAG, "Sistema listo. Esperando eventos...");
}
