#include <Arduino.h>

// Arduino pin numbers from the BBCmicrobitV2 variant (not nRF GPIO numbers).
constexpr uint8_t kRows[] = {21, 22, 23, 24, 25};
constexpr uint8_t kColumns[] = {4, 7, 3, 6, 10};
constexpr uint32_t kDebounceMs = 30;
constexpr uint32_t kFrameMs = 400;

// Each byte is one row; bit 4 is the leftmost pixel.
constexpr uint8_t kPictures[][5] = {
    {0b01010, 0b11111, 0b11111, 0b01110, 0b00100}, // large heart
    {0b00000, 0b01010, 0b01110, 0b00100, 0b00000}, // small heart
    {0b00000, 0b01010, 0b00000, 0b10001, 0b01110}, // smile
};

struct Button {
  explicit Button(uint8_t inputPin) : pin(inputPin) {}

  uint8_t pin;
  bool previous = HIGH;
  bool stable = HIGH;
  uint32_t changedAt = 0;

  bool pressed(uint32_t now) {
    const bool reading = digitalRead(pin);
    if (reading != previous) {
      previous = reading;
      changedAt = now;
    }
    if (reading != stable && now - changedAt >= kDebounceMs) {
      stable = reading;
      return stable == LOW;
    }
    return false;
  }
};

Button buttonA{PIN_BUTTON_A};
Button buttonB{PIN_BUTTON_B};
bool smile = false;
bool paused = false;
bool smallHeart = false;
uint32_t lastFrame = 0;
uint32_t lastStatus = 0;

// Refresh one row every millisecond. Disable the old row before changing
// columns to avoid ghost pixels. Keep loop() free of blocking delays.
void refreshDisplay() {
  static uint32_t lastScan = 0;
  static uint8_t row = 4;
  const uint32_t now = micros();
  if (now - lastScan < 1000) {
    return;
  }
  lastScan = now;
  digitalWrite(kRows[row], LOW);
  row = (row + 1) % 5;
  const uint8_t picture = smile ? 2 : (smallHeart ? 1 : 0);
  for (uint8_t column = 0; column < 5; ++column) {
    const bool lit = kPictures[picture][row] & (1u << (4 - column));
    digitalWrite(kColumns[column], lit ? LOW : HIGH);
  }
  digitalWrite(kRows[row], HIGH);
}

void setup() {
  for (uint8_t pin : kRows) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
  }
  for (uint8_t pin : kColumns) {
    digitalWrite(pin, HIGH);
    pinMode(pin, OUTPUT);
  }
  pinMode(PIN_BUTTON_A, INPUT_PULLUP);
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("micro:bit V2 Arduino demo: A=heart/smile, B=pause/resume");
}

void loop() {
  refreshDisplay();
  const uint32_t now = millis();
  if (buttonA.pressed(now)) {
    smile = !smile;
    Serial.println(smile ? "A: smile" : "A: heart");
  }
  if (buttonB.pressed(now)) {
    paused = !paused;
    lastFrame = now;
    Serial.println(paused ? "B: paused" : "B: running");
  }
  if (!paused && now - lastFrame >= kFrameMs) {
    lastFrame = now;
    smallHeart = !smallHeart;
  }
  if (now - lastStatus >= 1000) {
    lastStatus = now;
    Serial.print("uptime(s)=");
    Serial.print(now / 1000);
    Serial.print(" picture=");
    Serial.print(smile ? "smile" : "heart");
    Serial.print(" animation=");
    Serial.println(paused ? "paused" : "running");
  }
}
