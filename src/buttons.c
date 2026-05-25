// SPDX-License-Identifier: MIT
#include "buttons.h"
#include "config.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

static const int PINS[BTN_COUNT] = {
    BTN_A_PIN, BTN_B_PIN, BTN_C_PIN, BTN_D_PIN, BTN_X_PIN, BTN_Y_PIN
};

static bool     raw[BTN_COUNT];
static bool     state[BTN_COUNT];
static uint32_t last_change_ms[BTN_COUNT];

void buttons_init(void) {
    for (int i = 0; i < BTN_COUNT; i++) {
        gpio_init(PINS[i]);
        gpio_set_dir(PINS[i], GPIO_IN);
        gpio_pull_up(PINS[i]);
        raw[i]           = false;
        state[i]         = false;
        last_change_ms[i] = 0;
    }
}

uint8_t buttons_scan(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint8_t pressed = 0;

    for (int i = 0; i < BTN_COUNT; i++) {
        bool cur = (gpio_get(PINS[i]) == 0);  // active-low
        if (cur != raw[i]) {
            raw[i]           = cur;
            last_change_ms[i] = now;
        }
        if ((now - last_change_ms[i]) >= BUTTON_DEBOUNCE_MS) {
            if (cur && !state[i]) {
                pressed |= (1u << i);   // rising edge after debounce
            }
            state[i] = cur;
        }
    }
    return pressed;
}

bool button_held(btn_id_t id) {
    return state[id];
}
