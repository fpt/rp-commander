// SPDX-License-Identifier: MIT
// UI renderer for ardu-commander — 240×240 LCD, 3-row layout

#include "ui.h"
#include "lcd.h"
#include "font.h"
#include "icons.h"
#include "profiles.h"

#include <string.h>

// ── Drawing primitives ────────────────────────────────────────────────────────

static void fill(uint16_t *fb, int x, int y, int w, int h, uint16_t col) {
    for (int ry = y; ry < y + h && ry < LCD_H; ry++)
        for (int rx = x; rx < x + w && rx < LCD_W; rx++)
            fb[ry * LCD_W + rx] = col;
}

static void hline(uint16_t *fb, int x, int y, int w, uint16_t col) {
    fill(fb, x, y, w, 1, col);
}

static void vline(uint16_t *fb, int x, int y, int h, uint16_t col) {
    fill(fb, x, y, 1, h, col);
}

// Draw 8×8 character at pixel (x,y), optional 2× scale
static void draw_char(uint16_t *fb, int x, int y, char c, int scale,
                      uint16_t fg, uint16_t bg) {
    const uint8_t *bm = font_get_char(c);
    for (int row = 0; row < FONT_H; row++) {
        uint8_t byte = bm[row];
        for (int col = 0; col < FONT_W; col++) {
            uint16_t px = (byte & (0x80 >> col)) ? fg : bg;
            for (int sy = 0; sy < scale; sy++)
                for (int sx = 0; sx < scale; sx++) {
                    int px_ = x + col * scale + sx;
                    int py_ = y + row * scale + sy;
                    if (px_ >= 0 && px_ < LCD_W && py_ >= 0 && py_ < LCD_H)
                        fb[py_ * LCD_W + px_] = px;
                }
        }
    }
}

static void draw_str(uint16_t *fb, int x, int y, const char *s, int scale,
                     uint16_t fg, uint16_t bg) {
    while (*s) {
        draw_char(fb, x, y, *s++, scale, fg, bg);
        x += FONT_W * scale;
        if (x + FONT_W * scale > LCD_W) break;
    }
}

// Centre-aligned string within a box
static void draw_str_centre(uint16_t *fb, int box_x, int box_y,
                            int box_w, int box_h,
                            const char *s, int scale,
                            uint16_t fg, uint16_t bg) {
    int len   = (int)strlen(s);
    int tw    = len * FONT_W * scale;
    int th    = FONT_H * scale;
    int sx    = box_x + (box_w - tw) / 2;
    int sy    = box_y + (box_h - th) / 2;
    // Fill box background first
    fill(fb, box_x, box_y, box_w, box_h, bg);
    draw_str(fb, sx, sy, s, scale, fg, bg);
}

// ── App strip (top row) ───────────────────────────────────────────────────────

void ui_draw_strip(uint16_t *fb, int profile_idx) {
    int cur_app = g_profiles[profile_idx].app_idx;
    int cell_w  = LCD_W / g_num_apps;   // 48px for 5 apps

    fill(fb, 0, UI_STRIP_Y, LCD_W, UI_STRIP_H, COL_DGRAY);

    for (int i = 0; i < g_num_apps; i++) {
        int cx   = i * cell_w;
        bool sel = (i == cur_app);

        if (sel) fill(fb, cx, UI_STRIP_Y, cell_w, UI_STRIP_H, RGB565(30, 30, 60));

        int icon_sz = 32;
        int ix = cx + (cell_w - icon_sz) / 2;
        int iy = UI_STRIP_Y + (UI_STRIP_H - icon_sz) / 2;
        icon_draw(fb, ix, iy, icon_sz, g_apps[i].icon_idx, sel);

        // Vertical separator between cells
        if (i > 0) vline(fb, cx, UI_STRIP_Y, UI_STRIP_H, COL_GRAY);
    }

    // Bottom border
    hline(fb, 0, UI_STRIP_Y + UI_STRIP_H - 1, LCD_W, COL_GRAY);
}

// ── Middle row: app name + profile name + indicator dots ─────────────────────

void ui_draw_mid(uint16_t *fb, int profile_idx) {
    const profile_t *p   = &g_profiles[profile_idx];
    int              app  = p->app_idx;

    fill(fb, 0, UI_MID_Y, LCD_W, UI_MID_H, COL_BLACK);

    // Large app icon on the left (48×48)
    icon_draw(fb, 4, UI_MID_Y + 16, 48, g_apps[app].icon_idx, false);

    // App name (scale 1)
    draw_str(fb, 8, UI_MID_Y + UI_MID_H - FONT_H - 4,
             g_apps[app].name, 1, COL_LGRAY, COL_BLACK);

    // Profile name (scale 2, white)
    draw_str(fb, 60, UI_MID_Y + 14, p->name, 2, COL_WHITE, COL_BLACK);

    // Profile indicator dots: one dot per profile that belongs to same app
    int dot_x = 60, dot_y = UI_MID_Y + 48;
    int dot_size = 6, dot_gap = 10;
    for (int i = 0; i < g_num_profiles; i++) {
        if (g_profiles[i].app_idx != app) continue;
        uint16_t col = (i == profile_idx) ? COL_WHITE : COL_GRAY;
        fill(fb, dot_x, dot_y, dot_size, dot_size, col);
        dot_x += dot_gap;
    }

    // Bottom border
    hline(fb, 0, UI_MID_Y + UI_MID_H - 1, LCD_W, COL_GRAY);
}

// ── Encoder row: left = ENC1 (profile nav), right = ENC2 + X/Y ───────────────

void ui_draw_enc(uint16_t *fb, int profile_idx) {
    const profile_t *p = &g_profiles[profile_idx];
    int y = UI_ENC_Y, h = UI_ENC_H;

    fill(fb, 0, y, LCD_W, h, COL_BLACK);
    vline(fb, LCD_W / 2, y, h, COL_GRAY);

    // Left half: ENC1 (fixed: profile navigation)
    draw_str(fb,  4, y +  4, "< Profile >", 1, COL_CYAN, COL_BLACK);
    draw_str(fb,  4, y + 18, "X:", 1, COL_LGRAY, COL_BLACK);
    draw_str(fb, 20, y + 18, p->btn_x.label, 1, COL_WHITE, COL_BLACK);

    // Right half: ENC2 + Y
    int rx = LCD_W / 2 + 4;
    draw_str(fb, rx, y +  4, p->enc2_label, 1, COL_CYAN, COL_BLACK);
    draw_str(fb, rx + (int)strlen(p->enc2_label) * FONT_W + 4,
                  y + 4, p->enc2_cw.label, 1, COL_LGRAY, COL_BLACK);
    draw_str(fb, rx, y + 18, "Y:", 1, COL_LGRAY, COL_BLACK);
    draw_str(fb, rx + 16, y + 18, p->btn_y.label, 1, COL_WHITE, COL_BLACK);

    hline(fb, 0, y + h - 1, LCD_W, COL_GRAY);
}

// ── Button row (A B C D) ──────────────────────────────────────────────────────

void ui_draw_btns(uint16_t *fb, int profile_idx, uint8_t held_mask) {
    const profile_t *p    = &g_profiles[profile_idx];
    const action_t  *acts[4] = {&p->btn_a, &p->btn_b, &p->btn_c, &p->btn_d};
    const char      *names[4] = {"A", "B", "C", "D"};
    int cell_w = LCD_W / 4;
    int y = UI_BTN_Y, h = UI_BTN_H;

    fill(fb, 0, y, LCD_W, h, COL_BLACK);

    for (int i = 0; i < 4; i++) {
        int cx   = i * cell_w;
        bool hld = (held_mask >> i) & 1;
        uint16_t bg = hld ? COL_DGRAY : COL_BLACK;

        fill(fb, cx, y, cell_w, h, bg);
        if (i > 0) vline(fb, cx, y, h, COL_GRAY);

        // Button name top-left
        uint16_t lbl_col = hld ? COL_YELLOW : COL_LGRAY;
        draw_str(fb, cx + 4, y + 4, names[i], 1, lbl_col, bg);

        // Action label centred below
        const char *al = acts[i]->label;
        int tw  = (int)strlen(al) * FONT_W;
        int ax  = cx + (cell_w - tw) / 2;
        draw_str(fb, ax, y + 20, al, 1, COL_WHITE, bg);
    }
}

// ── Full redraw ───────────────────────────────────────────────────────────────

void ui_draw(uint16_t *fb, int profile_idx) {
    ui_draw_strip(fb, profile_idx);
    ui_draw_mid(fb, profile_idx);
    ui_draw_enc(fb, profile_idx);
    ui_draw_btns(fb, profile_idx, 0);
}
