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

// audio stuff
#define SAMPLE_COUNT 1024
#define GAIN 16
#define ALPHA 0.1

uint32_t raw_buffer[2 * SAMPLE_COUNT];
float hanning_table[SAMPLE_COUNT];

int main() {
    stdio_init_all();
    sleep_ms(100);

    // init i2c
    i2c_init(i2c0, 100000);
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    gpio_pull_up(8);
    gpio_pull_up(9);

    // init mic
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

    // init oled
    sh110x_t oled;
    sh110x_init(&oled, i2c0, 0x3C);


    // precompute hanning
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        hanning_table[i] = 0.5 * (1 - cos(2 * M_PI * i / (SAMPLE_COUNT - 1)));
    }

    // main loop
    float wave[SAMPLE_COUNT];

    float filt_prev = 0;
    float reals[SAMPLE_COUNT], imags[SAMPLE_COUNT];

    uint16_t bars[SAMPLE_COUNT / 2];
    while (1) {
        // read from sensor
        dma_channel_configure(
            dma_chan,
            &dc,
            raw_buffer,
            &pio->rxf[sm],
            2 * SAMPLE_COUNT,
            true
        );
        dma_channel_wait_for_finish_blocking(dma_chan);

        int32_t min = 100000000;
        int32_t max = -100000000;

        float sum = 0; // to remove DC
        for (int i = 0; i < 2 * SAMPLE_COUNT; i += 2) {
            int32_t s = (int32_t) raw_buffer[i];
            if (s & 0x00800000) s |= 0xFF000000; // sign extend 24-bit

            float x = (float) s / 8388608.0f;
            wave[i / 2] = x;
            sum += x;
        }

        float av = sum / SAMPLE_COUNT;
        // remove dc
        for (int i = 0; i < SAMPLE_COUNT; ++i) {
            wave[i] -= av;
            wave[i] *= GAIN;

            // apply low pass filtering (digital signal processing :fire:)
            // filt_prev = ALPHA * wave[i] + (1 - ALPHA) * filt_prev;
            // wave[i] = filt_prev;

            // hanning window
            wave[i] *= hanning_table[i];
        }

        fft2(wave, SAMPLE_COUNT, reals, imags);

        // bar computation
        int net_bins = SAMPLE_COUNT / 2; //512
        int start_bin = 2;
        int end_bin = net_bins * 0.8; // 409.6
        int usable_bins = end_bin - start_bin; // 407.6

        int num_bars = 124;
        
        // int bins_per_bar = net_bins / num_bars;
        int bins_per_bar = usable_bins / num_bars; // 12.73

        for (int i = 0; i < num_bars; ++i) {
            float max_power = 0;

            for (int b = 0; b < bins_per_bar; ++b) {
                // int ffti = i * bins_per_bar + b;
                int ffti = start_bin + i * bins_per_bar + b;

                float power = reals[ffti] * reals[ffti] + imags[ffti] * imags[ffti];
                if (power > max_power) max_power = power;
            }

            float db = 10.0f * log10f(max_power + 1e-6f);
            int height = (int) (db + 0.f) * 2;
            if (height < 0) height = 0;
            if (height > 63) height = 63;
            bars[i] = height;
        }

        // display oled
        sh110x_clear(&oled);

        graph_draw_axes(&oled);
        graph_draw_fft(&oled, bars, num_bars);

        sh110x_update(&oled);
    }
}
