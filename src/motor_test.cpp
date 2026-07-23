// ─────────────────────────────────────────────────────────────────────────────
//  motor_test.cpp — DUMB motor test. No WiFi, no Spotify, no panel.
//
//  Purpose: prove the 28BYJ-48 + ULN2003 wiring works in isolation. It just
//  half-steps the motor forever at a slow, smooth speed. Success = the driver
//  board's 4 blue LEDs chase in sequence AND the shaft slowly rotates.
//
//  Compiled ONLY by [env:motortest] (see platformio.ini). The real app and the
//  panel test are excluded, so there's no clash between their setup()/loop().
// ─────────────────────────────────────────────────────────────────────────────
#include <Arduino.h>
#include "pins.h"

// The four ULN2003 inputs, in order. If you remap in pins.h, this follows.
static const int IN[4] = { PIN_MOTOR_IN1, PIN_MOTOR_IN2, PIN_MOTOR_IN3, PIN_MOTOR_IN4 };

// 28BYJ-48 FULL-step sequence (4 phases). Two coils energised every step =
// ~2× the torque of half-step. We use full-step here so the motor can turn even
// on a weak 3.3V supply (the ESP32's 5V pin is dead on this board).
static const uint8_t SEQ[4][4] = {
  {1,1,0,0},
  {0,1,1,0},
  {0,0,1,1},
  {1,0,0,1},
};

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\n=== MOTOR TEST (no WiFi) ==="));
  for (int i = 0; i < 4; i++) {
    pinMode(IN[i], OUTPUT);
    digitalWrite(IN[i], LOW);
  }
  Serial.printf("[test] driving IN1..IN4 on GPIO %d %d %d %d\n",
                IN[0], IN[1], IN[2], IN[3]);
  Serial.println(F("[test] shaft should turn; driver LEDs should chase."));
}

// ── COIL-CHECK diagnostic ────────────────────────────────────────────────────
// Light each of the 4 coils ONE AT A TIME for 0.8 s, in a loop. Watch the driver
// board's 4 blue LEDs: all four should light in turn (1→2→3→4→1…). If one LED
// NEVER lights, that coil's wire / solder joint is the dead connection.
void loop() {
  static int which = 0;
  const char *names[4] = { "IN1 (LED 1)", "IN2 (LED 2)", "IN3 (LED 3)", "IN4 (LED 4)" };

  for (int i = 0; i < 4; i++) digitalWrite(IN[i], LOW);   // all off
  digitalWrite(IN[which], HIGH);                          // just this one on
  Serial.printf("[coil-check] now ON: %s  (GPIO %d)\n", names[which], IN[which]);

  which = (which + 1) & 3;
  delay(800);
}
