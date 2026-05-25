// SPDX-License-Identifier: MIT
// Simple 8x8 bitmap font

#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>

#define FONT_W 8
#define FONT_H 8

// Returns pointer to 8 bytes (one per row, MSB = leftmost pixel) for ASCII char
const uint8_t *font_get_char(char c);

#endif // _FONT_H_
