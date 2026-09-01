// Morse SOS (... --- ...) on P9 (D-K relay LED). LSC Smart Connect 3202087.2.
// P11 (Wi-Fi status LED) and P24 (relay) are intentionally unused.

#include <Arduino.h>

// P9 = relay LED per docs/3202087-lsc-plug/0-feasibility.md (LibreTiny GPIO 9).
static const uint8_t LED_PIN = 9;

// Active level [to be verified on hardware]. Flip if P9 stays dark after flash.
#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW 1
#endif

static const uint16_t MORSE_UNIT_MS = 120;

static void ledOn() {
#if LED_ACTIVE_LOW
  digitalWrite(LED_PIN, LOW);
#else
  digitalWrite(LED_PIN, HIGH);
#endif
}

static void ledOff() {
#if LED_ACTIVE_LOW
  digitalWrite(LED_PIN, HIGH);
#else
  digitalWrite(LED_PIN, LOW);
#endif
}

static void morseGap(uint16_t units) {
  ledOff();
  delay(static_cast<unsigned long>(MORSE_UNIT_MS) * units);
}

static void morseDit() {
  ledOn();
  delay(MORSE_UNIT_MS);
  morseGap(1);
}

static void morseDah() {
  ledOn();
  delay(static_cast<unsigned long>(MORSE_UNIT_MS) * 3);
  morseGap(1);
}

static void morseLetterGap() { morseGap(3); }

static void morseS() {
  morseDit();
  morseDit();
  morseDit();
}

static void morseO() {
  morseDah();
  morseDah();
  morseDah();
}

static void morseSOS() {
  morseS();
  morseLetterGap();
  morseO();
  morseLetterGap();
  morseS();
  morseGap(7);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  ledOff();
}

void loop() { morseSOS(); }
