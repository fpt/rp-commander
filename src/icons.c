// SPDX-License-Identifier: MIT
// App icon renderers — simple geometric shapes in RGB565

#include "icons.h"
#include "lcd.h"
#include "profiles.h"

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

// ── Claude: "C" shape in Anthropic orange ────────────────────────────────────

static void icon_claude(uint16_t *fb, int x, int y, int sz, bool sel) {
    uint16_t bg  = RGB565( 20,  20,  20);
    uint16_t acc = RGB565(210, 110,  50);
    uint16_t brd = sel ? COL_WHITE : COL_LGRAY;
    int t = sz / 7;    // thickness
    fill_rect(fb, x, y, sz, sz, bg);
    // Outer "C": top bar, left side, bottom bar
    fill_rect(fb, x + t,     y + t,        sz - 2*t, t,  acc);  // top
    fill_rect(fb, x + t,     y + t,        t, sz - 2*t, acc);  // left
    fill_rect(fb, x + t,     y + sz - 2*t, sz - 2*t, t,  acc);  // bottom
    // Gap on right — leave open (C shape)
    if (sel) draw_rect(fb, x, y, sz, sz, brd);
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

// ── KiCad: PCB trace — two pads connected by a green track ───────────────────

static void icon_kicad(uint16_t *fb, int x, int y, int sz, bool sel) {
    uint16_t bg  = RGB565( 10,  30,  10);
    uint16_t trk = RGB565( 40, 200,  40);
    uint16_t brd = sel ? COL_WHITE : COL_LGRAY;
    int t = sz / 5;
    fill_rect(fb, x, y, sz, sz, bg);
    // Left pad
    fill_rect(fb, x + t/2, y + sz/2 - t/2, t, t, trk);
    // Right pad
    fill_rect(fb, x + sz - t/2 - t, y + sz/2 - t/2, t, t, trk);
    // Horizontal track connecting pads
    fill_rect(fb, x + t/2 + t, y + sz/2 - 1, sz - 2*t - t, 3, trk);
    // Via (small square at centre)
    int vm = sz / 2 - t/4;
    fill_rect(fb, x + vm, y + vm, t/2, t/2, COL_YELLOW);
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

// ── Dispatch ──────────────────────────────────────────────────────────────────

void icon_draw(uint16_t *fb, int x, int y, int size, int icon_idx, bool selected) {
    switch (icon_idx) {
    case ICON_CHROME: icon_chrome(fb, x, y, size, selected); break;
    case ICON_CLAUDE: icon_claude(fb, x, y, size, selected); break;
    case ICON_FUSION: icon_fusion(fb, x, y, size, selected); break;
    case ICON_KICAD:  icon_kicad(fb, x, y, size, selected); break;
    case ICON_KRITA:  icon_krita(fb, x, y, size, selected); break;
    default:
        fill_rect(fb, x, y, size, size, COL_GRAY);
        break;
    }
}
