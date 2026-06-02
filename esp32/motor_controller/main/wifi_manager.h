#pragma once

/* ================================================================
 * wifi_manager.h — API de inicialización WiFi
 * ================================================================ */

/**
 * @brief Inicializa el stack TCP/IP, el event loop, la interfaz STA
 *        y registra los handlers de eventos WiFi e IP.
 *        Al obtener IP, arranca automáticamente el cliente MQTT.
 *        Debe llamarse una sola vez al inicio del programa.
 */
void wifi_init(void);
