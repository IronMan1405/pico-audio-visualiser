#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "drivers/oled/sh110x.h"
#include "graphics/graph.h"
#include <stdio.h>

int main() {
    stdio_init_all();

    sleep_ms(2000);

    printf("HELLO FROM PICO\n");

    sleep_ms(1000);

    i2c_init(i2c0, 100000);
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    gpio_pull_up(8);
    gpio_pull_up(9);

    sh110x_t oled;
    sh110x_init(&oled, i2c0, 0x3C);

    sh110x_clear(&oled);

    // for (int i = 0; i < 64; i++) {
    //     sh110x_draw_pixel(&oled, 128-i, 64-i, true);
    // }

    // sh110x_draw_line(&oled, 0, 0, 127, 0);
    // sh110x_draw_line(&oled, 0, 0, 0, 63);
    // sh110x_draw_line(&oled, 0, 0, 127, 63);
    // sh110x_draw_line(&oled, 64, 63, 64, 0);
    // sh110x_draw_line(&oled, 10, 50, 100, 20);

    // sh110x_draw_text(&oled, 1, 1, "FFT ONLINE", 1);
    // sh110x_draw_text(&oled, 5, 15, "DRIVERS > SLEEP", 1);

    // uint16_t fft_mag[32] = {
    //     12, 15, 14, 13, 16, 15, 14, 13,
    //     12, 14, 15, 16, 14, 13, 15, 14,
    //     13, 12, 14, 15, 13, 14, 16, 15,
    //     14, 13, 15, 14, 13, 12, 14, 13
    // };

    uint16_t fft_mag[32] = {
        3, 4, 6, 10, 16, 28, 62, 254,
        62, 28, 16, 10, 6, 4, 3, 2,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1
    };

    for (int i = 0; i < 32; i++) {
        int h = (fft_mag[i] * 60) / 32;
        printf("%d\n", h);
    }

    graph_draw_axes(&oled);
    graph_draw_fft(&oled, fft_mag, 31);

    sh110x_update(&oled);

    while (1) tight_loop_contents();
}