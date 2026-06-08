#include "oled_u8g2.h"
#include "u8g2.h"
#include "esp32_hw_i2c.h"
#include "esp_log.h"
#include "driver/gpio.h"   // para gpio_reset_pin()
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *OLED_TAG = "OLED";
static u8g2_t u8g2;
static u8g2_esp32_i2c_ctx_t ctx;
static bool oled_ok = false;  // flag: false si el OLED no está conectado



void oled_init(int sda_pin, int scl_pin) {
    /* CAUSA del "GPIO X is not usable": el firmware anterior dejó GPIO 21/22
     * configurados en el GPIO matrix del ESP32. Al crear un nuevo bus I2C
     * el driver detecta los pines como "en uso" → no los puede controlar
     * → hardware timeout en cada transmisión.
     *
     * SOLUCIÓN: gpio_reset_pin() limpia la entrada del GPIO matrix
     * antes de que u8g2 configure los pines para I2C. */
    gpio_reset_pin((gpio_num_t)sda_pin);
    gpio_reset_pin((gpio_num_t)scl_pin);
    ESP_LOGI(OLED_TAG, "GPIO %d y %d reiniciados para I2C", sda_pin, scl_pin);

    ctx.cfg = (u8g2_esp32_i2c_config_t)U8G2_ESP32_I2C_CONFIG_DEFAULT();
    ctx.cfg.sda_pin       = sda_pin;
    ctx.cfg.scl_pin       = scl_pin;
    ctx.cfg.clk_hz        = 400000;
    ctx.cfg.timeout_ms    = 200;
    ctx.cfg.dev_addr_7bit = 0x3C;
    u8g2_esp32_i2c_set_default_context(&ctx);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        &u8g2,
        U8G2_R0,
        u8x8_byte_esp32_hw_i2c,
        u8x8_gpio_and_delay_esp32_i2c);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    oled_ok = true;
    ESP_LOGI(OLED_TAG, "OLED listo (SDA=%d SCL=%d 0x3C)", sda_pin, scl_pin);
}


static void dibujarTomate(int x, int y, int tamano) {
    // Cuerpo principal (circulo grande)
    u8g2_DrawDisc(&u8g2, x, y, tamano, U8G2_DRAW_ALL);
    
    // Tallo (rectangulo)
    u8g2_DrawBox(&u8g2, x - 1, y - tamano - 8, 2, 8);
    
    // Hojas (dos lineas anguladas)
    u8g2_DrawLine(&u8g2, x - 2, y - tamano - 6, x - 8, y - tamano - 4);
    u8g2_DrawLine(&u8g2, x + 2, y - tamano - 6, x + 8, y - tamano - 4);
    
    // Detalles/sombras en negro (para contraste)
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawCircle(&u8g2, x - 7, y - 2, 3, U8G2_DRAW_ALL);
    u8g2_DrawCircle(&u8g2, x + 6, y + 5, 2, U8G2_DRAW_ALL);
    u8g2_DrawCircle(&u8g2, x, y + tamano - 3, 2, U8G2_DRAW_ALL);
    
    // Brillo (circulo pequeno)
    u8g2_DrawCircle(&u8g2, x - 8, y - 8, 2, U8G2_DRAW_ALL);
    u8g2_SetDrawColor(&u8g2, 1);
}

void oled_draw_tomate(int frame) {
    if (!oled_ok) return;
    u8g2_ClearBuffer(&u8g2);
    
    // Tomate animado que crece y decrece
    int tamano = 12 + ((int)fabs(sin(frame * 0.05) * 4.0));
    dibujarTomate(64, 35, tamano);
    
    // Titulo
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    u8g2_DrawStr(&u8g2, 20, 15, "TOMATE!");
    
    // Contador
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 40, 60, "Frame: ");
    
    char frame_str[16];
    snprintf(frame_str, sizeof(frame_str), "%d", frame);
    u8g2_DrawStr(&u8g2, 85, 60, frame_str);
    
    u8g2_SendBuffer(&u8g2);
}

void oled_render_pantalla(int pantalla_actual, int aceptadas, int podridas, int verdes, const char* estado_motor, bool wifi_ok, int rechazos_consecutivos) {
    if (!oled_ok) return;
    u8g2_ClearBuffer(&u8g2);

    // --- ENCABEZADO ---
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawBox(&u8g2, 0, 0, 128, 14); // Fondo negro
    u8g2_SetDrawColor(&u8g2, 0); // Texto "transparente" (blanco)
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);

    if (wifi_ok) {
        u8g2_DrawStr(&u8g2, 100, 11, "W:ON");
    } else {
        u8g2_DrawStr(&u8g2, 95, 11, "W:OFF");
    }

    switch (pantalla_actual) {
        case 0: u8g2_DrawStr(&u8g2, 4, 11, "CONTADORES FRUTA"); break;
        case 1: u8g2_DrawStr(&u8g2, 4, 11, "DIAGNOSTICO SIST"); break;
        case 2: u8g2_DrawStr(&u8g2, 4, 11, "SISTEMA ALERTAS"); break;
    }
    
    // Restaurar color a normal
    u8g2_SetDrawColor(&u8g2, 1);

    // --- CUERPO ---
    if (pantalla_actual == 0) {
        u8g2_SetFont(&u8g2, u8g2_font_8x13B_tr);
        
        char buf[16];
        snprintf(buf, sizeof(buf), "OK: %d", aceptadas);
        u8g2_DrawStr(&u8g2, 5, 30, buf);
        
        snprintf(buf, sizeof(buf), "Po: %d", podridas);
        u8g2_DrawStr(&u8g2, 5, 45, buf);
        
        snprintf(buf, sizeof(buf), "Ve: %d", verdes);
        u8g2_DrawStr(&u8g2, 5, 60, buf);

        // Caja de motor a la derecha
        u8g2_DrawFrame(&u8g2, 65, 20, 60, 42);
        u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
        u8g2_DrawStr(&u8g2, 75, 32, "MOTOR:");
        
        if (strcmp(estado_motor, "iniciando") == 0 || strcmp(estado_motor, "activo") == 0) {
            u8g2_DrawBox(&u8g2, 69, 42, 52, 15);
            u8g2_SetDrawColor(&u8g2, 0);
            u8g2_DrawStr(&u8g2, 75, 53, " ACTIVO ");
            u8g2_SetDrawColor(&u8g2, 1);
        } else {
            u8g2_DrawStr(&u8g2, 70, 53, "DETENIDO");
        }
    } 
    else if (pantalla_actual == 1) {
        u8g2_SetFont(&u8g2, u8g2_font_8x13_tr);
        char buf[32];
        snprintf(buf, sizeof(buf), "Total: %d", aceptadas + podridas + verdes);
        u8g2_DrawStr(&u8g2, 10, 35, buf);
        
        u8g2_DrawLine(&u8g2, 10, 42, 118, 42);
        
        u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
        u8g2_DrawStr(&u8g2, 10, 56, "Rechazos: ");
        snprintf(buf, sizeof(buf), "%d", rechazos_consecutivos);
        u8g2_DrawStr(&u8g2, 80, 56, buf);
    }
    else if (pantalla_actual == 2) {
        if (rechazos_consecutivos >= 3) {
            u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
            u8g2_DrawStr(&u8g2, 20, 40, "¡ALERTA!");
            u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
            u8g2_DrawStr(&u8g2, 15, 58, "Demasiados rechazos");
            u8g2_DrawFrame(&u8g2, 0, 16, 128, 48);
            u8g2_DrawFrame(&u8g2, 2, 18, 124, 44);
        } else {
            u8g2_SetFont(&u8g2, u8g2_font_8x13_tr);
            u8g2_DrawStr(&u8g2, 15, 45, "Sistema Normal");
        }
    }
    
    u8g2_SendBuffer(&u8g2);
}
