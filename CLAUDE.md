# CLAUDE.md — rp-commander (USB HID Commander)

## Quick commands

```bash
make build          # build firmware inside Docker
make deploy         # copy commander.uf2 to RP2040 in BOOTSEL mode
make quick          # build + deploy
make monitor        # screen to UART0 (requires USB-serial adapter on GP0/GP1)
make docker-build   # one-time Docker image creation
make icons          # convert resources/ → src/icons_data.h (pip3 install Pillow cairosvg)
```

## Hardware

| Component | Part | Interface |
|-----------|------|-----------|
| MCU | Waveshare RP2040-Zero | USB-C (native USB HID + CDC) |
| Display | ST7789 240×240 | SPI0 (GP2/3/4/5/6/7) |
| ENC1 | EC11 rotary encoder | GPIO IRQ GP14/15 — **profile nav only** |
| ENC2 | EC11 rotary encoder | GPIO IRQ GP28/29 — per-profile HID |
| Buttons | 6 × tact switch (A B C D X Y) | GPIO pull-up GP8-13 |

## Available GPIO (RP2040-Zero)

Only GP0–GP15 and GP26–GP29 are accessible. GP16 = onboard WS2812.

| GPIO | Function |
|------|----------|
| GP0  | UART0 TX (serial debug) |
| GP1  | UART0 RX |
| GP2  | LCD SCK (SPI0) |
| GP3  | LCD MOSI (SPI0) |
| GP4  | LCD RST |
| GP5  | LCD BL (PWM) |
| GP6  | LCD DC |
| GP7  | LCD CS |
| GP8  | BTN A |
| GP9  | BTN B |
| GP10 | BTN C |
| GP11 | BTN D |
| GP12 | BTN X (left-side, ENC1 companion) |
| GP13 | BTN Y (right-side, ENC2 companion) |
| GP14 | ENC1 A |
| GP15 | ENC1 B |
| GP16 | Onboard WS2812 (reserved) |
| GP26 | Optional Grove SDA (Wire1 / I2C1) |
| GP27 | Optional Grove SCL |
| GP28 | ENC2 A |
| GP29 | ENC2 B |

## Source layout

```
src/
├── config.h           — pin assignments, timing constants
├── main.c             — init + event loop
├── lcd.h / lcd.c      — ST7789 SPI0 driver with DMA (240×240, 0° rotation)
├── font.h / font.c    — 8×8 bitmap font; font_draw_char() for icon use
├── encoder.h / .c     — quadrature encoder with GPIO IRQ
├── buttons.h / .c     — 6-button debounced scanner
├── usb_hid.h / .c     — TinyUSB keyboard + mouse report queue
├── usb_descriptors.c  — TinyUSB device/config/HID+CDC descriptors (IAD)
├── tusb_config.h      — TinyUSB build config (CFG_TUD_HID=1, CFG_TUD_CDC=1)
├── profiles.h / .c    — app + profile table, JSON serialization/deserialization
├── icons.h / icons.c  — per-app icon draw functions (geometric + optional blit)
├── ui.h / ui.c        — 3-row LCD layout renderer, Claude mascot animation
├── config_store.h / .c — flash R/W with CRC32 (last sector of 2 MB flash)
└── cdc_config.h / .c  — CDC config protocol (HELLO/GET/SET/SAVE/RESET)
```

## UI layout (240×240)

```
y=  0 ┌────────────────────────────────────────┐
      │  App strip: 5 icons × 48 px           │ h=48
y= 48 ├────────────────────────────────────────┤
      │  Large icon │ App name                 │ h=80
      │  (animated  │ Profile name (×2)        │
      │   for Claude)│ ● ○ ○  (dots per profile)│
y=128 ├──────────────────┬─────────────────────┤
      │ ← Profile →      │ ENC2: <label>       │ h=52
      │ X: <action>      │ Y: <action>         │
y=180 ├──────┬──────┬────┴┬─────┐              │
      │  A   │  B   │  C  │  D  │              │ h=60
      │<act> │<act> │<act>│<act>│              │
y=240 └──────┴──────┴─────┴─────┘
```

## UX / Navigation

| Input | Action |
|-------|--------|
| ENC1 CW | Next profile (wraps across apps) |
| ENC1 CCW | Previous profile |
| ENC2 CW/CCW | Per-profile HID action (label shown in enc row) |
| BTN X | Per-profile HID action (shown left in enc row) |
| BTN Y | Per-profile HID action (shown right in enc row) |
| BTN A/B/C/D | Per-profile HID actions (bottom row) |

Profile cycle order: Chrome/Browsing → Claude/Chat → Fusion/Sketch →
Fusion/Modeling → KiCad/Schematics → KiCad/PCB → Krita/Painting → (wrap)

LCD backlight turns off after 30 s of inactivity. Any input wakes it;
the triggering input is discarded so no accidental action fires on wake.

## LCD notes

- ST7789 240×240 on SPI0 at 40 MHz
- `MADCTL = 0x00` (0° rotation)
- Row address offset = 0
- RGB565 values in framebuffer are byte-swapped for DMA/SPI big-endian output:
  use `RGB565(r,g,b)` macro (applies `__builtin_bswap16`) or predefined `COL_xxx`
- `lcd_flush_full()` uses DMA; `lcd_flush_region()` is blocking row-by-row SPI

## USB

- Composite device: HID (keyboard + mouse) + CDC (config serial)
- No mass storage, MIDI, or vendor interface
- Serial debug via UART0 (GP0=TX, GP1=RX, 115200 baud) — not USB
- HID report IDs: 1 = keyboard, 2 = mouse (with scroll wheel)
- CDC interface string: "Commander CDC"; used by both the web configurator and Python CLI
- `usb_hid_task()` and `cdc_config_task()` must both be called every loop iteration

## CDC config protocol

Line-based, UTF-8. Commands sent by host, responses from device.

| Command | Response |
|---------|----------|
| `HELLO` | `rp-commander 1.0\nOK` |
| `GET` | `<compact JSON>\nOK` |
| `SET <len>\n<body>` | `OK` or `ERR …\nFAIL` |
| `SAVE` | `OK` (writes current profiles to flash) |
| `RESET` | `OK` then watchdog reboot |

JSON schema: `{"v":1,"profiles":[{…}, …]}`

Each profile object: `{"app":N,"name":"…","el":"…","ew":{action},"eccw":{action},"x":{action},…}`

Action: `{"t":0}` (none) / `{"t":1,"m":N,"k":N,"l":"…"}` (key) / `{"t":2,"d":N,"l":"…"}` (scroll)

## Flash config store

- Location: last sector of flash (`PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE`, i.e. offset 0x1FF000 on 2 MB)
- Header: magic `0x52504346` ("RPCF") + CRC32 + length (12 bytes total)
- Interrupts disabled during erase+program
- Loaded at startup; if missing or corrupt, compiled-in defaults are used

## Build

Board: `pico` (generic RP2040). If the Pico SDK has `waveshare_rp2040_zero`,
set `PICO_BOARD=waveshare_rp2040_zero` in CMakeLists.txt.

Pico SDK path: `../pico-sdk` (relative to project root).

GitHub Actions builds the UF2 on every push to `main` (see `.github/workflows/build.yml`).

## Configuring profiles

### Python CLI

```bash
pip install pyserial
python3 tools/commander.py ports              # list candidate ports
python3 tools/commander.py dump profiles.json # read from device
python3 tools/commander.py load profiles.json # write + save to flash
python3 tools/commander.py --port /dev/tty.usbmodemXXX dump
```

### Web configurator

`docs/index.html` — deployed to GitHub Pages. Requires Chrome or Edge (WebSerial API).
Enable Pages in repo Settings → Pages → source: `docs/` on `main`.

## Adding a profile

1. In `src/profiles.c`, add a new `profile_t` entry to `g_profiles[]`.
   Set `app_idx` to an existing `ICON_xxx` or add a new app in `g_apps[]`.
2. Define `enc2_cw/ccw`, `enc2_label`, `btn_x/y/a/b/c/d` using the
   `KEY(mod, key, label)` or `SCR(delta, label)` macros.
3. Rebuild — profile count (`g_num_profiles`) and ENC1 wrap update automatically.

## Shortcut platform

Default platform: **macOS** (`_G` = KEYBOARD_MODIFIER_LEFTGUI = Command).
For Windows, change `_G` to `_C` (LEFTCTRL) in `profiles.c`.

## Docs

| File | Contents |
|------|----------|
| `docs/WIRING.md` | Physical wiring table (LCD, encoders, buttons) |
| `docs/index.html` | WebSerial profile configurator |

## Resources

Source icon images live in `resources/`. They are not used directly by the firmware —
they must be converted to 32×32 RGB565 C arrays and embedded in `src/icons_data.h`.

| File | App |
|------|-----|
| `Google_Chrome_icon_(February_2022).svg` | Chrome |
| `Claude_icon.png` | Claude |
| `fusion-360-product-design-extension-2023-simplified-badge-75x75.png.webp` | Fusion 360 |
| `kicad.png` | KiCad |
| `Calligra_Krita_icon.svg.png` | Krita |

Conversion pipeline (Python + Pillow):
```bash
make icons                      # converts all resources/ → src/icons_data.h
# or directly:
python3 tools/make_icons.py [--size N]   # default 32×32
```

`src/icons_data.h` is auto-included by `src/icons.c` via `__has_include`.
Each successfully converted icon replaces the corresponding geometric placeholder;
missing or failed icons fall back to the geometric shape automatically.

## Arduino sketches (kept for hardware reference)

`step1_hw_test/` — LCD + encoder + button bringup (Arduino / Philhower core)
`step2_usb_hid/` — basic USB HID with Keyboard.h + Mouse.h (Arduino)
