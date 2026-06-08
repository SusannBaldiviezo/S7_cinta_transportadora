# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS
        "esp32_cinta.c"
        "motor.c"
        "servo.c"
        "mqtt_handler.c"
        "wifi_manager.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_event esp_netif mqtt driver esp_driver_gpio esp_driver_ledc)
