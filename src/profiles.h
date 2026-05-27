// SPDX-License-Identifier: MIT
// App + profile table for ardu-commander
//
// Navigation:
//   ENC1 CW/CCW  → cycle through all profiles (wraps across apps)
//   Display top  → shows current app icon highlighted
//   Display mid  → shows app name + profile name + dot indicator

#ifndef _PROFILES_H_
#define _PROFILES_H_

#include <stdint.h>
#include <stddef.h>
#include "class/hid/hid.h"   // HID_KEY_xxx, KEYBOARD_MODIFIER_xxx

// ── Action (one HID event) ────────────────────────────────────────────────────

typedef enum {
    ACT_NONE   = 0,
    ACT_KEY,        // keyboard key with optional modifier
    ACT_SCROLL,     // mouse scroll wheel (scroll = +1 up / -1 down)
} act_type_t;

typedef struct {
    act_type_t  type;
    uint8_t     mod;      // KEYBOARD_MODIFIER_xxx bitmask
    uint8_t     key;      // HID_KEY_xxx
    int8_t      scroll;   // for ACT_SCROLL
    char        label[9]; // display label (max 8 chars)
} action_t;

// ── Profile ───────────────────────────────────────────────────────────────────

typedef struct {
    uint8_t      icon_idx;     // ICON_xxx constant
    char         name[16];     // display name, e.g. "Chrome"
    // ENC2 (right encoder) action
    action_t     enc2_cw;
    action_t     enc2_ccw;
    char         enc2_label[9]; // short label shown on display
    // Per-button actions
    action_t     btn_x;        // left-side button
    action_t     btn_y;        // right-side button
    action_t     btn_a;
    action_t     btn_b;
    action_t     btn_c;
    action_t     btn_d;
} profile_t;

// App icon indices (also used as profile index in the 1-profile-per-app model)
#define ICON_CHROME  0
#define ICON_CLAUDE  1
#define ICON_FUSION  2
#define ICON_KICAD   3
#define ICON_KRITA   4

// ── Global tables ─────────────────────────────────────────────────────────────

extern profile_t g_profiles[];
extern const int g_num_profiles;

// Serialize all profiles to compact JSON. Returns total length written.
int profiles_to_json(char *buf, size_t bufsize);

// Parse compact JSON and update g_profiles[]. Returns 0 on success, -1 on error.
int profiles_from_json(const char *json);

#endif // _PROFILES_H_
