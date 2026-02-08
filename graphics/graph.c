#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "graph.h"
#include "drivers/oled/sh110x.h"

#define GRAPH_X0 2
#define GRAPH_Y0 2
#define GRAPH_X1 126
#define GRAPH_Y1 62

#define GRAPH_WIDTH  (GRAPH_X1 - GRAPH_X0)
#define GRAPH_HEIGHT (GRAPH_Y1 - GRAPH_Y0)


void graph_draw_axes(sh110x_t *oled) {
    sh110x_draw_line(oled, GRAPH_X0, GRAPH_Y0, GRAPH_X0, GRAPH_Y1);
    sh110x_draw_line(oled, GRAPH_X0, GRAPH_Y1, GRAPH_X1, GRAPH_Y1);
}

void graph_draw_fft(sh110x_t *oled, const uint16_t *fft_mag, int bins) {
    int bar_w = GRAPH_WIDTH / bins;

    if (bar_w < 1) bar_w = 1;

    for (int i = 0; i < bins; i++) {
        int h = fft_mag[i];

        if (h > GRAPH_HEIGHT) h = GRAPH_HEIGHT;
        if (h < 0) h = 0;

        int x = GRAPH_X0 + i * bar_w;
        int y = GRAPH_Y1 - h;

        int w = bar_w > 1 ? bar_w - 1 : 1;

        sh110x_fill_rect(oled, x, y, w, h);
    }
}

void graph_draw_wave(sh110x_t *oled, float *samples, int len) {
    int _x = GRAPH_X0;
    int _y = GRAPH_Y1 / 2;

    int step = len / GRAPH_WIDTH;
    if (step < 1) step = 1;

    for (int i = 0; i < GRAPH_WIDTH; ++i) {
        int sample_index = i * step;
        if (sample_index >= len) break;

        float val = samples[sample_index];
        int y = 32 - (int) (val * 32);
        if (y < 0) y = 0; if (y > 63) y = 63;
        int x = GRAPH_X0 + i;

        sh110x_draw_line(oled, _x, _y, x, y);
        _x = x;
        _y = y;
    }
}
