// SPDX-License-Identifier: MIT
// UI renderer — 3-row layout for 240×240 LCD

#ifndef _UI_H_
#define _UI_H_

#include <stdint.h>
#include <stdbool.h>

// ── Layout constants (all in pixels) ─────────────────────────────────────────
//
//  y=  0 ┌─────────────────────────────────────┐
//        │  App strip  (5 icons × 48 px wide)  │ h=48
//  y= 48 ├─────────────────────────────────────┤
//        │  App name + profile + dots          │ h=80
//  y=128 ├──────────────────┬──────────────────┤
//        │ ENC1: ← Profile→ │ ENC2: <label>    │ h=52
//        │ X: <action>      │ Y: <action>      │
//  y=180 ├──────┬──────┬────┴──────┬──────┐   │
//        │  A   │  B   │  C   │  D        │   │ h=60
//  y=240 └──────┴──────┴──────┴───────────┘

#define UI_STRIP_Y    0
#define UI_STRIP_H   48
#define UI_MID_Y     48
#define UI_MID_H     80
#define UI_ENC_Y    128
#define UI_ENC_H     52
#define UI_BTN_Y    180
#define UI_BTN_H     60

// Mid-row large icon region (used for Claude animation partial flush)
#define UI_MID_ICON_X    4
#define UI_MID_ICON_Y   (UI_MID_Y + 16)
#define UI_MID_ICON_SZ  48

// ── API ───────────────────────────────────────────────────────────────────────

// Full redraw into framebuf. Call after any state change.
void ui_draw(uint16_t *fb, int profile_idx);

// Partial redraws (cheaper; use when only one zone changed)
void ui_draw_strip(uint16_t *fb, int profile_idx);    // top row
void ui_draw_mid(uint16_t *fb, int profile_idx);      // middle row
void ui_draw_enc(uint16_t *fb, int profile_idx);      // encoder row
void ui_draw_btns(uint16_t *fb, int profile_idx,
                  uint8_t held_mask);                 // button row

// Advance the Claude mascot bounce animation.  Returns true when the strip
// cell was redrawn and the caller should flush (0, UI_STRIP_Y, LCD_W, UI_STRIP_H).
// No-op (returns false) when the active app is not Claude.
bool ui_claude_anim_tick(uint16_t *fb, int profile_idx);

#endif // _UI_H_
