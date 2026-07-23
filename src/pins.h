// ─────────────────────────────────────────────────────────────────────────────
//  pins.h — GPIO map for the Spotify Turntable (ESP32-S3-N8R2)
//
//  Why these pins: on an S3 module with PSRAM you must AVOID GPIO 26–37 (used by
//  the flash/PSRAM SPI bus), GPIO 0/3/45/46 (strapping), and 19/20 (native USB).
//  Everything below lives in the safe set. If you wire differently, this is the
//  ONLY file you need to change — nothing else references raw pin numbers.
// ─────────────────────────────────────────────────────────────────────────────
#pragma once

// ── HUB75 64×64 RGB matrix (14 lines for a 1/32-scan panel) ──────────────────
// Upper-half RGB
#define PIN_R1   4
#define PIN_G1   5
#define PIN_B1   6
// Lower-half RGB
#define PIN_R2   7
#define PIN_G2  15
#define PIN_B2  16
// Row-address lines (A..E — E is required on a 64-high panel)
#define PIN_A   18
#define PIN_B    8
#define PIN_C    9
#define PIN_D   10
#define PIN_E   14
// Control
#define PIN_LAT 17
#define PIN_OE  21
#define PIN_CLK 13

// ── 28BYJ-48 stepper via ULN2003 (record spin) ───────────────────────────────
// Remapped off the original 11/12/38/39 because those holes had leftover solder
// stuck in them. 1/2/41/42 are fresh, free, output-capable, and don't clash with
// the HUB75 panel. (41 was an optional-mic pin; the mic isn't fitted, so it's free.)
#define PIN_MOTOR_IN1 1
#define PIN_MOTOR_IN2 2
#define PIN_MOTOR_IN3 42
#define PIN_MOTOR_IN4 41

// ── WS2812B halo ring ────────────────────────────────────────────────────────
#define PIN_LED_DIN   40

// ── (Optional) INMP441 I²S microphone — audio-reactive halo ──────────────────
// Leave HALO_USE_MIC = 0 in config.h if you didn't fit the mic.
#define PIN_I2S_SCK   41
#define PIN_I2S_WS    47
#define PIN_I2S_SD    48
