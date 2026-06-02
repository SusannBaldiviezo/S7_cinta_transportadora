#pragma once

/* ================================================================
 * valvula.h — API de control de la válvula solenoide (relé)
 * ================================================================ */

/**
 * @brief Configura el pin del relé como salida digital y lo
 *        inicializa en estado LOW (válvula cerrada).
 *        Crea la tarea FreeRTOS dedicada que espera notificaciones.
 *        Debe llamarse una sola vez al inicio del programa.
 */
void valvula_init(void);

/**
 * @brief Notifica a la tarea de la válvula para que dispare el
 *        cilindro neumático. No bloquea el hilo que lo llama.
 *        Seguro de llamar desde callbacks de MQTT o interrupciones.
 */
void valvula_disparar(void);
