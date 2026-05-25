// SPDX-License-Identifier: MIT
// USB HID send helpers (keyboard + mouse scroll)

#ifndef _USB_HID_H_
#define _USB_HID_H_

#include <stdint.h>
#include <stdbool.h>

#define REPORT_ID_KEYBOARD  1
#define REPORT_ID_MOUSE     2

// Must be called from main loop every iteration
void usb_hid_task(void);

// Queue a keyboard keypress (auto-releases after ~30 ms)
// mod: KEYBOARD_MODIFIER_xxx bitmask, key: HID_KEY_xxx
void usb_hid_key(uint8_t mod, uint8_t key);

// Send a single mouse scroll tick (+ve = up / away, -ve = down / toward user)
void usb_hid_scroll(int8_t amount);

#endif // _USB_HID_H_
