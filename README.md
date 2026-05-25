# ardu-commander

USB HID shortcut-key device based on the Waveshare RP2040-Zero.

Rotary encoders and buttons send keyboard/mouse shortcuts to a host Mac (or PC).
An ST7789 240×240 LCD shows the active app, current profile, and all button assignments.
ENC1 cycles through profiles; ENC2 and six buttons send per-profile shortcuts.

## Hardware

| Part | Role |
|------|------|
| Waveshare RP2040-Zero | MCU, native USB HID |
| ST7789 240×240 LCD | Status display (SPI0) |
| ENC1 (GP14/15) | Profile navigation |
| ENC2 (GP28/29) | Per-profile HID encoder |
| BTN A–D (GP8–11) | Per-profile shortcuts |
| BTN X (GP12) | Per-profile shortcut, left side |
| BTN Y (GP13) | Per-profile shortcut, right side |

See **[docs/WIRING.md](docs/WIRING.md)** for the full pin/wiring table.

## Apps and profiles

| App | Profiles |
|-----|---------|
| Chrome | Browsing |
| Claude | Chat |
| Fusion 360 | Sketch, Modeling |
| KiCad | Schematics, PCB |
| Krita | Painting |

ENC1 cycles through all profiles in order. The top LCD row highlights the active app icon.

## Build

Requires Docker and `../pico-sdk`.

```bash
make docker-build   # one-time setup
make quick          # build + flash (hold BOOTSEL on RP2040 before connecting)
make monitor        # UART console via USB-serial adapter on GP0 (TX) / GP1 (RX)
```

## Customising shortcuts

Edit `src/profiles.c`. Each profile defines ENC2 CW/CCW, X, Y, A, B, C, D actions
using the `KEY(modifier, keycode, label)` or `SCR(delta, label)` macros.
Shortcuts default to **macOS** (Cmd = `_G`). Change `_G` → `_C` for Windows.

See **CLAUDE.md** for the full architecture reference.
