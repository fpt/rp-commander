// SPDX-License-Identifier: MIT
// Simple 8x8 bitmap font

#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>

#define FONT_W 8
#define FONT_H 8

// Returns pointer to 8 bytes (one per row, MSB = leftmost pixel) for ASCII char
const uint8_t *font_get_char(char c);

// Render one character into fb at (x,y) with given scale and colours.
void font_draw_char(uint16_t *fb, int x, int y, char c, int scale,
                    uint16_t fg, uint16_t bg);

#endif // _FONT_H_
