// Hardware bringup test: ST7789 LCD + rotary encoders + buttons + WS2812 LED
// Board: Waveshare RP2040-Zero  →  Raspberry Pi RP2040 (Philhower core)
// USB Stack: Adafruit TinyUSB (or default — HID not needed here)

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>

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

// ── Onboard WS2812 ───────────────────────────────────────────
#define LED_PIN 16

// ─────────────────────────────────────────────────────────────

Adafruit_ST7789 tft(LCD_CS, LCD_DC, LCD_RST);
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// Color wheel: cycle through these on every input change
static const uint32_t COLORS[] = {
  0xFF0000, // red
  0xFF8000, // orange
  0xFFFF00, // yellow
  0x00FF00, // green
  0x00FFFF, // cyan
  0x0000FF, // blue
  0xFF00FF, // magenta
  0xFFFFFF, // white
};
#define NUM_COLORS (sizeof(COLORS) / sizeof(COLORS[0]))
static uint8_t color_idx = 0;

volatile int enc_pos[2] = {0, 0};
volatile int enc_last_a[2];
volatile int isr2_count = 0;  // increments on every ENC2 interrupt — 0 means ISR never fired

void isr_enc1() {
  int a = digitalRead(ENC1_A);
  int b = digitalRead(ENC1_B);
  if (a != enc_last_a[0]) { enc_last_a[0] = a; enc_pos[0] += (a == b) ? 1 : -1; }
}
void isr_enc2() {
  isr2_count++;
  int a = digitalRead(ENC2_A);
  int b = digitalRead(ENC2_B);
  if (a != enc_last_a[1]) { enc_last_a[1] = a; enc_pos[1] += (a == b) ? 1 : -1; }
}

const int BTN_PINS[] = {BTN_A, BTN_B, BTN_C, BTN_D, BTN_X, BTN_Y};
const char* BTN_NAMES[] = {"A", "B", "C", "D", "X", "Y"};
#define NUM_BTNS 6

void setup() {
  Serial.begin(115200);

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
  tft.setCursor(10, 10);
  tft.println("HW Test");
  tft.setCursor(10, 36);
  tft.println("Turn encoders");
  tft.setCursor(10, 62);
  tft.println("Press buttons");

  // Encoders
  pinMode(ENC1_A, INPUT_PULLUP); pinMode(ENC1_B, INPUT_PULLUP);
  pinMode(ENC2_A, INPUT_PULLUP); pinMode(ENC2_B, INPUT_PULLUP);
  enc_last_a[0] = digitalRead(ENC1_A);
  enc_last_a[1] = digitalRead(ENC2_A);
  attachInterrupt(digitalPinToInterrupt(ENC1_A), isr_enc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), isr_enc2, CHANGE);

  // Buttons
  for (int i = 0; i < NUM_BTNS; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);

  // WS2812
  led.begin();
  led.setBrightness(40);
  led.setPixelColor(0, COLORS[color_idx]);
  led.show();
}

static int last_pos[2] = {};
static bool last_btn[NUM_BTNS] = {};
static unsigned long last_debug_ms = 0;

void loop() {
  bool redraw = false;

  // Print raw GP28/GP29 state + ISR fire count every 500 ms for diagnostics.
  // If GP28/29 don't toggle when you manually short them to GND → wiring issue.
  // If isr2_count never changes while turning ENC2 → interrupt not firing.
  // If isr2_count changes but enc_pos[1] doesn't → direction logic issue.
  if (millis() - last_debug_ms >= 500) {
    last_debug_ms = millis();
    Serial.print("DBG  GP28="); Serial.print(digitalRead(ENC2_A));
    Serial.print(" GP29="); Serial.print(digitalRead(ENC2_B));
    Serial.print("  isr2_count="); Serial.println(isr2_count);
  }

  for (int i = 0; i < 2; i++) {
    if (enc_pos[i] != last_pos[i]) { last_pos[i] = enc_pos[i]; redraw = true; }
  }
  for (int i = 0; i < NUM_BTNS; i++) {
    bool pressed = (digitalRead(BTN_PINS[i]) == LOW);
    if (pressed != last_btn[i]) { last_btn[i] = pressed; redraw = true; }
  }

  if (redraw) {
    color_idx = (color_idx + 1) % NUM_COLORS;
    led.setPixelColor(0, COLORS[color_idx]);
    led.show();

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);

    // Encoders
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, 10);
    tft.print("ENC1: "); tft.println(enc_pos[0]);
    tft.setCursor(10, 36);
    tft.print("ENC2: "); tft.println(enc_pos[1]);

    // Buttons
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 80);
    tft.print("Btn: ");
    for (int i = 0; i < NUM_BTNS; i++) {
      if (last_btn[i]) {
        tft.setTextColor(ST77XX_YELLOW);
        tft.print(BTN_NAMES[i]);
        tft.setTextColor(ST77XX_WHITE);
      } else {
        tft.print("_");
      }
      tft.print(" ");
    }

    // Serial mirror
    Serial.print("ENC1="); Serial.print(enc_pos[0]);
    Serial.print(" ENC2="); Serial.print(enc_pos[1]);
    Serial.print(" BTN:");
    for (int i = 0; i < NUM_BTNS; i++) {
      Serial.print(last_btn[i] ? BTN_NAMES[i] : "_");
    }
    Serial.println();
  }

  delay(10);
}
