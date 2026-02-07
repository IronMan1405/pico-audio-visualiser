#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "drivers/oled/sh110x.h"
#include "graphics/graph.h"
#include "fft/fft.h"

#include "i2s_rx.pio.h"

#include <stdio.h>
#include <math.h>

#define WS 5
#define BCLK 4
#define DATA 6

#define SAMPLE_COUNT 256

uint32_t raw_buffer[SAMPLE_COUNT];

int main() {
    stdio_init_all();
        while (!stdio_usb_connected()) {
        sleep_ms(100);
    }


    sleep_ms(2000);

    printf("HELLO FROM PICO\n");

    sleep_ms(1000);

    i2c_init(i2c0, 100000);
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    gpio_pull_up(8);
    gpio_pull_up(9);
    
    PIO pio = pio0;
    int sm = 0;

    uint offset = pio_add_program(pio, &i2s_rx_program);

    i2s_rx_program_init(pio, sm, offset, BCLK, WS, DATA);

    int dma_chan = dma_claim_unused_channel(true);
    dma_channel_config dc = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&dc, DMA_SIZE_32);
    channel_config_set_read_increment(&dc, false);
    channel_config_set_write_increment(&dc, true);
    channel_config_set_dreq(&dc, pio_get_dreq(pio, sm, false));

    dma_channel_configure(
        dma_chan,
        &dc,
        raw_buffer,
        &pio->rxf[sm],
        SAMPLE_COUNT,
        true
    );

    // sh110x_clear(&oled);

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
    
    // uint16_t wave[32] = {
    //     12, 15, 14, 13, 16, 15, 14, 13,
    //     12, 14, 15, 16, 14, 13, 15, 14,
    //     13, 12, 14, 15, 13, 14, 16, 15,
    //     14, 13, 15, 14, 13, 12, 14, 13
    // };

    // double wave[64];
    // double xr[64], xi[64];
    // uint16_t mag[32];

    // int dt = 0;
    // for (;;) {
    //     sh110x_clear(&oled);
    //     // update
    //     for (int i = 0; i < 64; i++) {
    //         float y = cos((i + dt) / 32.0 * 2 * M_PI);
    //         wave[i] = fabs(sin(dt)) * y;
    //     }

    //     // fft
    //     fft2(wave, 64, xr, xi);

    //     for (int i = 0; i < 32; ++i) {
    //         mag[i] = sqrtf(xr[i + 32] * xr[i + 32] + xi[i + 32] * xi[i + 32]);
    //     }

    //     graph_draw_axes(&oled);
    //     graph_draw_fft(&oled, mag, 32);

    //     sh110x_update(&oled);
    //     sleep_ms(100);

    //     ++dt;
    // }

    while (1) {
        // dma_channel_wait_for_finish_blocking(dma_chan);

        // for (int i = 0; i < SAMPLE_COUNT; i += 2) {
        //     // LEFT channel only
        //     int32_t sample = (int32_t)(raw_buffer[i] << 8);
        //     sample >>= 8;  // sign extend 24-bit

        //     printf("%ld\n", sample);
        //     // char buf[8];
        //     // sprintf(buf, "%8ld", sample);
        //     // sh110x_draw_text(&oled, 2, 2, buf, 1);
        // }

        // int32_t min =  1e9;
        // int32_t max = -1e9;

        // for (int i = 0; i < SAMPLE_COUNT; i += 2) {
        //     // int32_t s = (int32_t)(raw_buffer[i] << 8) >> 8;
        //     int32_t s = (int32_t)raw_buffer[i];
        //     s >>= 8;
        //     if (s < min) min = s;
        //     if (s > max) max = s;
        // }

        // printf("min=%ld max=%ld\n", min, max);

        dma_channel_wait_for_finish_blocking(dma_chan);

        int32_t min =  100000000;
        int32_t max = -100000000;

        for (int i = 0; i < SAMPLE_COUNT; i += 2) {
            int32_t s = (int32_t)raw_buffer[i];
            if (s & 0x00800000) s |= 0xFF000000; // sign extend 24-bit

            if (s < min) min = s;
            if (s > max) max = s;
        }

        printf("min=%ld max=%ld\n", min, max);

        dma_channel_set_read_addr(dma_chan, &pio->rxf[sm], true);


        dma_channel_configure(
            dma_chan,
            &dc,
            raw_buffer,
            &pio->rxf[sm],
            SAMPLE_COUNT,
            true
        );

        // sh110x_update(&oled);
    }




    while (1) tight_loop_contents();
}