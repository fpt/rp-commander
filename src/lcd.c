// SPDX-License-Identifier: MIT
// ST7789 240×240 driver for ardu-commander (SPI0, RP2040-Zero)
// Adapted from rp-carsensor / rp-picones lcd drivers.

#include "lcd.h"
#include "config.h"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

// ── ST7789 register map ───────────────────────────────────────────────────────

#define ST_MADCTL  0x36
#define ST_COLMOD  0x3A
#define ST_CASET   0x2A
#define ST_RASET   0x2B
#define ST_RAMWR   0x2C
#define ST_INVON   0x21
#define ST_SLPOUT  0x11
#define ST_NORON   0x13
#define ST_DISPON  0x29

// 180° rotation: MY | MX = 0xC0
// Row offset for 240×240 panel at 180°: the panel covers GRAM rows 0-239 at 0°;
// at 180° (MY|MX) the effective start address shifts by 80.
#define MADCTL_VAL  0xC0
#define ROW_OFFSET  80
#define COL_OFFSET  0

// ── Backlight PWM ─────────────────────────────────────────────────────────────

#define PWM_FREQ  10000
#define PWM_WRAP  1000

static uint pwm_slice, pwm_chan;

// ── DMA ───────────────────────────────────────────────────────────────────────

static int dma_ch = -1;
static volatile bool dma_busy = false;

static void dma_irq_handler(void) {
    dma_hw->ints0 = 1u << dma_ch;
    sleep_us(10);
    gpio_put(LCD_CS_PIN, 1);
    dma_busy = false;
}

static void dma_wait(void) {
    while (dma_busy) tight_loop_contents();
}

// ── SPI helpers ───────────────────────────────────────────────────────────────

#define SPI_INST  spi0

static inline void cs_lo(void)  { gpio_put(LCD_CS_PIN, 0); }
static inline void cs_hi(void)  { gpio_put(LCD_CS_PIN, 1); }
static inline void dc_cmd(void) { gpio_put(LCD_DC_PIN, 0); }
static inline void dc_dat(void) { gpio_put(LCD_DC_PIN, 1); }

static void cmd(uint8_t c) {
    cs_lo(); dc_cmd();
    spi_write_blocking(SPI_INST, &c, 1);
    cs_hi();
}

static void dat8(uint8_t d) {
    sleep_us(5);
    cs_lo(); dc_dat();
    spi_write_blocking(SPI_INST, &d, 1);
    cs_hi();
}

static void dat_buf(const uint8_t *b, size_t n) {
    cs_lo(); dc_dat();
    spi_write_blocking(SPI_INST, b, n);
    cs_hi();
}

// ── Window address setup ──────────────────────────────────────────────────────

static void set_window(int x0, int y0, int x1, int y1) {
    x0 += COL_OFFSET; x1 += COL_OFFSET;
    y0 += ROW_OFFSET; y1 += ROW_OFFSET;

    uint8_t d[4];

    d[0] = x0 >> 8; d[1] = x0 & 0xFF; d[2] = x1 >> 8; d[3] = x1 & 0xFF;
    cmd(ST_CASET); dat_buf(d, 4);

    d[0] = y0 >> 8; d[1] = y0 & 0xFF; d[2] = y1 >> 8; d[3] = y1 & 0xFF;
    cmd(ST_RASET); dat_buf(d, 4);

    cmd(ST_RAMWR);
}

// ── Register init (same gamma as rp-picones for this ST7789 panel) ────────────

static void reg_init(void) {
    cmd(ST_MADCTL); dat8(MADCTL_VAL);
    cmd(ST_COLMOD); dat8(0x05);          // 16-bit RGB565

    cmd(0xB2); dat8(0x0C); dat8(0x0C); dat8(0x00); dat8(0x33); dat8(0x33);
    cmd(0xB7); dat8(0x35);
    cmd(0xBB); dat8(0x19);
    cmd(0xC0); dat8(0x2C);
    cmd(0xC2); dat8(0x01);
    cmd(0xC3); dat8(0x12);
    cmd(0xC4); dat8(0x20);
    cmd(0xC6); dat8(0x0F);
    cmd(0xD0); dat8(0xA4); dat8(0xA1);

    cmd(0xE0);
    dat8(0xD0); dat8(0x04); dat8(0x0D); dat8(0x11); dat8(0x13); dat8(0x2B);
    dat8(0x3F); dat8(0x54); dat8(0x4C); dat8(0x18); dat8(0x0D); dat8(0x0B);
    dat8(0x1F); dat8(0x23);

    cmd(0xE1);
    dat8(0xD0); dat8(0x04); dat8(0x0C); dat8(0x11); dat8(0x13); dat8(0x2C);
    dat8(0x3F); dat8(0x44); dat8(0x51); dat8(0x2F); dat8(0x1F); dat8(0x1F);
    dat8(0x20); dat8(0x23);

    cmd(ST_INVON);
    cmd(ST_SLPOUT); sleep_ms(120);
    cmd(ST_NORON);
    cmd(ST_DISPON);
}

// ── Public API ────────────────────────────────────────────────────────────────

void lcd_init(void) {
    spi_init(SPI_INST, 40 * 1000 * 1000);
    spi_set_format(SPI_INST, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(LCD_SCK_PIN,  GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);

    gpio_init(LCD_CS_PIN);  gpio_set_dir(LCD_CS_PIN,  GPIO_OUT); gpio_put(LCD_CS_PIN,  1);
    gpio_init(LCD_DC_PIN);  gpio_set_dir(LCD_DC_PIN,  GPIO_OUT);
    gpio_init(LCD_RST_PIN); gpio_set_dir(LCD_RST_PIN, GPIO_OUT);

    // Backlight PWM
    float clk = (float)clock_get_hz(clk_sys);
    gpio_set_function(LCD_BL_PIN, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(LCD_BL_PIN);
    pwm_chan  = pwm_gpio_to_channel(LCD_BL_PIN);
    pwm_set_clkdiv(pwm_slice, clk / (PWM_FREQ * PWM_WRAP));
    pwm_set_wrap(pwm_slice, PWM_WRAP);
    pwm_set_chan_level(pwm_slice, pwm_chan, 0);
    pwm_set_enabled(pwm_slice, true);

    // DMA channel for bulk pixel transfers
    dma_ch = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_ch);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_dreq(&cfg, spi_get_dreq(SPI_INST, true));
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    dma_channel_configure(dma_ch, &cfg, &spi_get_hw(SPI_INST)->dr, NULL, 0, false);
    dma_channel_set_irq0_enabled(dma_ch, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Hardware reset
    gpio_put(LCD_RST_PIN, 1); sleep_ms(50);
    gpio_put(LCD_RST_PIN, 0); sleep_ms(50);
    gpio_put(LCD_RST_PIN, 1); sleep_ms(150);

    reg_init();
    lcd_set_backlight(80);
}

void lcd_set_backlight(uint8_t pct) {
    if (pct > 100) pct = 100;
    pwm_set_chan_level(pwm_slice, pwm_chan, PWM_WRAP / 100 * pct);
}

void lcd_flush_region(const uint16_t *fb, int x, int y, int w, int h) {
    dma_wait();
    set_window(x, y, x + w - 1, y + h - 1);
    cs_lo(); dc_dat();
    for (int row = 0; row < h; row++) {
        const uint8_t *src = (const uint8_t *)&fb[(y + row) * LCD_W + x];
        spi_write_blocking(SPI_INST, src, (size_t)w * 2);
    }
    cs_hi();
}

void lcd_flush_full(const uint16_t *fb) {
    dma_wait();
    set_window(0, 0, LCD_W - 1, LCD_H - 1);
    cs_lo(); dc_dat();
    dma_busy = true;
    dma_channel_set_trans_count(dma_ch, LCD_W * LCD_H * 2, false);
    dma_channel_set_read_addr(dma_ch, fb, true);
    dma_wait();
}
