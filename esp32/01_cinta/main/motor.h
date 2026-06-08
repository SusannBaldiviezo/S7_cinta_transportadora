#pragma once

/* ================================================================
 * motor.h — API de control del motor DC via BTS7960
 * ================================================================ */

/**
 * @brief Inicializa los pines de habilitación (R_EN, L_EN) y
 *        configura los dos canales PWM (RPWM, LPWM) del BTS7960.
 *        Debe llamarse una sola vez al inicio del programa.
 */
void motor_init(void);

/**
 * @brief Activa el motor hacia adelante a la velocidad indicada.
 * @param velocidad  Valor de duty cycle entre 0 (parado) y 255 (máximo).
 */
void motor_avanzar(int velocidad);

/**
 * @brief Detiene el motor poniendo ambos canales PWM a cero.
 */
void motor_detener(void);
