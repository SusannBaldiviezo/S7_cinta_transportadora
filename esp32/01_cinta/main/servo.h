#pragma once

/* ================================================================
 * servo.h — API de control de 2 servomotores sincronizados (GPIO 19 y 17)
 *
 * ESTADOS:
 *   maduro   → 45°  derecha
 *   pintón/verde → 90° centro (reposo)
 *   podrido  → 135° izquierda
 * ================================================================ */

/**
 * @brief Configura LEDC para los dos servos y crea la tarea FreeRTOS
 *        dedicada que procesa las señales de fruta.
 *        Debe llamarse una sola vez al inicio del programa.
 *        Los servos arrancan en 90° (centro / pintón).
 */
void servo_init(void);

/**
 * @brief Notifica a la tarea servo que la fruta detectada es MADURA.
 *        → Los servos van a 45° (derecha) de inmediato.
 *        No bloquea el hilo que lo llama.
 */
void servo_disparar_maduro(void);

/**
 * @brief Notifica a la tarea servo que la fruta detectada es PINTONA o VERDE.
 *        → Los servos van a 90° (centro) de inmediato.
 *        Si ya están en el centro, NO se produce movimiento innecesario.
 *        No bloquea el hilo que lo llama.
 */
void servo_disparar_pinton(void);

/**
 * @brief Notifica a la tarea servo que la fruta detectada está PODRIDA.
 *        → Los servos van a 135° (izquierda) de inmediato.
 *        No bloquea el hilo que lo llama.
 */
void servo_disparar_podrido(void);
