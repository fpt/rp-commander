# CLAUDE.md — RP2040-Zero USB HID Commander

## Hardware

| Component | Part | Interface |
|-----------|------|-----------|
| MCU | Waveshare RP2040-Zero | USB-C (TinyUSB HID) |
| Display | ST7789 SPI LCD 240×240 | Hardware SPI (SPI0) |
| Encoders | 2–4 × EC11 rotary encoders w/ push switch | GPIO (interrupt) |
| Switches | Tactile switches | GPIO (pull-up) |

## RP2040-Zero Pin Availability

Only pads GP0–GP15 and GP26–GP29 are accessible. GP16 is the onboard WS2812 RGB LED
(not a general-purpose pad).

Total available: 20 GPIO pins.

## Pin Assignment (provisional — adjust in each sketch's `pins.h`)

### ST7789 — SPI0

| Signal | GPIO |
|--------|------|
| SCK    | GP2  |
| MOSI   | GP3  |
| RST    | GP4  |
| BL     | GP5  |
| DC     | GP6  |
| CS     | GP7  |

Use the 3-arg Adafruit_ST7789 constructor (hardware SPI). BL is a plain GPIO output
(HIGH = backlight on):
```cpp
SPI.setSCK(2);
SPI.setTX(3);
pinMode(5, OUTPUT);
digitalWrite(5, HIGH);  // backlight on
Adafruit_ST7789 tft(7, 6, 4);  // CS, DC, RST
tft.init(240, 240);
```

### Rotary Encoders — GPIO interrupts

| Encoder | A    | B    |
|---------|------|------|
| ENC1    | GP14 | GP15 |
| ENC2    | GP28 | GP29 |

Wire A/B to GPIO with internal pull-ups (`INPUT_PULLUP`).

Encoder ISR pattern (Philhower core supports interrupts on any GPIO):
```cpp
volatile int enc_pos = 0;

void enc_isr() {
  int a = digitalRead(ENC_A);
  int b = digitalRead(ENC_B);
  enc_pos += (a == b) ? 1 : -1;
}

// In setup():
pinMode(ENC_A, INPUT_PULLUP);
pinMode(ENC_B, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(ENC_A), enc_isr, CHANGE);
```

### Buttons / Switches

| Button | GPIO |
|--------|------|
| BTN A  | GP8  |
| BTN B  | GP9  |
| BTN C  | GP10 |
| BTN D  | GP11 |
| BTN X  | GP12 |
| BTN Y  | GP13 |

All buttons wired with `INPUT_PULLUP`. Active-low.

## Optional Interfaces

### PIO USB — second USB port (GP0/GP1)

GP0 (D−) and GP1 (D+) are reserved for a PIO-based USB port using the `pio-usb` library.
This lets the RP2040-Zero act as a USB host (or second device) on those pins while the
native USB-C remains the primary HID device.

```cpp
// Requires "Adafruit_TinyUSB" + "pio-usb" library
#include <pio_usb.h>
// pin_dp = 1 (GP1, D+), pin_dm = 0 (GP0, D−)
```

Leave GP0/GP1 unconnected when the PIO USB port is not used.

### Grove I2C — expansion (GP26/GP27)

GP26 = SDA, GP27 = SCL. These are I2C1 (`Wire1`) pins on RP2040.

```cpp
Wire1.setSDA(26);
Wire1.setSCL(27);
Wire1.begin();
Wire1.setClock(400000);
```

Compatible with any Grove I2C sensor/module (3.3 V logic).

## USB HID (TinyUSB via Philhower core)

Board selection: **Raspberry Pi RP2040** → **Waveshare RP2040 Zero**  
USB Stack setting: **Adafruit TinyUSB**

For a consumer/multimedia HID controller use `<HID-Project.h>`:
- `Consumer.begin()` — media keys (play/pause, volume, etc.)
- `Gamepad.begin()` — gamepad axes and buttons
- `Keyboard.begin()` / `Mouse.begin()` — standard HID (no extra library)

For custom HID descriptor (e.g. knob surface), use TinyUSB directly with a raw
HID report descriptor.

```cpp
#include <Adafruit_TinyUSB.h>
// or for simple keyboard/consumer:
#include "HID-Project.h"
```

### Step 2 approach: Consumer + Keyboard

Each encoder delta → Consumer volume / media control or Keyboard shortcut.
Buttons → Keyboard modifiers or Consumer keys.
LCD → status display (current mode, values).

## Arduino Libraries

| Library | Header | Purpose |
|---------|--------|---------|
| `Adafruit GFX Library` | `<Adafruit_GFX.h>` | Graphics primitives |
| `Adafruit ST7789` | `<Adafruit_ST7789.h>` | Display driver |
| `HID-Project` | `<HID-Project.h>` | Consumer/Gamepad/Keyboard HID |

## Files

| File | Purpose |
|------|---------|
| `step1_hw_test/step1_hw_test.ino` | LCD + encoder + switch bringup test |
| `step2_usb_hid/step2_usb_hid.ino` | Full USB HID commander |
| `WIRING.md` | Physical wiring reference |
