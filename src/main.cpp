#include <Arduino.h>

/* Arduino pin numbers from the BBC micro:bit V2 variant. */
static const uint8_t rows[5] = {21, 22, 23, 24, 25};
static const uint8_t columns[5] = {4, 7, 3, 6, 10};
static const uint8_t pictures[3][5] = {
    {0b01010, 0b11111, 0b11111, 0b01110, 0b00100}, /* large heart */
    {0b00000, 0b01010, 0b01110, 0b00100, 0b00000}, /* small heart */
    {0b00000, 0b01010, 0b00000, 0b10001, 0b01110}  /* smile */
};

#define DEBOUNCE_MS 30UL
#define FRAME_MS 400UL

static const uint8_t button_pins[2] = {PIN_BUTTON_A, PIN_BUTTON_B};
static uint8_t button_previous[2] = {HIGH, HIGH};
static uint8_t button_stable[2] = {HIGH, HIGH};
static uint32_t button_changed_at[2] = {0, 0};
static uint8_t show_smile = 0;
static uint8_t paused = 0;
static uint8_t small_heart = 0;
static uint32_t last_frame = 0;
static uint32_t last_status = 0;

static uint8_t button_pressed(uint8_t button, uint32_t now) {
  uint8_t reading = (uint8_t)digitalRead(button_pins[button]);

  if (reading != button_previous[button]) {
    button_previous[button] = reading;
    button_changed_at[button] = now;
  }

  if (reading != button_stable[button] &&
      now - button_changed_at[button] >= DEBOUNCE_MS) {
    button_stable[button] = reading;
    return button_stable[button] == LOW;
  }

  return 0;
}

/* Refresh one LED matrix row every millisecond without blocking loop(). */
static void refresh_display(void) {
  static uint32_t last_scan = 0;
  static uint8_t row = 4;
  uint32_t now = micros();
  uint8_t picture;
  uint8_t column;

  if (now - last_scan < 1000UL) {
    return;
  }
  last_scan = now;

  digitalWrite(rows[row], LOW);
  row = (uint8_t)((row + 1) % 5);
  picture = show_smile ? 2 : (small_heart ? 1 : 0);

  for (column = 0; column < 5; column++) {
    uint8_t mask = (uint8_t)(1U << (4 - column));
    uint8_t lit = (uint8_t)(pictures[picture][row] & mask);
    digitalWrite(columns[column], lit ? LOW : HIGH);
  }

  digitalWrite(rows[row], HIGH);
}

void setup(void) {
  uint8_t i;

  for (i = 0; i < 5; i++) {
    digitalWrite(rows[i], LOW);
    pinMode(rows[i], OUTPUT);
    digitalWrite(columns[i], HIGH);
    pinMode(columns[i], OUTPUT);
  }

  pinMode(PIN_BUTTON_A, INPUT_PULLUP);
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("micro:bit V2 demo: A=heart/smile, B=pause/resume");
}

void loop(void) {
  uint32_t now;

  refresh_display();
  now = millis();

  if (button_pressed(0, now)) {
    show_smile = (uint8_t)!show_smile;
    Serial.println(show_smile ? "A: smile" : "A: heart");
  }

  if (button_pressed(1, now)) {
    paused = (uint8_t)!paused;
    last_frame = now;
    Serial.println(paused ? "B: paused" : "B: running");
  }

  if (!paused && now - last_frame >= FRAME_MS) {
    last_frame = now;
    small_heart = (uint8_t)!small_heart;
  }

  if (now - last_status >= 1000UL) {
    last_status = now;
    Serial.print("uptime(s)=");
    Serial.print(now / 1000UL);
    Serial.print(" picture=");
    Serial.print(show_smile ? "smile" : "heart");
    Serial.print(" animation=");
    Serial.println(paused ? "paused" : "running");
  }
}
