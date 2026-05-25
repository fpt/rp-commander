// SPDX-License-Identifier: MIT
// ardu-commander — USB HID shortcut device
//
// Hardware: Waveshare RP2040-Zero
//   LCD ST7789 240×240   SPI0  GP2/3/4/5/6/7
//   ENC1 (profile nav)   GPIO  GP14/15
//   ENC2 (HID action)    GPIO  GP28/29
//   Buttons A-D, X, Y   GPIO  GP8-13

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "bsp/board.h"
#include "tusb.h"

#include "config.h"
#include "lcd.h"
#include "font.h"
#include "encoder.h"
#include "buttons.h"
#include "usb_hid.h"
#include "profiles.h"
#include "ui.h"

// ── Framebuffer ───────────────────────────────────────────────────────────────

static uint16_t fb[LCD_W * LCD_H];

// ── State ─────────────────────────────────────────────────────────────────────

static int     cur_profile = 0;
static uint8_t held_mask   = 0;   // which of A-D are currently held

// ── Action dispatch ───────────────────────────────────────────────────────────

static void dispatch(const action_t *a) {
    if (!a) return;
    switch (a->type) {
    case ACT_KEY:
        usb_hid_key(a->mod, a->key);
        break;
    case ACT_SCROLL:
        usb_hid_scroll(a->scroll);
        break;
    default:
        break;
    }
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(void) {
    stdio_init_all();
    printf("\nardu-commander start\n");

    // USB HID init (TinyUSB)
    board_init();
    tud_init(BOARD_TUD_RHPORT);

    // Peripherals
    lcd_init();
    encoder_init();
    buttons_init();

    // Initial UI
    ui_draw(fb, cur_profile);
    lcd_flush_full(fb);

    uint8_t prev_held = 0;

    while (1) {
        // ── USB ─────────────────────────────────────────────────────────────
        usb_hid_task();   // calls tud_task() + manages key/scroll queue

        // ── ENC1: profile navigation ────────────────────────────────────────
        int d1 = encoder_consume(0);
        if (d1 != 0) {
            cur_profile = (cur_profile + d1 % g_num_profiles + g_num_profiles)
                          % g_num_profiles;
            ui_draw(fb, cur_profile);
            lcd_flush_full(fb);
            printf("profile %d: %s / %s\n", cur_profile,
                   g_apps[g_profiles[cur_profile].app_idx].name,
                   g_profiles[cur_profile].name);
        }

        // ── ENC2: HID action ────────────────────────────────────────────────
        int d2 = encoder_consume(1);
        if (d2 != 0) {
            const profile_t *p = &g_profiles[cur_profile];
            int steps = d2 > 0 ? d2 : -d2;
            const action_t *act = d2 > 0 ? &p->enc2_cw : &p->enc2_ccw;
            for (int i = 0; i < steps; i++) dispatch(act);
        }

        // ── Buttons ─────────────────────────────────────────────────────────
        uint8_t pressed = buttons_scan();

        const profile_t *p = &g_profiles[cur_profile];
        const action_t  *btn_acts[BTN_COUNT] = {
            &p->btn_a, &p->btn_b, &p->btn_c, &p->btn_d,
            &p->btn_x, &p->btn_y,
        };

        if (pressed) {
            for (int i = 0; i < BTN_COUNT; i++) {
                if (pressed & (1u << i)) dispatch(btn_acts[i]);
            }
        }

        // Recompute held mask for A-D (indices 0-3)
        held_mask = 0;
        for (int i = 0; i < 4; i++) {
            if (button_held((btn_id_t)i)) held_mask |= (1u << i);
        }

        // Refresh button row if held state changed
        if (held_mask != prev_held) {
            prev_held = held_mask;
            ui_draw_btns(fb, cur_profile, held_mask);
            lcd_flush_region(fb, 0, UI_BTN_Y, LCD_W, UI_BTN_H);
        }
    }

    return 0;
}
