#include "album_art.h"
#include "playback_state.h"

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <JPEGDEC.h>
#include <esp_heap_caps.h>   // heap_caps_malloc / MALLOC_CAP_SPIRAM

// Scratch buffer the decoder writes into. We only publish to gArt once decoding
// finishes, so the renderer never sees a half-drawn image. 64*64*2 = 8 KB.
static uint16_t artScratch[ART_W * ART_H];
static JPEGDEC  jpeg;

// JPEGDEC hands us the image one MCU block at a time (RGB565). Clamp each block
// into the 64×64 scratch — this makes any source size / offset safe.
static int jpegDrawBlock(JPEGDRAW *p) {
  for (int j = 0; j < p->iHeight; j++) {
    int y = p->y + j;
    if (y < 0 || y >= ART_H) continue;
    const uint16_t *row = p->pPixels + (j * p->iWidth);
    uint16_t *dst = artScratch + (y * ART_W);
    for (int i = 0; i < p->iWidth; i++) {
      int x = p->x + i;
      if (x < 0 || x >= ART_W) continue;
      dst[x] = row[i];
    }
  }
  return 1;  // keep going
}

// Map a JPEGDEC scale option to the bit-shift it applies to the dimensions.
static int scaleShift(int opt) {
  switch (opt) {
    case JPEG_SCALE_HALF:    return 1;
    case JPEG_SCALE_QUARTER: return 2;
    case JPEG_SCALE_EIGHTH:  return 3;
    default:                 return 0;
  }
}

static bool decodeToScratch(uint8_t *data, size_t size) {
  if (!jpeg.openRAM(data, (int)size, jpegDrawBlock)) return false;

  const int w = jpeg.getWidth();
  const int h = jpeg.getHeight();

  // Choose the largest power-of-two downscale that still overfills 64×64, so we
  // keep as much detail as possible then center-crop.
  int opt = 0;
  if      (w >= 512 || h >= 512) opt = JPEG_SCALE_EIGHTH;
  else if (w >= 256 || h >= 256) opt = JPEG_SCALE_QUARTER;   // 300 → 75
  else if (w >= 128 || h >= 128) opt = JPEG_SCALE_HALF;

  const int shift = scaleShift(opt);
  const int dw = w >> shift;
  const int dh = h >> shift;

  // Center (crop if bigger than 64, pad if smaller). Pre-clear so any padding
  // border shows as black rather than stale pixels.
  for (int i = 0; i < ART_W * ART_H; i++) artScratch[i] = 0x0000;
  const int offX = (ART_W - dw) / 2;
  const int offY = (ART_H - dh) / 2;

  // RGB565_BIG_ENDIAN matches Adafruit-GFX/HUB75 colour order. If reds and blues
  // look swapped on the panel, change this to RGB565_LITTLE_ENDIAN.
  jpeg.setPixelType(RGB565_BIG_ENDIAN);
  int ok = jpeg.decode(offX, offY, opt);
  jpeg.close();
  return ok == 1;
}

// Read the whole HTTP body into a buffer (PSRAM preferred). Small images, so an
// 80 KB cap is plenty even for a 300×300 cover.
static uint8_t *downloadToBuffer(const char *url, size_t &outLen) {
  WiFiClientSecure tls;
  tls.setInsecure();

  HTTPClient http;
  if (!http.begin(tls, url)) return nullptr;
  http.setTimeout(8000);
  // Spotify's image CDN 302-redirects; follow it.
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("[art] HTTP GET failed: %d\n", code);
    http.end();
    return nullptr;
  }

  int len = http.getSize();                 // may be -1 (chunked)
  size_t cap = (len > 0) ? (size_t)len : 80 * 1024;
  if (cap > 80 * 1024) cap = 80 * 1024;

  uint8_t *buf = (uint8_t *)heap_caps_malloc(cap, MALLOC_CAP_SPIRAM);
  if (!buf) buf = (uint8_t *)malloc(cap);   // fall back to internal RAM
  if (!buf) { http.end(); return nullptr; }

  WiFiClient *stream = http.getStreamPtr();
  size_t got = 0;
  uint32_t lastData = millis();
  while (http.connected() && got < cap) {
    if (len > 0 && got >= (size_t)len) break;
    size_t avail = stream->available();
    if (avail) {
      size_t want = cap - got;
      if (avail < want) want = avail;
      int r = stream->readBytes(buf + got, want);
      if (r > 0) { got += r; lastData = millis(); }
    } else {
      if (millis() - lastData > 8000) break;  // stalled
      vTaskDelay(pdMS_TO_TICKS(2));
    }
  }
  http.end();

  if (got == 0) { free(buf); return nullptr; }
  outLen = got;
  return buf;
}

bool albumArtFetch(const char *url) {
  if (!url || url[0] == '\0') return false;

  size_t len = 0;
  uint8_t *buf = downloadToBuffer(url, len);
  if (!buf) return false;

  bool ok = decodeToScratch(buf, len);
  free(buf);
  if (!ok) { Serial.println(F("[art] decode failed")); return false; }

  // Publish: copy scratch → gArt under the mutex, then bump the version so the
  // renderer swaps to it on its next frame.
  if (gArtMutex && xSemaphoreTake(gArtMutex, portMAX_DELAY) == pdTRUE) {
    memcpy(gArt, artScratch, sizeof(artScratch));
    gArtVersion++;
    xSemaphoreGive(gArtMutex);
  }
  Serial.println(F("[art] new cover decoded"));
  return true;
}
