#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "drivers/oled/sh110x.h"

int main() {
    stdio_init_all();

    i2c_init(i2c0, 100000);
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    gpio_pull_up(8);
    gpio_pull_up(9);

    sh110x_t oled;
    sh110x_init(&oled, i2c0, 0x3C);

    sh110x_clear(&oled);

    for (int i = 0; i < 64; i++) {
        sh110x_draw_pixel(&oled, i, i, true);
    }

    sh110x_update(&oled);

    // printf("Scanning I2C bus...\n");
    
    // for (uint8_t addr = 1; addr < 127; addr++) {
    //     if (i2c_write_blocking(i2c0, addr, NULL, 0, false) >= 0) {
    //         printf("Found device at 0x%02X\n", addr);
    //     }
    // }

    while (1) tight_loop_contents();
}