// SPDX-License-Identifier: MIT
// Quadrature encoder driver (GPIO interrupt, RP2040)

#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <stdint.h>

// Call once during init — registers the shared GPIO IRQ handler
void encoder_init(void);

// Consume accumulated delta since last call (positive = CW, negative = CCW).
// Thread-safe: disables IRQ briefly to read atomically.
int encoder_consume(int enc_idx);   // enc_idx: 0=ENC1, 1=ENC2

#endif // _ENCODER_H_
