// SPDX-License-Identifier: MIT
// Button driver with debounce — 6 buttons (A B C D X Y)

#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BTN_A = 0,
    BTN_B,
    BTN_C,
    BTN_D,
    BTN_X,
    BTN_Y,
    BTN_COUNT
} btn_id_t;

void buttons_init(void);

// Call every loop iteration. Returns bitmask of newly-pressed buttons (bit = btn_id_t).
uint8_t buttons_scan(void);

// True if the button is currently held down (after debounce)
bool button_held(btn_id_t id);

#endif // _BUTTONS_H_
