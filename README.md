# rp-commander

USB HID shortcut-key device based on the Waveshare RP2040-Zero.

Two rotary encoders and six buttons send keyboard/mouse shortcuts to a host Mac (or PC).
An ST7789 240×240 LCD shows the active app, current profile, and all button assignments.
ENC1 cycles through profiles; ENC2 and the six buttons send per-profile shortcuts.

Profiles are configurable at runtime via a USB CDC serial port — no reflashing required.

## Hardware

| Part | Role |
|------|------|
| Waveshare RP2040-Zero | MCU, native USB (HID + CDC) |
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
Profiles saved to flash persist across power cycles.

## Build

Requires Docker and `../pico-sdk` (or set `PICO_SDK_PATH`).

```bash
make docker-build   # one-time Docker image setup
make quick          # build + flash (hold BOOTSEL on RP2040 before connecting)
make monitor        # UART console via USB-serial adapter on GP0 (TX) / GP1 (RX)
```

A GitHub Actions workflow builds the UF2 on every push to `main` and uploads it as an artifact.

## Configuring profiles

The device exposes a USB CDC serial port ("Commander CDC") alongside the HID interface.
Profiles can be read and written at runtime using either the web configurator or the CLI tool.

### Web configurator (Chrome / Edge only)

Open the GitHub Pages URL for this repo, click **Connect**, and select the "Commander CDC" port.
Requires enabling GitHub Pages in repo Settings → Pages → source: `docs/` on `main`.

### Python CLI

```bash
pip install pyserial

python3 tools/commander.py ports              # list candidate ports
python3 tools/commander.py dump profiles.json # read profiles from device
python3 tools/commander.py load profiles.json # write + save to flash
```

Auto-detects the CDC port. Use `--port /dev/tty.usbmodemXXX` if detection fails.

### Manually editing profiles

Edit `src/profiles.c`. Each profile defines ENC2 CW/CCW, X, Y, A, B, C, D actions
using the `KEY(modifier, keycode, label)` or `SCR(delta, label)` macros.
Shortcuts default to **macOS** (Cmd = `_G`). Change `_G` → `_C` for Windows.

## Icon resources

Source icon images are in `resources/`. To replace the geometric placeholder icons
with real brand icons, convert them with the bundled script:

```bash
pip install Pillow cairosvg
make icons   # generates src/icons_data.h, auto-included by src/icons.c
```

See **CLAUDE.md** for the full architecture reference.
