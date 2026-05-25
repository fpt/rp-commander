// SPDX-License-Identifier: MIT
// App and profile definitions for ardu-commander
//
// Platform: macOS (modifier key = LEFTGUI = Command).
// For Windows: replace MOD_META with KEYBOARD_MODIFIER_LEFTCTRL.

#include "profiles.h"

// Modifier shortcuts
#define _C   KEYBOARD_MODIFIER_LEFTCTRL
#define _S   KEYBOARD_MODIFIER_LEFTSHIFT
#define _A   KEYBOARD_MODIFIER_LEFTALT
#define _G   KEYBOARD_MODIFIER_LEFTGUI    // Cmd on Mac

// Action constructors
#define NONE_ACT          {ACT_NONE, 0, 0, 0, "---"}
#define KEY(m,k,l)        {ACT_KEY,  (m), HID_KEY_##k, 0, (l)}
#define SCR(d,l)          {ACT_SCROLL, 0, 0, (d), (l)}

// ── Apps ─────────────────────────────────────────────────────────────────────

const app_t g_apps[] = {
    [ICON_CHROME] = {"Chrome",  ICON_CHROME},
    [ICON_CLAUDE] = {"Claude",  ICON_CLAUDE},
    [ICON_FUSION] = {"Fusion",  ICON_FUSION},
    [ICON_KICAD]  = {"KiCad",   ICON_KICAD},
    [ICON_KRITA]  = {"Krita",   ICON_KRITA},
};
const int g_num_apps = 5;

// ── Profiles ──────────────────────────────────────────────────────────────────
// Order determines ENC1 cycle sequence.

const profile_t g_profiles[] = {

    // ── Chrome / Browsing ─────────────────────────────────────────────────────
    {
        .app_idx    = ICON_CHROME,
        .name       = "Browsing",
        .enc2_cw    = {ACT_KEY, _C,    HID_KEY_TAB,         0, "Next→"},
        .enc2_ccw   = {ACT_KEY, _C|_S, HID_KEY_TAB,         0, "←Prev"},
        .enc2_label = "Tab",
        .btn_x      = KEY(_G,   EQUAL,        "Zoom+"),
        .btn_y      = KEY(_G,   MINUS,        "Zoom-"),
        .btn_a      = KEY(_G,   BRACKET_LEFT, "Back"),
        .btn_b      = KEY(_G,   BRACKET_RIGHT,"Fwd"),
        .btn_c      = KEY(_G,   R,            "Reload"),
        .btn_d      = KEY(_G,   T,            "New Tab"),
    },

    // ── Claude / Chat ─────────────────────────────────────────────────────────
    {
        .app_idx    = ICON_CLAUDE,
        .name       = "Chat",
        .enc2_cw    = SCR(-1, "Down"),
        .enc2_ccw   = SCR(+1, "Up"),
        .enc2_label = "Scroll",
        .btn_x      = KEY(_S,   TAB,    "S+Tab"),
        .btn_y      = KEY(0,    RETURN, "Enter"),
        .btn_a      = KEY(0,    1,      "1"),
        .btn_b      = KEY(0,    2,      "2"),
        .btn_c      = KEY(0,    3,      "3"),
        .btn_d      = KEY(0,    4,      "4"),
    },

    // ── Fusion 360 / Sketch ───────────────────────────────────────────────────
    {
        .app_idx    = ICON_FUSION,
        .name       = "Sketch",
        .enc2_cw    = SCR(-1, "Zoom+"),
        .enc2_ccw   = SCR(+1, "Zoom-"),
        .enc2_label = "Zoom",
        .btn_x      = KEY(0,   F, "Fit"),
        .btn_y      = KEY(0,   S, "Sketch"),
        .btn_a      = KEY(_G,  Z, "Undo"),
        .btn_b      = KEY(_G|_S, Z, "Redo"),
        .btn_c      = KEY(0,   E, "Extrude"),
        .btn_d      = KEY(0,   L, "Line"),
    },

    // ── Fusion 360 / Modeling ─────────────────────────────────────────────────
    {
        .app_idx    = ICON_FUSION,
        .name       = "Modeling",
        .enc2_cw    = SCR(-1, "Zoom+"),
        .enc2_ccw   = SCR(+1, "Zoom-"),
        .enc2_label = "Zoom",
        .btn_x      = KEY(0,   F, "Fit"),
        .btn_y      = KEY(0,   Q, "Prs/Pull"),
        .btn_a      = KEY(_G,  Z, "Undo"),
        .btn_b      = KEY(_G|_S, Z, "Redo"),
        .btn_c      = KEY(0,   M, "Move"),
        .btn_d      = KEY(0,   V, "Visible"),
    },

    // ── KiCad / Schematics ────────────────────────────────────────────────────
    {
        .app_idx    = ICON_KICAD,
        .name       = "Schematics",
        .enc2_cw    = SCR(-1, "Zoom+"),
        .enc2_ccw   = SCR(+1, "Zoom-"),
        .enc2_label = "Zoom",
        .btn_x      = {ACT_KEY, _C, HID_KEY_0, 0, "Fit"},
        .btn_y      = KEY(0,   W, "Wire"),
        .btn_a      = KEY(_C,  Z, "Undo"),
        .btn_b      = KEY(_C,  Y, "Redo"),
        .btn_c      = KEY(0,   L, "Label"),
        .btn_d      = KEY(0,   A, "Add Part"),
    },

    // ── KiCad / PCB ───────────────────────────────────────────────────────────
    {
        .app_idx    = ICON_KICAD,
        .name       = "PCB",
        .enc2_cw    = SCR(-1, "Zoom+"),
        .enc2_ccw   = SCR(+1, "Zoom-"),
        .enc2_label = "Zoom",
        .btn_x      = {ACT_KEY, _C, HID_KEY_0, 0, "Fit"},
        .btn_y      = KEY(0,   X, "Route"),
        .btn_a      = KEY(_C,  Z, "Undo"),
        .btn_b      = KEY(_C,  Y, "Redo"),
        .btn_c      = KEY(0,   F, "Flip"),
        .btn_d      = KEY(0,   U, "Update"),
    },

    // ── Krita / Painting ──────────────────────────────────────────────────────
    {
        .app_idx    = ICON_KRITA,
        .name       = "Painting",
        .enc2_cw    = KEY(0, BRACKET_RIGHT, "Size+"),
        .enc2_ccw   = KEY(0, BRACKET_LEFT,  "Size-"),
        .enc2_label = "Size",
        .btn_x      = KEY(0,   E,  "Eraser"),
        .btn_y      = KEY(0,   K,  "Eyedrop"),
        .btn_a      = KEY(_C,  Z,  "Undo"),
        .btn_b      = KEY(_C|_S, Z, "Redo"),
        .btn_c      = KEY(0,   F,  "Fill"),
        .btn_d      = KEY(0,   TAB, "Canvas"),
    },
};

const int g_num_profiles = (int)(sizeof(g_profiles) / sizeof(g_profiles[0]));

// ── Helpers ───────────────────────────────────────────────────────────────────

int profile_app(int idx) {
    if (idx < 0 || idx >= g_num_profiles) return 0;
    return g_profiles[idx].app_idx;
}

int profile_first_for_app(int app_idx) {
    for (int i = 0; i < g_num_profiles; i++)
        if (g_profiles[i].app_idx == app_idx) return i;
    return 0;
}
