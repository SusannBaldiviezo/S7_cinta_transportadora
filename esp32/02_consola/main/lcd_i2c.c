#include "lcd_i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LCD_BACKLIGHT    0x08
#define LCD_ENABLE       0x04
#define LCD_CMD          0x00
#define LCD_CHAR         0x01

#include "rom/ets_sys.h"

static void lcd_send_nibble(uint8_t nibble, uint8_t mode)
{
    uint8_t data = nibble | mode | LCD_BACKLIGHT;
    uint8_t buffer[4];
    buffer[0] = data;               // EN=0
    buffer[1] = data | LCD_ENABLE;  // EN=1
    buffer[2] = data | LCD_ENABLE;  // EN=1 (extiende el pulso)
    buffer[3] = data;               // EN=0

    i2c_master_write_to_device(LCD_I2C_PORT, LCD_I2C_ADDR,
                               buffer, 4, pdMS_TO_TICKS(100));
}

static void lcd_send_byte(uint8_t byte, uint8_t mode)
{
    lcd_send_nibble(byte & 0xF0, mode);
    ets_delay_us(100);
    lcd_send_nibble((byte << 4) & 0xF0, mode);
    ets_delay_us(100);
}

void i2c_scan(void)
{
    ESP_LOGI("I2C_SCAN", "Escaneando bus I2C...");
    for (int addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t res = i2c_master_cmd_begin(LCD_I2C_PORT, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);
        if (res == ESP_OK) {
            ESP_LOGI("I2C_SCAN", ">>> Dispositivo I2C encontrado en: 0x%02X <<<", addr);
        }
    }
    ESP_LOGI("I2C_SCAN", "Escaneo finalizado.");
}

void lcd_init(int sda_gpio, int scl_gpio)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = LCD_I2C_FREQ,
    };
    i2c_param_config(LCD_I2C_PORT, &conf);
    i2c_driver_install(LCD_I2C_PORT, conf.mode, 0, 0, 0);

    // Escanear para encontrar la dirección real
    i2c_scan();

    vTaskDelay(pdMS_TO_TICKS(50));

    // Secuencia de inicialización HD44780 en modo 4-bit
    lcd_send_nibble(0x30, LCD_CMD); vTaskDelay(pdMS_TO_TICKS(5));
    lcd_send_nibble(0x30, LCD_CMD); vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_nibble(0x30, LCD_CMD); vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_nibble(0x20, LCD_CMD); vTaskDelay(pdMS_TO_TICKS(1));

    lcd_send_byte(0x28, LCD_CMD);  // 4-bit, 2 líneas, fuente 5x8
    lcd_send_byte(0x0C, LCD_CMD);  // Display ON, cursor OFF
    lcd_send_byte(0x06, LCD_CMD);  // Entrada: incrementar cursor
    lcd_send_byte(0x01, LCD_CMD);  // Clear
    vTaskDelay(pdMS_TO_TICKS(5));
}

void lcd_clear(void)
{
    lcd_send_byte(0x01, LCD_CMD);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcd_set_cursor(int col, int row)
{
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    addr += col;
    lcd_send_byte(addr, LCD_CMD);
}

void lcd_print(const char *str)
{
    while (*str) {
        lcd_send_byte(*str++, LCD_CHAR);
    }
}
