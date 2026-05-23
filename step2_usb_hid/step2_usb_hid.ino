// USB HID Commander
// Board: Waveshare RP2040-Zero  →  Raspberry Pi RP2040 (Philhower core)
// USB Stack: Adafruit TinyUSB
// Libraries: Adafruit GFX, Adafruit ST7789, HID-Project
//
// Each encoder delta → Consumer key (ENC1: volume, ENC2: configurable)
// Buttons A-D → keyboard shortcuts, X/Y → modifier + key

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <HID-Project.h>   // Consumer + Keyboard HID

// ── LCD ───────────────────────────────────────────────────────
#define LCD_SCK  2
#define LCD_MOSI 3
#define LCD_RST  4
#define LCD_BL   5
#define LCD_DC   6
#define LCD_CS   7

// ── Buttons ───────────────────────────────────────────────────
#define BTN_A  8
#define BTN_B  9
#define BTN_C  10
#define BTN_D  11
#define BTN_X  12
#define BTN_Y  13

// ── Encoders ─────────────────────────────────────────────────
#define ENC1_A 14
#define ENC1_B 15
#define ENC2_A 28
#define ENC2_B 29

// ── HID mappings — change these to suit your workflow ─────────
// Encoder CW / CCW consumer keys:
#define ENC1_CW  MEDIA_VOLUME_UP
#define ENC1_CCW MEDIA_VOLUME_DOWN
#define ENC2_CW  CONSUMER_FAST_FORWARD
#define ENC2_CCW CONSUMER_REWIND

// Button → keyboard key:
#define KEY_A   KEY_F13
#define KEY_B   KEY_F14
#define KEY_C   KEY_F15
#define KEY_D   KEY_F16
#define KEY_X   KEY_F17
#define KEY_Y   KEY_F18

// ─────────────────────────────────────────────────────────────

Adafruit_ST7789 tft(LCD_CS, LCD_DC, LCD_RST);

volatile int enc_pos[2] = {0, 0};
volatile int enc_last_a[2];

void isr_enc1() {
  int a = digitalRead(ENC1_A), b = digitalRead(ENC1_B);
  if (a != enc_last_a[0]) { enc_last_a[0] = a; enc_pos[0] += (a == b) ? 1 : -1; }
}
void isr_enc2() {
  int a = digitalRead(ENC2_A), b = digitalRead(ENC2_B);
  if (a != enc_last_a[1]) { enc_last_a[1] = a; enc_pos[1] += (a == b) ? 1 : -1; }
}

const int BTN_PINS[]    = {BTN_A, BTN_B, BTN_C, BTN_D, BTN_X, BTN_Y};
const uint8_t BTN_KEYS[]= {KEY_A, KEY_B, KEY_C, KEY_D, KEY_X, KEY_Y};
const char* BTN_LABELS[] = {"A", "B", "C", "D", "X", "Y"};
#define NUM_BTNS 6

void lcdStatus(int pos0, int pos1, bool* btns) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);

  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, 10);  tft.print("ENC1 "); tft.println(pos0);
  tft.setCursor(10, 36);  tft.print("ENC2 "); tft.println(pos1);

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 80);
  tft.print("BTN: ");
  for (int i = 0; i < NUM_BTNS; i++) {
    if (btns[i]) { tft.setTextColor(ST77XX_YELLOW); }
    tft.print(BTN_LABELS[i]);
    tft.setTextColor(ST77XX_WHITE);
    tft.print(" ");
  }

  tft.setCursor(10, 210);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("USB HID");
}

void setup() {
  // LCD
  SPI.setSCK(LCD_SCK);
  SPI.setTX(LCD_MOSI);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  tft.init(240, 240);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  tft.println("Commander");
  tft.setCursor(10, 126);
  tft.println("Starting...");

  // Encoders
  pinMode(ENC1_A, INPUT_PULLUP); pinMode(ENC1_B, INPUT_PULLUP);
  pinMode(ENC2_A, INPUT_PULLUP); pinMode(ENC2_B, INPUT_PULLUP);
  enc_last_a[0] = digitalRead(ENC1_A);
  enc_last_a[1] = digitalRead(ENC2_A);
  attachInterrupt(digitalPinToInterrupt(ENC1_A), isr_enc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), isr_enc2, CHANGE);

  // Buttons
  for (int i = 0; i < NUM_BTNS; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);

  // USB HID
  Consumer.begin();
  Keyboard.begin();

  delay(500);
  bool no_btns[NUM_BTNS] = {};
  lcdStatus(0, 0, no_btns);
}

static int last_pos[2] = {};
static bool last_btn[NUM_BTNS] = {};

void loop() {
  bool redraw = false;

  // Encoder deltas → consumer keys
  for (int enc = 0; enc < 2; enc++) {
    int delta = enc_pos[enc] - last_pos[enc];
    if (delta == 0) continue;
    last_pos[enc] = enc_pos[enc];
    redraw = true;

    ConsumerKeycode cw  = (enc == 0) ? ENC1_CW  : ENC2_CW;
    ConsumerKeycode ccw = (enc == 0) ? ENC1_CCW : ENC2_CCW;
    int steps = abs(delta);
    ConsumerKeycode key = (delta > 0) ? cw : ccw;
    for (int i = 0; i < steps; i++) {
      Consumer.write(key);
      delay(10);
    }
  }

  // Buttons → keyboard keys (send on press, release on release)
  for (int i = 0; i < NUM_BTNS; i++) {
    bool pressed = (digitalRead(BTN_PINS[i]) == LOW);
    if (pressed != last_btn[i]) {
      last_btn[i] = pressed;
      redraw = true;
      if (pressed) Keyboard.press(BTN_KEYS[i]);
      else         Keyboard.release(BTN_KEYS[i]);
    }
  }

  if (redraw) lcdStatus(last_pos[0], last_pos[1], last_btn);

  delay(5);
}
