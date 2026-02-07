#pragma once

#include "drivers/oled/sh110x.h"
#include <stdio.h>


void graph_draw_axes(sh110x_t *oled);
void graph_draw_fft(sh110x_t *oled, const uint16_t *fft_mag, int bins);
