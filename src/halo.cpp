#include "halo.h"
#include "pins.h"
#include "config.h"
#include "playback_state.h"

#include <Adafruit_NeoPixel.h>
#include <math.h>

#if HALO_USE_MIC
  #include <driver/i2s.h>   // legacy I²S API (compiles on IDF4/5; deprecation warnings only)
#endif

static Adafruit_NeoPixel ring(HALO_LED_COUNT, PIN_LED_DIN, NEO_GRB + NEO_KHZ800);

// Accent colour sampled from the album art (8-bit RGB), refreshed on art change.
static uint8_t  accR = 0x1D, accG = 0xB9, accB = 0x54;   // default: Spotify green
static uint32_t seenArtVersion = 0xFFFFFFFF;

// Pull an average colour out of the current cover; boost it a little so a muddy
// average still reads as a colour on the ring.
static void refreshAccent() {
  if (!gArtMutex) return;
  if (xSemaphoreTake(gArtMutex, portMAX_DELAY) != pdTRUE) return;
  uint32_t r = 0, g = 0, b = 0;
  const int N = ART_W * ART_H;
  for (int i = 0; i < N; i++) {
    uint16_t c = gArt[i];
    r += ((c >> 11) & 0x1F) << 3;
    g += ((c >> 5)  & 0x3F) << 2;
    b += ( c        & 0x1F) << 3;
  }
  xSemaphoreGive(gArtMutex);
  r /= N; g /= N; b /= N;

  // Lift toward full brightness so dark covers still tint the ring.
  uint32_t mx = r;
  if (g > mx) mx = g;
  if (b > mx) mx = b;
  if (mx > 0 && mx < 200) {
    uint32_t k = 200;
    r = r * k / mx; g = g * k / mx; b = b * k / mx;
  }
  accR = (uint8_t)(r > 255 ? 255 : r);
  accG = (uint8_t)(g > 255 ? 255 : g);
  accB = (uint8_t)(b > 255 ? 255 : b);
}

static inline uint32_t scaled(float inten) {
  if (inten < 0) inten = 0; if (inten > 1) inten = 1;
  return ring.Color((uint8_t)(accR * inten), (uint8_t)(accG * inten), (uint8_t)(accB * inten));
}

void haloInit() {
  ring.begin();
  ring.setBrightness(200);   // headroom; per-pixel intensity does the rest
  ring.clear();
  ring.show();
}

// ── Mode A: breathing pulse + rotating comet (no mic) ─────────────────────────
static void renderBreathing(bool playing) {
  float t = millis() / 1000.0f;
  // Slow, calm breath while playing; a dim steady glow while paused.
  float period = playing ? 3.0f : 6.0f;
  float breath = 0.5f + 0.5f * sinf(2.0f * PI * t / period);
  float base   = playing ? (0.30f + 0.30f * breath) : 0.10f;

  // Comet head drifts around the ring to suggest the record turning.
  float head = playing ? fmodf(t * (HALO_LED_COUNT / 4.0f), (float)HALO_LED_COUNT) : -10.0f;

  for (int i = 0; i < HALO_LED_COUNT; i++) {
    float d = fabsf(i - head);
    d = min(d, (float)HALO_LED_COUNT - d);          // wrap-around distance
    float comet = playing ? expf(-(d * d) / 2.0f) * 0.5f : 0.0f;
    ring.setPixelColor(i, scaled(base + comet));
  }
  ring.show();
}

#if HALO_USE_MIC
// ── Mode B: audio-reactive VU ring (INMP441) ──────────────────────────────────
static bool  i2sReady = false;
static float level    = 0.0f;   // smoothed 0..1

static void micInit() {
  i2s_config_t cfg = {};
  cfg.mode                = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  cfg.sample_rate         = 16000;
  cfg.bits_per_sample     = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format      = I2S_CHANNEL_FMT_ONLY_LEFT;
  cfg.communication_format= I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags    = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count       = 4;
  cfg.dma_buf_len         = 256;
  cfg.use_apll            = false;

  i2s_pin_config_t pins = {};
  pins.bck_io_num   = PIN_I2S_SCK;
  pins.ws_io_num    = PIN_I2S_WS;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num  = PIN_I2S_SD;

  if (i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr) == ESP_OK &&
      i2s_set_pin(I2S_NUM_0, &pins) == ESP_OK) {
    i2sReady = true;
  } else {
    Serial.println(F("[halo] I2S mic init failed — falling back to breathing"));
  }
}

static void renderMic(bool playing) {
  if (!i2sReady) { renderBreathing(playing); return; }

  static int32_t buf[256];
  size_t bytesRead = 0;
  i2s_read(I2S_NUM_0, buf, sizeof(buf), &bytesRead, pdMS_TO_TICKS(20));
  int samples = bytesRead / sizeof(int32_t);

  // RMS of the 24-bit-in-32 samples.
  double sumSq = 0;
  for (int i = 0; i < samples; i++) {
    float s = (float)(buf[i] >> 8) / 8388608.0f;   // → roughly -1..1
    sumSq += (double)s * s;
  }
  float rms = samples ? sqrtf(sumSq / samples) : 0.0f;

  // Auto-gain + smoothing so quiet and loud rooms both look alive.
  float target = min(1.0f, rms * 6.0f);
  level += (target - level) * 0.25f;

  int lit = (int)(level * HALO_LED_COUNT + 0.5f);
  for (int i = 0; i < HALO_LED_COUNT; i++) {
    float inten = (i < lit) ? (0.25f + 0.75f * ((float)i / HALO_LED_COUNT)) : 0.06f;
    if (!playing) inten = 0.08f;
    ring.setPixelColor(i, scaled(inten));
  }
  ring.show();
}
#endif  // HALO_USE_MIC

void haloTask(void *param) {
#if HALO_USE_MIC
  micInit();
#endif
  for (;;) {
    // Refresh the accent colour when a new cover is decoded.
    uint32_t v = gArtVersion;
    if (v != seenArtVersion) { seenArtVersion = v; refreshAccent(); }

    bool playing = false;
    if (gStateMutex && xSemaphoreTake(gStateMutex, 0) == pdTRUE) {
      playing = gState.isPlaying && gState.valid;
      xSemaphoreGive(gStateMutex);
    }

#if HALO_USE_MIC
    renderMic(playing);
    vTaskDelay(pdMS_TO_TICKS(15));
#else
    renderBreathing(playing);
    vTaskDelay(pdMS_TO_TICKS(25));   // ~40 fps breathing
#endif
  }
}
