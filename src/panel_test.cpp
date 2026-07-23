// ─────────────────────────────────────────────────────────────────────────────
//  panel_test.cpp — DUMB panel test. No WiFi, no Spotify, no tasks.
//
//  Purpose: prove the 32×32 HUB75 panel + wiring + power actually work, in
//  isolation, before we trust the full app. Cycles solid colours and draws a
//  moving line so you can spot dead rows/columns or wrong colour order.
//
//  This file is compiled ONLY by the [env:paneltest] PlatformIO environment
//  (see platformio.ini build_src_filter). The normal app excludes it, so the
//  two setup()/loop() pairs never collide.
// ─────────────────────────────────────────────────────────────────────────────
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "pins.h"

static const int W = 32;
static const int H = 32;

static MatrixPanel_I2S_DMA *dma = nullptr;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\n=== PANEL TEST (no WiFi) ==="));

  HUB75_I2S_CFG::i2s_pins pinmap = {
    PIN_R1, PIN_G1, PIN_B1, PIN_R2, PIN_G2, PIN_B2,
    PIN_A,  PIN_B,  PIN_C,  PIN_D,  PIN_E,
    PIN_LAT, PIN_OE, PIN_CLK
  };
  HUB75_I2S_CFG mxconfig(W, H, 1 /* chain */, pinmap);
  mxconfig.double_buff = true;
  mxconfig.clkphase    = false;
  // This QIANGLI panel stays completely dark without the FM6126A wake-up
  // sequence — enabling it here. (If it now shows garbage instead of black,
  // that tells us the chip is actually a plain shift-register type; we'd remove
  // this. But "totally black" is the classic FM6126A-not-initialised symptom.)
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  dma = new MatrixPanel_I2S_DMA(mxconfig);
  dma->begin();
  dma->setBrightness8(45);   // low: safe on USB power for the first light
  dma->clearScreen();
  Serial.println(F("[test] panel initialised — you should see colours now"));
}

static void fillAndHold(uint8_t r, uint8_t g, uint8_t b, const char *name) {
  Serial.printf("[test] %s\n", name);
  dma->fillScreenRGB888(r, g, b);
  dma->flipDMABuffer();
  delay(1200);
}

void loop() {
  // 1) Solid colours — confirms every pixel + correct R/G/B order.
  fillAndHold(255, 0,   0,   "RED");
  fillAndHold(0,   255, 0,   "GREEN");
  fillAndHold(0,   0,   255, "BLUE");
  fillAndHold(255, 255, 255, "WHITE");
  fillAndHold(0,   0,   0,   "OFF");

  // 2) Moving vertical line — confirms columns/scan are healthy.
  for (int x = 0; x < W; x++) {
    dma->clearScreen();
    dma->drawLine(x, 0, x, H - 1, dma->color565(0x1D, 0xB9, 0x54)); // Spotify green
    dma->flipDMABuffer();
    delay(40);
  }
}
