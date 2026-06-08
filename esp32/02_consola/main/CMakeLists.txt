# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS esp32_consola.c lcd_i2c.c oled_u8g2.c
    INCLUDE_DIRS "."
    REQUIRES nvs_flash driver esp_wifi esp_event esp_netif esp_driver_gpio
)
