// SPDX-License-Identifier: MIT
// Quadrature encoder driver for ardu-commander

#include "encoder.h"
#include "config.h"

#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

static volatile int enc_pos[2]    = {0, 0};
static volatile int enc_last_a[2] = {0, 0};

// Single GPIO callback for both encoders (RP2040 has one callback per bank)
static void gpio_cb(uint gpio, uint32_t events) {
    (void)events;

    int idx = -1, pin_a, pin_b;

    if (gpio == ENC1_A_PIN || gpio == ENC1_B_PIN) {
        idx = 0; pin_a = ENC1_A_PIN; pin_b = ENC1_B_PIN;
    } else if (gpio == ENC2_A_PIN || gpio == ENC2_B_PIN) {
        idx = 1; pin_a = ENC2_A_PIN; pin_b = ENC2_B_PIN;
    }
    if (idx < 0) return;

    int a = gpio_get(pin_a);
    int b = gpio_get(pin_b);
    if (a != enc_last_a[idx]) {
        enc_last_a[idx] = a;
        enc_pos[idx] += (a == b) ? 1 : -1;
    }
}

void encoder_init(void) {
    const int pins[] = {ENC1_A_PIN, ENC1_B_PIN, ENC2_A_PIN, ENC2_B_PIN};
    for (int i = 0; i < 4; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);
    }
    enc_last_a[0] = gpio_get(ENC1_A_PIN);
    enc_last_a[1] = gpio_get(ENC2_A_PIN);

    // Register one shared callback; then enable IRQ on all four A/B pins
    gpio_set_irq_enabled_with_callback(ENC1_A_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, gpio_cb);
    gpio_set_irq_enabled(ENC1_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ENC2_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ENC2_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

int encoder_consume(int idx) {
    if (idx < 0 || idx > 1) return 0;
    uint32_t saved = save_and_disable_interrupts();
    int d = enc_pos[idx];
    enc_pos[idx] = 0;
    restore_interrupts(saved);
    return d;
}
