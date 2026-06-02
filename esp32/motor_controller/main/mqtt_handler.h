#pragma once

/* ================================================================
 * mqtt_handler.h — API del manejador de eventos MQTT
 * ================================================================ */

#include "mqtt_client.h"

/**
 * @brief Manejador de eventos MQTT registrado con
 *        esp_mqtt_client_register_event().
 *        Procesa CONNECTED, DISCONNECTED, DATA y ERROR.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                        int32_t event_id, void *event_data);

/**
 * @brief Handle global del cliente MQTT.
 *        Compartido con valvula.c para publicar confirmaciones.
 */
extern esp_mqtt_client_handle_t cliente_mqtt;

/**
 * @brief Bandera de estado del sistema (true = sistema activo).
 */
extern bool sistema_activo;
