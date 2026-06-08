#ifndef OLED_U8G2_H
#define OLED_U8G2_H

#include <stdint.h>
#include <stdbool.h>

void oled_init(int sda_pin, int scl_pin);
void oled_draw_tomate(int frame);
void oled_render_pantalla(int pantalla_actual, int aceptadas, int podridas, int verdes, const char* estado_motor, bool wifi_ok, int rechazos_consecutivos);


#endif // OLED_U8G2_H
