#include "display_ui.h"
#include "pins.h"
#include "config.h"
#include "playback_state.h"

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

static MatrixPanel_I2S_DMA *dma = nullptr;

// Caption band geometry.
static const int CAPTION_TOP = 49;   // first darkened row
static const int TEXT_Y      = 51;   // top of the marquee glyphs (default 6×8 font)
static const int BAR_Y       = 62;   // 2-px progress bar occupies rows 62–63

// Spotify green (#1DB954) in RGB565, computed once.
static uint16_t SPOTIFY_GREEN;

// A private copy of the art we blit each frame (avoids holding gArtMutex during
// the whole 4096-pixel loop). Initialised to the grey placeholder gArt holds.
static uint16_t frameArt[ART_W * ART_H];

// ── small colour helpers ─────────────────────────────────────────────────────
static inline uint16_t dim565(uint16_t c, uint8_t num, uint8_t den) {
  uint16_t r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  r = (uint16_t)(r * num / den);
  g = (uint16_t)(g * num / den);
  b = (uint16_t)(b * num / den);
  return (uint16_t)((r << 11) | (g << 5) | b);
}
static inline uint16_t fade565(uint16_t c, uint8_t alpha) {  // alpha 0..255
  return dim565(c, alpha, 255);
}

void displayInit() {
  HUB75_I2S_CFG::i2s_pins pinmap = {
    PIN_R1, PIN_G1, PIN_B1, PIN_R2, PIN_G2, PIN_B2,
    PIN_A,  PIN_B,  PIN_C,  PIN_D,  PIN_E,
    PIN_LAT, PIN_OE, PIN_CLK
  };

  HUB75_I2S_CFG mxconfig(ART_W, ART_H, 1 /* chain length */, pinmap);
  mxconfig.double_buff = true;     // tear-free text overlay
  mxconfig.clkphase    = false;    // flip to true if the image looks shifted 1px
  // Some panels need a specific driver chip; uncomment if yours shows garbage:
  // mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  dma = new MatrixPanel_I2S_DMA(mxconfig);
  dma->begin();
  dma->setBrightness8(MATRIX_BRIGHTNESS);
  dma->clearScreen();

  SPOTIFY_GREEN = dma->color565(0x1D, 0xB9, 0x54);

  // Seed frameArt from the placeholder so the very first frames aren't garbage.
  memcpy(frameArt, gArt, sizeof(frameArt));
}

// Boot / "nothing yet" splash: a green pill + animated dots.
static void drawSplash() {
  dma->fillScreen(0);
  dma->setTextColor(SPOTIFY_GREEN);
  dma->setTextSize(1);
  dma->setTextWrap(false);
  dma->setCursor(6, 20);
  dma->print("SPOTIFY");
  dma->setTextColor(dim565(0xFFFF, 2, 3));
  dma->setCursor(10, 34);
  dma->print("turntable");
  // three blinking connecting dots
  int dots = (millis() / 400) % 4;
  dma->setTextColor(SPOTIFY_GREEN);
  dma->setCursor(24, 46);
  for (int i = 0; i < dots; i++) dma->print(".");
  dma->flipDMABuffer();
}

void displayRenderFrame() {
  if (!dma) return;

  PlaybackState s = stateSnapshot();
  if (!s.valid) { drawSplash(); return; }

  // ── fade-in bookkeeping on a new cover ─────────────────────────────────────
  static uint32_t lastArtVersion = 0xFFFFFFFF;
  static uint32_t fadeStart = 0;
  uint32_t v = gArtVersion;
  if (v != lastArtVersion) {
    lastArtVersion = v;
    fadeStart = millis();
    // Refresh our private art copy (brief lock).
    if (gArtMutex && xSemaphoreTake(gArtMutex, portMAX_DELAY) == pdTRUE) {
      memcpy(frameArt, gArt, sizeof(frameArt));
      xSemaphoreGive(gArtMutex);
    }
  }
  uint32_t elapsed = millis() - fadeStart;
  uint8_t alpha = (elapsed >= 400) ? 255 : (uint8_t)(elapsed * 255UL / 400UL);

  // ── blit art (+ darken caption band, + apply fade) ─────────────────────────
  for (int y = 0; y < ART_H; y++) {
    bool band = (y >= CAPTION_TOP);
    const uint16_t *src = frameArt + (y * ART_W);
    for (int x = 0; x < ART_W; x++) {
      uint16_t c = src[x];
      if (alpha < 255) c = fade565(c, alpha);
      if (band)        c = dim565(c, 1, 4);
      dma->drawPixel(x, y, c);
    }
  }

  // ── scrolling caption ──────────────────────────────────────────────────────
  static char lastTrack[48] = {0};
  static int  scrollX = ART_W;
  static uint32_t lastScrollMs = 0;

  if (strncmp(lastTrack, s.trackId, sizeof(lastTrack) - 1) != 0) {
    strncpy(lastTrack, s.trackId, sizeof(lastTrack) - 1);
    lastTrack[sizeof(lastTrack) - 1] = '\0';
    scrollX = ART_W;                 // restart marquee for the new track
  }

  char caption[300];
  snprintf(caption, sizeof(caption), "%s  -  %s", s.trackName, s.artistName);

  dma->setTextSize(1);
  dma->setTextWrap(false);
  dma->setTextColor(0xFFFF);

  int16_t bx, by; uint16_t bw, bh;
  dma->getTextBounds(caption, 0, 0, &bx, &by, &bw, &bh);

  if (bw <= ART_W) {
    dma->setCursor((ART_W - (int)bw) / 2, TEXT_Y);   // short title: centre it
    dma->print(caption);
  } else {
    if (millis() - lastScrollMs > 40) {              // ~25 px/s marquee
      lastScrollMs = millis();
      if (--scrollX < -(int)bw) scrollX = ART_W;      // wrap with a gap
    }
    dma->setCursor(scrollX, TEXT_Y);
    dma->print(caption);
  }

  // ── progress bar ───────────────────────────────────────────────────────────
  long prog = stateInterpolatedProgress(s);
  int filled = 0;
  if (s.durationMs > 0) filled = (int)((int64_t)ART_W * prog / s.durationMs);
  if (filled < 0) filled = 0;
  if (filled > ART_W) filled = ART_W;

  uint16_t track = dim565(0xFFFF, 1, 6);
  dma->drawFastHLine(0, BAR_Y,     ART_W, track);
  dma->drawFastHLine(0, BAR_Y + 1, ART_W, track);
  if (filled > 0) {
    dma->drawFastHLine(0, BAR_Y,     filled, SPOTIFY_GREEN);
    dma->drawFastHLine(0, BAR_Y + 1, filled, SPOTIFY_GREEN);
  }

  dma->flipDMABuffer();
}
