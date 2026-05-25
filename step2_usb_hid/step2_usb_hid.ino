// USB HID Commander
// Board: Waveshare RP2040-Zero  →  Raspberry Pi RP2040 (Philhower core)
// USB Stack: default (Pico SDK) — uses built-in <Keyboard.h> and <Mouse.h>
// No extra HID library needed.
//
// ENC1 → Mouse scroll wheel
// ENC2 → keyboard shortcuts (left/right arrow; change to taste)
// Buttons A-D, X, Y → keyboard keys

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Keyboard.h>
#include <Mouse.h>

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

// ── HID mappings — change to suit your workflow ───────────────
// ENC1: scroll wheel — positive = scroll up, negative = scroll down
// ENC2: keyboard keys sent per step
#define ENC2_CW_KEY  KEY_RIGHT_ARROW
#define ENC2_CCW_KEY KEY_LEFT_ARROW

// Button → keyboard key
#define KEY_BTN_A KEY_F13
#define KEY_BTN_B KEY_F14
#define KEY_BTN_C KEY_F15
#define KEY_BTN_D KEY_F16
#define KEY_BTN_X KEY_F17
#define KEY_BTN_Y KEY_F18

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

const int BTN_PINS[]       = {BTN_A,     BTN_B,     BTN_C,     BTN_D,     BTN_X,     BTN_Y};
const uint8_t BTN_KEYS[]   = {KEY_BTN_A, KEY_BTN_B, KEY_BTN_C, KEY_BTN_D, KEY_BTN_X, KEY_BTN_Y};
const char*   BTN_LABELS[] = {"A",       "B",       "C",       "D",       "X",       "Y"};
#define NUM_BTNS 6

void lcdStatus(int scroll_total, int enc2_pos, bool* btns) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);

  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, 10);  tft.print("SCR "); tft.println(scroll_total);
  tft.setCursor(10, 36);  tft.print("ENC2 "); tft.println(enc2_pos);

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 80);
  tft.print("BTN: ");
  for (int i = 0; i < NUM_BTNS; i++) {
    if (btns[i]) tft.setTextColor(ST77XX_YELLOW);
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
  Keyboard.begin();
  Mouse.begin();

  delay(500);
  bool no_btns[NUM_BTNS] = {};
  lcdStatus(0, 0, no_btns);
}

static int last_pos[2] = {};
static bool last_btn[NUM_BTNS] = {};
static int scroll_total = 0;

void loop() {
  bool redraw = false;

  // ENC1 → scroll wheel
  int delta0 = enc_pos[0] - last_pos[0];
  if (delta0 != 0) {
    last_pos[0] = enc_pos[0];
    scroll_total += delta0;
    Mouse.move(0, 0, delta0);
    redraw = true;
  }

  // ENC2 → keyboard arrow keys (one keypress per step)
  int delta1 = enc_pos[1] - last_pos[1];
  if (delta1 != 0) {
    last_pos[1] = enc_pos[1];
    int steps = abs(delta1);
    uint8_t key = (delta1 > 0) ? ENC2_CW_KEY : ENC2_CCW_KEY;
    for (int i = 0; i < steps; i++) {
      Keyboard.press(key);
      delay(5);
      Keyboard.release(key);
      delay(5);
    }
    redraw = true;
  }

  // Buttons → keyboard keys
  for (int i = 0; i < NUM_BTNS; i++) {
    bool pressed = (digitalRead(BTN_PINS[i]) == LOW);
    if (pressed != last_btn[i]) {
      last_btn[i] = pressed;
      if (pressed) Keyboard.press(BTN_KEYS[i]);
      else         Keyboard.release(BTN_KEYS[i]);
      redraw = true;
    }
  }

  if (redraw) lcdStatus(scroll_total, last_pos[1], last_btn);

  delay(5);
}
