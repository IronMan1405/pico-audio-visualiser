// #pragma once

#ifndef SH110X_H
#define SH110X_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

#define SH110X_WIDTH 128
#define SH110X_HEIGHT 64
#define SH110X_PAGES (SH110X_HEIGHT / 8)

typedef struct {
    i2c_inst_t *i2c;
    uint8_t address;
    uint8_t buffer[SH110X_WIDTH * SH110X_PAGES];
} sh110x_t;

bool sh110x_init(sh110x_t *oled, i2c_inst_t *i2c, uint8_t addr);
void sh110x_clear(sh110x_t *oled);
void sh110x_update(sh110x_t *oled);

void sh110x_draw_pixel(sh110x_t *oled, int x, int y, bool on);
void sh110x_draw_line(sh110x_t *oled, int x0, int y0, int x1, int y1);
void sh110x_draw_char(sh110x_t *oled, int x, int y, char c, uint8_t scale);
void sh110x_draw_text(sh110x_t *oled, int x, int y, const char *text, uint8_t scale);

#endif