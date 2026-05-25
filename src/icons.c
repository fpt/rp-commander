// SPDX-License-Identifier: MIT
// App icon renderers — geometric placeholders with optional real icon overlay.
// Run `make icons` to generate src/icons_data.h from resources/ and enable
// real brand icons in place of the geometric shapes below.

#include "icons.h"
#include "lcd.h"
#include "font.h"
#include "profiles.h"

#if __has_include("icons_data.h")
#include "icons_data.h"
#endif

// ── Primitive helpers ────────────────────────────────────────────────────────

static void fill_rect(uint16_t *fb, int x, int y, int w, int h, uint16_t col) {
    for (int ry = y; ry < y + h; ry++) {
        if (ry < 0 || ry >= LCD_H) continue;
        for (int rx = x; rx < x + w; rx++) {
            if (rx < 0 || rx >= LCD_W) continue;
            fb[ry * LCD_W + rx] = col;
        }
    }
}

static void draw_rect(uint16_t *fb, int x, int y, int w, int h, uint16_t col) {
    fill_rect(fb, x,         y,         w, 1, col);
    fill_rect(fb, x,         y + h - 1, w, 1, col);
    fill_rect(fb, x,         y,         1, h, col);
    fill_rect(fb, x + w - 1, y,         1, h, col);
}

// ── Chrome: four coloured quadrants + white ring ──────────────────────────────

static void icon_chrome(uint16_t *fb, int x, int y, int sz, bool sel) {
    int h = sz / 2, q = sz / 4;
    uint16_t dim = sel ? COL_WHITE : COL_LGRAY;
    // Background quadrants
    fill_rect(fb, x,     y,     h, h, RGB565(66, 133, 244));   // blue TL
    fill_rect(fb, x + h, y,     h, h, RGB565(234, 67, 53));    // red  TR
    fill_rect(fb, x,     y + h, h, h, RGB565( 52,168, 83));    // green BL
    fill_rect(fb, x + h, y + h, h, h, RGB565(251,188,  5));    // yellow BR
    // White cross divider
    fill_rect(fb, x,         y + h - 1, sz, 2, COL_WHITE);
    fill_rect(fb, x + h - 1, y,         2, sz, COL_WHITE);
    // White centre circle (approximated)
    fill_rect(fb, x + q - 1, y + q - 1, q + 2, q + 2, COL_WHITE);
    if (sel) draw_rect(fb, x, y, sz, sz, dim);
}

// ── Claude: orange mascot blob with two feet ─────────────────────────────────
//
//   ▐▛███▜▌    rounded orange body
//   ▜█████▛▘
//    ▘▘ ▝▝     two small feet

static void draw_claude_mascot(uint16_t *fb, int x, int y, int sz, bool sel, int dy) {
    uint16_t brd = sel ? COL_WHITE : COL_LGRAY;
    fill_rect(fb, x, y, sz, sz, COL_BLACK);

    int m  = sz / 8;
    int bx = x + m,        bw = sz - 2 * m;
    int bt = y + m + dy,   bh = sz * 5 / 8;

    // Rounded body (skip single-pixel corners)
    fill_rect(fb, bx + 1, bt,     bw - 2, bh,     COL_ORANGE);
    fill_rect(fb, bx,     bt + 1, bw,     bh - 2, COL_ORANGE);

    // Two stubby feet below the body
    int fw = bw / 4,  fh = sz / 8;
    int fy = bt + bh - 1;
    fill_rect(fb, bx + fw / 2,               fy, fw, fh, COL_ORANGE);
    fill_rect(fb, bx + bw - fw - fw / 2, fy, fw, fh, COL_ORANGE);

    if (sel) draw_rect(fb, x, y, sz, sz, brd);
}

static void icon_claude(uint16_t *fb, int x, int y, int sz, bool sel) {
    draw_claude_mascot(fb, x, y, sz, sel, 0);
}

void icon_claude_anim(uint16_t *fb, int x, int y, int sz, bool sel, int frame) {
    static const int8_t dy_tbl[8] = {0, -2, -5, -7, -5, -2, 0, 2};
    draw_claude_mascot(fb, x, y, sz, sel, dy_tbl[frame & 7]);
}

// ── Fusion: bold "F" in blue ──────────────────────────────────────────────────

static void icon_fusion(uint16_t *fb, int x, int y, int sz, bool sel) {
    uint16_t bg  = RGB565( 15,  50, 120);
    uint16_t fg  = COL_WHITE;
    uint16_t brd = sel ? COL_WHITE : COL_LGRAY;
    int t = sz / 6;
    fill_rect(fb, x, y, sz, sz, bg);
    // F: vertical bar
    fill_rect(fb, x + t, y + t, t, sz - 2*t, fg);
    // F: top horizontal
    fill_rect(fb, x + t, y + t, sz - 2*t, t, fg);
    // F: middle horizontal (3/5 height)
    fill_rect(fb, x + t, y + t + (sz - 2*t) * 2 / 5, sz * 3 / 5, t, fg);
    if (sel) draw_rect(fb, x, y, sz, sz, brd);
}

// ── KiCad: "Ki" in green ─────────────────────────────────────────────────────

static void icon_kicad(uint16_t *fb, int x, int y, int sz, bool sel) {
    uint16_t brd = sel ? COL_WHITE : COL_LGRAY;
    fill_rect(fb, x, y, sz, sz, COL_BLACK);

    int scale = sz / 16;                       // 2 at sz=32, 3 at sz=48
    int tw    = 2 * FONT_W * scale;
    int lx    = x + (sz - tw) / 2;
    int ly    = y + (sz - FONT_H * scale) / 2;
    font_draw_char(fb, lx,                ly, 'K', scale, COL_GREEN, COL_BLACK);
    font_draw_char(fb, lx + FONT_W*scale, ly, 'i', scale, COL_GREEN, COL_BLACK);

    if (sel) draw_rect(fb, x, y, sz, sz, brd);
}

// ── Krita: paint drop (diamond + circle base) in purple ──────────────────────

static void icon_krita(uint16_t *fb, int x, int y, int sz, bool sel) {
    uint16_t bg  = RGB565( 20,  10,  30);
    uint16_t drp = RGB565(150,  50, 220);
    uint16_t brd = sel ? COL_WHITE : COL_LGRAY;
    int t = sz / 5;
    fill_rect(fb, x, y, sz, sz, bg);
    int cx = x + sz / 2;
    // Teardrop: triangle-ish top
    for (int row = 0; row < sz / 2; row++) {
        int hw = row * t / (sz / 2 - 1) + 1;
        fill_rect(fb, cx - hw, y + t/2 + row, hw * 2, 1, drp);
    }
    // Round base (thick horizontal bar)
    fill_rect(fb, cx - t, y + sz/2, t * 2, t, drp);
    if (sel) draw_rect(fb, x, y, sz, sz, brd);
}

// ── Real-icon blit (used when icons_data.h is present) ───────────────────────

#ifdef ICON_DATA_SIZE
static void blit_icon(uint16_t *fb, int x, int y, int cell,
                      const uint16_t *data, bool selected) {
    int isz = ICON_DATA_SIZE;
    int off = (cell - isz) / 2;   // centre within cell
    fill_rect(fb, x, y, cell, cell, COL_BLACK);
    for (int row = 0; row < isz; row++) {
        int dy = y + off + row;
        if (dy < 0 || dy >= LCD_H) continue;
        for (int col = 0; col < isz; col++) {
            int dx = x + off + col;
            if (dx < 0 || dx >= LCD_W) continue;
            fb[dy * LCD_W + dx] = data[row * isz + col];
        }
    }
    if (selected) draw_rect(fb, x, y, cell, cell, COL_WHITE);
}
#endif

// ── Dispatch ──────────────────────────────────────────────────────────────────

void icon_draw(uint16_t *fb, int x, int y, int size, int icon_idx, bool selected) {
    switch (icon_idx) {
    case ICON_CHROME:
#ifdef ICON_DATA_CHROME
        blit_icon(fb, x, y, size, icon_data_chrome, selected); break;
#else
        icon_chrome(fb, x, y, size, selected); break;
#endif
    case ICON_CLAUDE:
        // Always use animated geometric mascot; ui_claude_anim_tick() overrides
        // the strip cell when this profile is active.
        icon_claude(fb, x, y, size, selected); break;
    case ICON_FUSION:
#ifdef ICON_DATA_FUSION
        blit_icon(fb, x, y, size, icon_data_fusion, selected); break;
#else
        icon_fusion(fb, x, y, size, selected); break;
#endif
    case ICON_KICAD:
#ifdef ICON_DATA_KICAD
        blit_icon(fb, x, y, size, icon_data_kicad, selected); break;
#else
        icon_kicad(fb, x, y, size, selected); break;
#endif
    case ICON_KRITA:
#ifdef ICON_DATA_KRITA
        blit_icon(fb, x, y, size, icon_data_krita, selected); break;
#else
        icon_krita(fb, x, y, size, selected); break;
#endif
    default:
        fill_rect(fb, x, y, size, size, COL_GRAY);
        break;
    }
}
