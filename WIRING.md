# WIRING.md — RP2040-Zero USB HID Commander

## RP2040-Zero Pin Assignment

| GPIO | Function | Note |
|------|----------|------|
| GP0  | PIO USB D− | optional second USB port |
| GP1  | PIO USB D+ | optional second USB port |
| GP2  | LCD SCK  | SPI0 |
| GP3  | LCD MOSI | SPI0 |
| GP4  | LCD RST  | |
| GP5  | LCD BL   | HIGH = backlight on |
| GP6  | LCD DC   | |
| GP7  | LCD CS   | |
| GP8  | BTN A    | INPUT_PULLUP, active-low |
| GP9  | BTN B    | INPUT_PULLUP, active-low |
| GP10 | BTN C    | INPUT_PULLUP, active-low |
| GP11 | BTN D    | INPUT_PULLUP, active-low |
| GP12 | BTN X    | INPUT_PULLUP, active-low |
| GP13 | BTN Y    | INPUT_PULLUP, active-low |
| GP14 | ENC1 A   | INPUT_PULLUP |
| GP15 | ENC1 B   | INPUT_PULLUP |
| GP16 | WS2812   | onboard LED — do not use |
| GP26 | Grove SDA | optional I2C1 (Wire1) |
| GP27 | Grove SCL | optional I2C1 (Wire1) |
| GP28 | ENC2 A   | INPUT_PULLUP |
| GP29 | ENC2 B   | INPUT_PULLUP |

## ST7789 LCD Wiring

| LCD Pin  | RP2040-Zero | Note |
|----------|-------------|------|
| VCC      | 3V3         | |
| GND      | GND         | |
| SCL/SCK  | GP2         | SPI0 SCK |
| SDA/MOSI | GP3         | SPI0 TX |
| RES      | GP4         | Reset |
| BL       | GP5         | Backlight enable |
| DC       | GP6         | Data/Command |
| CS       | GP7         | Chip Select |

## Rotary Encoder Wiring (per encoder)

```
        ┌─────────┐
   A ───┤  EC11   ├─── GND
   B ───┤         │
        └─────────┘
```

- A, B connect to GPIO with `INPUT_PULLUP`; other side to GND
- Add 100 nF caps across A–GND and B–GND to reduce noise

## Button Wiring

One side to GPIO (`INPUT_PULLUP`), other side to GND.
