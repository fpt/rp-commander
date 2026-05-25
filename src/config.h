// SPDX-License-Identifier: MIT
// ardu-commander: hardware pin assignments
//
// Board: Waveshare RP2040-Zero
// Available GPIO: GP0-15, GP26-29  (GP16 = onboard WS2812)

#ifndef _CONFIG_H_
#define _CONFIG_H_

// ── LCD (ST7789 240×240, SPI0) ────────────────────────────────────────────────
#define LCD_SCK_PIN   2
#define LCD_MOSI_PIN  3
#define LCD_RST_PIN   4
#define LCD_BL_PIN    5
#define LCD_DC_PIN    6
#define LCD_CS_PIN    7

// ── Buttons (active-low, internal pull-up) ────────────────────────────────────
#define BTN_A_PIN   8
#define BTN_B_PIN   9
#define BTN_C_PIN  10
#define BTN_D_PIN  11
#define BTN_X_PIN  12   // left-side button (paired with ENC1)
#define BTN_Y_PIN  13   // right-side button (paired with ENC2)

// ── Rotary encoders ───────────────────────────────────────────────────────────
#define ENC1_A_PIN  14   // ENC1: profile navigation only
#define ENC1_B_PIN  15
#define ENC2_A_PIN  28   // ENC2: per-profile HID action
#define ENC2_B_PIN  29

// ── Optional interfaces ───────────────────────────────────────────────────────
// GP0  = PIO USB D−  (optional second USB port)
// GP1  = PIO USB D+
// GP26 = Grove SDA   (Wire1 / I2C1)
// GP27 = Grove SCL
// GP16 = Onboard WS2812 (do not use as GPIO)

// ── Timing constants ──────────────────────────────────────────────────────────
#define BUTTON_DEBOUNCE_MS   20
#define ENC_DELTA_TIMEOUT_MS  5   // flush encoder delta to HID after this idle

#endif // _CONFIG_H_
