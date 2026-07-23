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
#define PIN_B   47   // was 8  — 8 not broken out on this board; moved to 47
#define PIN_C    9
#define PIN_D   10
#define PIN_E   14   // unused on this 32-high (1/16-scan) panel — not wired
// Control
#define PIN_LAT 45   // was 17 — 17 not broken out; moved to 45 (strapping pin,
                     // but its default boot level is safe and LAT idles low)
#define PIN_OE  21
#define PIN_CLK 48   // was 13 — 13 not broken out; moved to 48

// ── 28BYJ-48 stepper via ULN2003 (record spin) ───────────────────────────────
// Remapped off the original 11/12/38/39 because those holes had leftover solder
// stuck in them. 1/2/41/42 are fresh, free, output-capable, and don't clash with
// the HUB75 panel. (41 was an optional-mic pin; the mic isn't fitted, so it's free.)
//
// Order matters: IN1..IN4 must fire the driver's coils in physical A→B→C→D order
// or the motor vibrates instead of turning. On this build the wires landed such
// that A=GPIO1, B=GPIO41, C=GPIO2, D=GPIO42 (verified with the coil-check test),
// so the pins are listed in that firing order rather than by GPIO number.
#define PIN_MOTOR_IN1 1
#define PIN_MOTOR_IN2 41
#define PIN_MOTOR_IN3 2
#define PIN_MOTOR_IN4 42

// ── WS2812B halo ring ────────────────────────────────────────────────────────
#define PIN_LED_DIN   40

// ── (Optional) INMP441 I²S microphone — audio-reactive halo ──────────────────
// Leave HALO_USE_MIC = 0 in config.h if you didn't fit the mic.
#define PIN_I2S_SCK   41
#define PIN_I2S_WS    47
#define PIN_I2S_SD    48
