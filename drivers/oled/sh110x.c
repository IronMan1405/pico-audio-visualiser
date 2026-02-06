#include "sh110x.h"
#include "sh110x_font.h"
#include "pico/stdlib.h"
#include <string.h>

#define SH110X_CMD 0x00
#define SH110X_DATA 0x40

static void sh110x_write_cmd(sh110x_t *oled, uint8_t cmd) {
    uint8_t buf[2] = {SH110X_CMD, cmd};
    i2c_write_blocking(oled->i2c, oled->address, buf, 2, false);
}

bool sh110x_init(sh110x_t *oled, i2c_inst_t *i2c, uint8_t addr) {
    oled->i2c = i2c;
    oled->address = addr;
    memset(oled->buffer, 0, sizeof(oled->buffer));

    sleep_ms(100);

    sh110x_write_cmd(oled, 0xAE); // OFF
    sh110x_write_cmd(oled, 0x20); // memory mode
    sh110x_write_cmd(oled, 0x00); // horizontal addressing
    sh110x_write_cmd(oled, 0xB0); // page start
    sh110x_write_cmd(oled, 0xC8); // com scan direction
    sh110x_write_cmd(oled, 0x00); // low column
    sh110x_write_cmd(oled, 0x10); // high column
    sh110x_write_cmd(oled, 0x40); // start line
    sh110x_write_cmd(oled, 0x81); // contrast
    sh110x_write_cmd(oled, 0x7F); // 
    sh110x_write_cmd(oled, 0xA1); // segment remap
    sh110x_write_cmd(oled, 0xA6); // normal display
    sh110x_write_cmd(oled, 0xA8); // multiplex
    sh110x_write_cmd(oled, 0x3F); // 
    sh110x_write_cmd(oled, 0xA4); // display follows RAM
    sh110x_write_cmd(oled, 0xD3); // display offset
    sh110x_write_cmd(oled, 0x00); // 
    sh110x_write_cmd(oled, 0xD5); // clock divide
    sh110x_write_cmd(oled, 0x80); // 
    sh110x_write_cmd(oled, 0xD9); // pre charge
    sh110x_write_cmd(oled, 0xF1); // 
    sh110x_write_cmd(oled, 0xDA); // COM pins
    sh110x_write_cmd(oled, 0x12); // 
    sh110x_write_cmd(oled, 0xDB); // VCOM detect
    sh110x_write_cmd(oled, 0x40); // 
    sh110x_write_cmd(oled, 0x8D); // charge pump
    sh110x_write_cmd(oled, 0x14); // 
    sh110x_write_cmd(oled, 0xAF); // ON

    // sh110x_write_cmd(oled, 0xAE); // Display OFF
    // sh110x_write_cmd(oled, 0xD5); sh110x_write_cmd(oled, 0x80); // Clock divide
    // sh110x_write_cmd(oled, 0xA8); sh110x_write_cmd(oled, 0x3F); // Multiplex
    // sh110x_write_cmd(oled, 0xD3); sh110x_write_cmd(oled, 0x00); // Display offset
    // sh110x_write_cmd(oled, 0x40); // Start line = 0
    // sh110x_write_cmd(oled, 0xAD); sh110x_write_cmd(oled, 0x8B); // DC-DC ON
    // sh110x_write_cmd(oled, 0xA1); // Segment remap
    // sh110x_write_cmd(oled, 0xC8); // COM scan dec
    // sh110x_write_cmd(oled, 0xDA); sh110x_write_cmd(oled, 0x12); // COM pins
    // sh110x_write_cmd(oled, 0x81); sh110x_write_cmd(oled, 0x7F); // Contrast
    // sh110x_write_cmd(oled, 0xA4); // Resume RAM
    // sh110x_write_cmd(oled, 0xA6); // Normal display
    // sh110x_write_cmd(oled, 0xAF); // Display ON

    return true;
}

void sh110x_clear(sh110x_t *oled) {
    memset(oled->buffer, 0, sizeof(oled->buffer));
}

void sh110x_update(sh110x_t *oled) {
    for (uint8_t page = 0; page < SH110X_PAGES; page++) {
        sh110x_write_cmd(oled, 0xB0 + page);
        sh110x_write_cmd(oled, 0x02);
        sh110x_write_cmd(oled, 0x10);
        
        uint8_t packet[SH110X_WIDTH + 1];
        packet[0] = SH110X_DATA;

        memcpy(&packet[1], &oled->buffer[page * SH110X_WIDTH], SH110X_WIDTH);

        i2c_write_blocking(oled->i2c, oled->address, packet, SH110X_WIDTH+1, false);
    }
}

void sh110x_draw_pixel(sh110x_t *oled, int x, int y, bool on) {
    if (x < 0 || x >= SH110X_WIDTH || y < 0 || y >= SH110X_HEIGHT) return;

    uint16_t index = x + (y / 8) * SH110X_WIDTH;
    uint8_t mask = 1 << (y % 8);

    if (on) {
        oled->buffer[index] |= mask;
    } else {
        oled->buffer[index] &= ~mask;
    }
}

void sh110x_draw_line(sh110x_t *oled, int x0, int y0, int x1, int y1) {
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    while (1) {
        sh110x_draw_pixel(oled, x0, y0, true);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void sh110x_draw_char(sh110x_t *oled, int x, int y, char c, uint8_t scale) {
    if (c < 32 || c > 127) {
        return;
    }
    
    const uint8_t *glyph = font5x7[c-32];

    for (int col = 0; col < 5; col++) {
        int bits = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (bits & ( 1<< row)){
                for (int dx = 0; dx < scale; dx++) {
                    for (int dy = 0; dy < scale; dy++) {
                        sh110x_draw_pixel(oled, x + col, y + row, true);
                    }
                }
            }
        }
    }
}

void sh110x_draw_text(sh110x_t *oled, int x, int y, const char *text, uint8_t scale) {
    int cursor_x = x;
    while (*text) {
        sh110x_draw_char(oled, cursor_x, y, *text, scale);
        cursor_x += 6 * scale; // 5 pixels width and 1 pixel space
        text++;
    }
}
