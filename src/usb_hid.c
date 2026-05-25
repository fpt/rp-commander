// SPDX-License-Identifier: MIT
// USB HID task — queues keyboard/mouse reports through TinyUSB

#include "usb_hid.h"
#include "tusb.h"
#include "pico/stdlib.h"

#define KEY_HOLD_MS  30   // time between key-down and key-up reports

// ── Keyboard queue ─────────────────────────────────────────────────────────────

#define KEY_QUEUE_LEN 8

typedef struct { uint8_t mod; uint8_t key; } key_event_t;

static key_event_t key_queue[KEY_QUEUE_LEN];
static int kq_head = 0, kq_tail = 0;

static bool kq_push(uint8_t mod, uint8_t key) {
    int next = (kq_tail + 1) % KEY_QUEUE_LEN;
    if (next == kq_head) return false;    // full
    key_queue[kq_tail] = (key_event_t){mod, key};
    kq_tail = next;
    return true;
}

static bool kq_pop(key_event_t *out) {
    if (kq_head == kq_tail) return false;
    *out = key_queue[kq_head];
    kq_head = (kq_head + 1) % KEY_QUEUE_LEN;
    return true;
}

// ── State machine ─────────────────────────────────────────────────────────────

typedef enum { S_IDLE, S_KEY_DOWN, S_KEY_UP } key_state_t;

static key_state_t  kstate     = S_IDLE;
static key_event_t  cur_key    = {0, 0};
static uint32_t     kstate_ms  = 0;

// ── Scroll queue ──────────────────────────────────────────────────────────────

#define SCROLL_QUEUE_LEN 16

static int8_t  scroll_queue[SCROLL_QUEUE_LEN];
static int sq_head = 0, sq_tail = 0;

static bool sq_push(int8_t v) {
    int next = (sq_tail + 1) % SCROLL_QUEUE_LEN;
    if (next == sq_head) return false;
    scroll_queue[sq_tail] = v;
    sq_tail = next;
    return true;
}

static bool sq_pop(int8_t *out) {
    if (sq_head == sq_tail) return false;
    *out = scroll_queue[sq_head];
    sq_head = (sq_head + 1) % SCROLL_QUEUE_LEN;
    return true;
}

// ── Public API ────────────────────────────────────────────────────────────────

void usb_hid_key(uint8_t mod, uint8_t key) {
    kq_push(mod, key);
}

void usb_hid_scroll(int8_t amount) {
    sq_push(amount);
}

void usb_hid_task(void) {
    tud_task();

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // ── Keyboard state machine ─────────────────────────────────────────────────
    if (tud_hid_ready()) {
        switch (kstate) {
        case S_IDLE: {
            key_event_t ev;
            if (kq_pop(&ev)) {
                cur_key  = ev;
                uint8_t keys[6] = {ev.key, 0, 0, 0, 0, 0};
                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, ev.mod, keys);
                kstate    = S_KEY_DOWN;
                kstate_ms = now;
            }
            break;
        }
        case S_KEY_DOWN:
            if ((now - kstate_ms) >= KEY_HOLD_MS) {
                uint8_t keys[6] = {0};
                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keys);
                kstate    = S_KEY_UP;
                kstate_ms = now;
            }
            break;
        case S_KEY_UP:
            if ((now - kstate_ms) >= 5) {
                kstate = S_IDLE;
            }
            break;
        }
    }

    // ── Mouse scroll (send immediately when keyboard is idle) ──────────────────
    if (kstate == S_IDLE && tud_hid_ready()) {
        int8_t sv;
        if (sq_pop(&sv)) {
            tud_hid_mouse_report(REPORT_ID_MOUSE, 0, 0, 0, sv, 0);
        }
    }
}
