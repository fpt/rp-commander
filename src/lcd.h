// SPDX-License-Identifier: MIT
// ST7789 240×240 LCD driver — SPI0, RP2040-Zero pins

#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>
#include <stdbool.h>

#define LCD_W  240
#define LCD_H  240

// RGB565 colour helpers (values stored byte-swapped for DMA/SPI big-endian output)
#define RGB565(r,g,b)  __builtin_bswap16((((r)&0xF8u)<<8)|(((g)&0xFCu)<<3)|((b)>>3))

#define COL_BLACK    0x0000u
#define COL_WHITE    0xFFFFu
#define COL_RED      RGB565(220,  40,  40)
#define COL_GREEN    RGB565( 40, 200,  40)
#define COL_BLUE     RGB565( 40,  80, 220)
#define COL_CYAN     RGB565(  0, 200, 220)
#define COL_YELLOW   RGB565(230, 200,   0)
#define COL_ORANGE   RGB565(230, 100,   0)
#define COL_PURPLE   RGB565(150,  50, 220)
#define COL_GRAY     RGB565( 80,  80,  80)
#define COL_DGRAY    RGB565( 30,  30,  30)
#define COL_LGRAY    RGB565(160, 160, 160)

void lcd_init(void);
void lcd_set_backlight(uint8_t pct);   // 0-100

// Push a rectangular region of framebuf (240-wide uint16_t array) to the display
void lcd_flush_region(const uint16_t *fb, int x, int y, int w, int h);

// Push the entire framebuf at once (blocking)
void lcd_flush_full(const uint16_t *fb);

#endif // _LCD_H_
