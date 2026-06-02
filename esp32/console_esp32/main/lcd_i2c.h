#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "driver/i2c.h"

#define LCD_I2C_ADDR     0x27       // Dirección típica (cambiar a 0x3F si no funciona)
#define LCD_I2C_PORT     I2C_NUM_0
#define LCD_I2C_FREQ     100000     // 100 kHz

void lcd_init(int sda_gpio, int scl_gpio);
void lcd_clear(void);
void lcd_set_cursor(int col, int row);
void lcd_print(const char *str);

#endif
