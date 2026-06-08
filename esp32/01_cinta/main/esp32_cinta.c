/* ================================================================
 * esp32_cinta.c — Punto de entrada principal
 *
 * Solo contiene app_main(). Toda la lógica está modularizada:
 *   config.h        → pines, red, tópicos MQTT
 *   motor.h/.c      → control BTS7960 (PWM)
 *   valvula.h/.c    → relé + tarea FreeRTOS no bloqueante
 *   mqtt_handler.h/.c → eventos MQTT
 *   wifi_manager.h/.c → conexión WiFi
 * ================================================================ */

#include "esp_log.h"
#include "nvs_flash.h"
#include "motor.h"
#include "servo.h"
#include "wifi_manager.h"
#define WIFI_SSID      "CintaTransportadora"
#define WIFI_PASSWORD  "cinta2026"
#define MQTT_BROKER    "mqtt://192.168.4.1:1883"

static const char *TAG = "CINTA";

void app_main(void)
{
    ESP_LOGI(TAG, "════════════════════════════════════════");
    ESP_LOGI(TAG, "  ESP32 CINTA — Clasificador de Frutas  ");
    ESP_LOGI(TAG, "  Driver: BTS7960                       ");
    ESP_LOGI(TAG, "════════════════════════════════════════");

    /* Inicializar NVS (requerido por WiFi) */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    /* Hardware */
    motor_init();
    servo_init();   // también crea la tarea FreeRTOS de los servos/relés
    motor_detener();  // asegurar estado inicial seguro

    /* Red + MQTT (el cliente MQTT arranca automáticamente al obtener IP) */
    wifi_init();

    ESP_LOGI(TAG, "Sistema listo. Esperando comandos MQTT...");
}
