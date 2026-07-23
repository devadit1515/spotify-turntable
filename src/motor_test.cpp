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

// 28BYJ-48 half-step sequence (8 phases). Half-stepping = smoother + more torque
// than full-step. Each row is which of the 4 coils are energised.
static const uint8_t SEQ[8][4] = {
  {1,0,0,0},
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1},
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

void loop() {
  static int phase = 0;
  for (int i = 0; i < 4; i++) {
    digitalWrite(IN[i], SEQ[phase][i] ? HIGH : LOW);
  }
  phase = (phase + 1) & 7;      // next of 8 phases
  delay(2);                     // 2 ms/half-step ≈ smooth slow spin
}
