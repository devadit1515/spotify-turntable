#include "display_ui.h"
#include "pins.h"
#include "config.h"
#include "playback_state.h"

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>

static MatrixPanel_I2S_DMA *dma = nullptr;

// Layout adapts to panel height (set via ART_W/ART_H in playback_state.h):
//   • Big panels (≥48 px tall, e.g. 64×64): full art + a darkened caption band
//     with a scrolling "Title — Artist" marquee + progress bar.
//   • Small panels (e.g. 32×32): no room for text — full-bleed art + a thin
//     progress bar only. The cover art carries it.
static const bool BIG_PANEL   = (ART_H >= 48);
static const int  CAPTION_TOP = BIG_PANEL ? (ART_H - 15) : (ART_H - 2);
static const int  TEXT_Y      = CAPTION_TOP + 2;
static const int  BAR_Y       = ART_H - 2;   // 2-px progress bar at the very bottom

static uint16_t SPOTIFY_GREEN;
static uint16_t frameArt[ART_W * ART_H];

static inline uint16_t dim565(uint16_t c, uint8_t num, uint8_t den) {
  uint16_t r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  r = (uint16_t)(r * num / den);
  g = (uint16_t)(g * num / den);
  b = (uint16_t)(b * num / den);
  return (uint16_t)((r << 11) | (g << 5) | b);
}
static inline uint16_t fade565(uint16_t c, uint8_t alpha) { return dim565(c, alpha, 255); }

void displayInit() {
  HUB75_I2S_CFG::i2s_pins pinmap = {
    PIN_R1, PIN_G1, PIN_B1, PIN_R2, PIN_G2, PIN_B2,
    PIN_A,  PIN_B,  PIN_C,  PIN_D,  PIN_E,
    PIN_LAT, PIN_OE, PIN_CLK
  };

  // ART_W×ART_H drives the scan config: a 32-high panel is 1/16-scan (A–D only,
  // E unused); a 64-high panel is 1/32-scan (A–E). The library handles this.
  HUB75_I2S_CFG mxconfig(ART_W, ART_H, 1 /* chain */, pinmap);
  mxconfig.double_buff = true;
  mxconfig.clkphase    = false;   // flip to true if the image looks shifted 1px
  // mxconfig.driver = HUB75_I2S_CFG::FM6126A;  // uncomment if the panel shows garbage

  dma = new MatrixPanel_I2S_DMA(mxconfig);
  dma->begin();
  dma->setBrightness8(MATRIX_BRIGHTNESS);
  dma->clearScreen();

  SPOTIFY_GREEN = dma->color565(0x1D, 0xB9, 0x54);
  memcpy(frameArt, gArt, sizeof(frameArt));   // seed from the grey placeholder
}

static void drawSplash() {
  if (BIG_PANEL) {
    dma->fillScreen(0);
    dma->setTextSize(1);
    dma->setTextWrap(false);
    dma->setTextColor(SPOTIFY_GREEN);
    dma->setCursor(6, 20);  dma->print("SPOTIFY");
    dma->setTextColor(dim565(0xFFFF, 2, 3));
    dma->setCursor(10, 34); dma->print("turntable");
    int dots = (millis() / 400) % 4;
    dma->setTextColor(SPOTIFY_GREEN);
    dma->setCursor(24, 46);
    for (int i = 0; i < dots; i++) dma->print(".");
  } else {
    // Too small for text — gently pulse the whole panel green while connecting.
    float b = 0.20f + 0.30f * (0.5f + 0.5f * sinf(millis() / 500.0f));
    dma->fillScreen(dim565(SPOTIFY_GREEN, (uint8_t)(b * 255), 255));
  }
  dma->flipDMABuffer();
}

void displayRenderFrame() {
  if (!dma) return;

  PlaybackState s = stateSnapshot();
  if (!s.valid) { drawSplash(); return; }

  // ── fade-in a freshly decoded cover ────────────────────────────────────────
  static uint32_t lastArtVersion = 0xFFFFFFFF;
  static uint32_t fadeStart = 0;
  uint32_t v = gArtVersion;
  if (v != lastArtVersion) {
    lastArtVersion = v;
    fadeStart = millis();
    if (gArtMutex && xSemaphoreTake(gArtMutex, portMAX_DELAY) == pdTRUE) {
      memcpy(frameArt, gArt, sizeof(frameArt));
      xSemaphoreGive(gArtMutex);
    }
  }
  uint32_t elapsed = millis() - fadeStart;
  uint8_t alpha = (elapsed >= 400) ? 255 : (uint8_t)(elapsed * 255UL / 400UL);

  // ── blit art (darken the bottom band where the bar/caption sit) ────────────
  for (int y = 0; y < ART_H; y++) {
    bool band = (y >= CAPTION_TOP);
    const uint16_t *src = frameArt + (y * ART_W);
    for (int x = 0; x < ART_W; x++) {
      uint16_t c = src[x];
      if (alpha < 255) c = fade565(c, alpha);
      if (band)        c = dim565(c, 1, 3);
      dma->drawPixel(x, y, c);
    }
  }

  // ── scrolling caption (big panels only — no room on 32 px) ─────────────────
  if (BIG_PANEL) {
    static char lastTrack[48] = {0};
    static int  scrollX = ART_W;
    static uint32_t lastScrollMs = 0;

    if (strncmp(lastTrack, s.trackId, sizeof(lastTrack) - 1) != 0) {
      strncpy(lastTrack, s.trackId, sizeof(lastTrack) - 1);
      lastTrack[sizeof(lastTrack) - 1] = '\0';
      scrollX = ART_W;
    }

    char caption[300];
    snprintf(caption, sizeof(caption), "%s  -  %s", s.trackName, s.artistName);

    dma->setTextSize(1);
    dma->setTextWrap(false);
    dma->setTextColor(0xFFFF);

    int16_t bx, by; uint16_t bw, bh;
    dma->getTextBounds(caption, 0, 0, &bx, &by, &bw, &bh);

    if (bw <= ART_W) {
      dma->setCursor((ART_W - (int)bw) / 2, TEXT_Y);
      dma->print(caption);
    } else {
      if (millis() - lastScrollMs > 40) {
        lastScrollMs = millis();
        if (--scrollX < -(int)bw) scrollX = ART_W;
      }
      dma->setCursor(scrollX, TEXT_Y);
      dma->print(caption);
    }
  }

  // ── progress bar (all panels) ──────────────────────────────────────────────
  long prog = stateInterpolatedProgress(s);
  int filled = (s.durationMs > 0) ? (int)((int64_t)ART_W * prog / s.durationMs) : 0;
  if (filled < 0) filled = 0;
  if (filled > ART_W) filled = ART_W;

  uint16_t track = dim565(0xFFFF, 1, 6);
  for (int by = BAR_Y; by < ART_H; by++) {
    dma->drawFastHLine(0, by, ART_W, track);
    if (filled > 0) dma->drawFastHLine(0, by, filled, SPOTIFY_GREEN);
  }

  dma->flipDMABuffer();
}
