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
#include "cdc_config.h"
#include "config_store.h"

// ── Framebuffer ───────────────────────────────────────────────────────────────

static uint16_t fb[LCD_W * LCD_H];

// ── State ─────────────────────────────────────────────────────────────────────

static int      cur_profile   = 0;
static uint8_t  held_mask     = 0;
static bool     lcd_on        = true;
static uint32_t last_event_ms = 0;

#define LCD_SLEEP_MS  30000U   // 30 seconds of inactivity → backlight off

// Returns true if the LCD was sleeping and has just been woken.
// On wake: drains encoder accumulators so no phantom movement.
static bool try_wake(void) {
    if (lcd_on) return false;
    lcd_set_backlight(80);
    lcd_on        = true;
    last_event_ms = to_ms_since_boot(get_absolute_time());
    encoder_consume(0);
    encoder_consume(1);
    return true;
}

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

    // Load saved config from flash
    static char cfg_json[4096];
    if (config_store_load(cfg_json, sizeof(cfg_json)) > 0)
        profiles_from_json(cfg_json);

    // Initial UI
    ui_draw(fb, cur_profile);
    lcd_flush_full(fb);

    uint8_t  prev_held = 0;
    last_event_ms      = to_ms_since_boot(get_absolute_time());

    while (1) {
        // ── USB ─────────────────────────────────────────────────────────────
        usb_hid_task();
        cdc_config_task();

        // ── Consume inputs (must happen every loop to keep queues clear) ────
        int     d1      = encoder_consume(0);
        int     d2      = encoder_consume(1);
        uint8_t pressed = buttons_scan();

        bool any = (d1 != 0 || d2 != 0 || pressed != 0);

        // ── Wake from sleep ─────────────────────────────────────────────────
        // Any input wakes the LCD; the triggering input is discarded so
        // no accidental profile-change or keypress fires on wake.
        if (any && try_wake()) continue;

        // ── Sleep check ─────────────────────────────────────────────────────
        if (lcd_on && to_ms_since_boot(get_absolute_time()) - last_event_ms > LCD_SLEEP_MS) {
            lcd_set_backlight(0);
            lcd_on = false;
        }

        if (!lcd_on) continue;   // nothing more to do while dark

        // ── ENC1: profile navigation ────────────────────────────────────────
        // EC11 emits 2 raw counts per detent; negate to invert direction.
        static int enc1_rem = 0;
        enc1_rem -= d1;                    // negate = invert physical direction
        int enc1_steps = enc1_rem / 2;    // 1 step per click
        enc1_rem -= enc1_steps * 2;

        if (enc1_steps != 0) {
            last_event_ms = to_ms_since_boot(get_absolute_time());
            cur_profile = (cur_profile + enc1_steps % g_num_profiles + g_num_profiles)
                          % g_num_profiles;
            ui_draw(fb, cur_profile);
            lcd_flush_full(fb);
            printf("profile %d: %s\n", cur_profile,
                   g_profiles[cur_profile].name);
        }

        // ── ENC2: HID action ────────────────────────────────────────────────
        if (d2 != 0) {
            last_event_ms = to_ms_since_boot(get_absolute_time());
            const profile_t *p     = &g_profiles[cur_profile];
            int              steps = d2 > 0 ? d2 : -d2;
            const action_t  *act   = d2 > 0 ? &p->enc2_ccw : &p->enc2_cw;  // inverted
            for (int i = 0; i < steps; i++) dispatch(act);
        }

        // ── Buttons ─────────────────────────────────────────────────────────
        const profile_t *p = &g_profiles[cur_profile];
        const action_t  *btn_acts[BTN_COUNT] = {
            &p->btn_a, &p->btn_b, &p->btn_c, &p->btn_d,
            &p->btn_x, &p->btn_y,
        };

        if (pressed) {
            last_event_ms = to_ms_since_boot(get_absolute_time());
            for (int i = 0; i < BTN_COUNT; i++) {
                if (pressed & (1u << i)) dispatch(btn_acts[i]);
            }
        }

        // Recompute held mask for A-D (indices 0-3)
        held_mask = 0;
        for (int i = 0; i < 4; i++) {
            if (button_held((btn_id_t)i)) held_mask |= (1u << i);
        }

        if (held_mask != prev_held) {
            prev_held = held_mask;
            ui_draw_btns(fb, cur_profile, held_mask);
            lcd_flush_region(fb, 0, UI_BTN_Y, LCD_W, UI_BTN_H);
        }

        // ── Claude mid-icon animation ────────────────────────────────────────
        if (ui_claude_anim_tick(fb, cur_profile))
            lcd_flush_region(fb, UI_MID_ICON_X, UI_MID_ICON_Y,
                             UI_MID_ICON_SZ, UI_MID_ICON_SZ);
    }

    return 0;
}
