// ─────────────────────────────────────────────────────────────────────────────
//  The Spotify Turntable — main.cpp
//
//  A 64×64 RGB matrix shows live album art + title + progress, a 3D-printed
//  record physically spins while music plays, and a WS2812B halo glows in the
//  album's colours.
//
//  Task / core layout:
//    core 0 : spotifyTask  — WiFi, token refresh, currently-playing poll,
//                            album-art download + JPEG decode (all blocking work)
//    core 1 : loop()       — matrix rendering (~60 fps, double-buffered)
//             motionTask   — 28BYJ-48 stepper (record spin)
//             haloTask     — WS2812B ring
//
//  Shared state lives in playback_state.{h,cpp}, guarded by two short-held
//  mutexes. See docs/ for wiring, BOM, and the build guide.
// ─────────────────────────────────────────────────────────────────────────────
#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "playback_state.h"
#include "display_ui.h"
#include "spotify_client.h"
#include "motion.h"
#include "halo.h"

static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);                 // keep latency low for API polls
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print(F("[wifi] connecting"));
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print('.');
    if (millis() - start > 20000) {     // stuck? kick it and retry
      Serial.println(F(" retry"));
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      start = millis();
    }
  }
  Serial.printf("\n[wifi] connected, IP %s\n", WiFi.localIP().toString().c_str());
}

// Core-0 task: everything that touches the network.
static void spotifyTask(void *param) {
  spotifyInit();
  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }
    spotifyPollOnce();
    vTaskDelay(pdMS_TO_TICKS(SPOTIFY_POLL_MS));
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("\n=== Spotify Turntable ==="));
  if (psramFound()) {
    Serial.printf("[mem] PSRAM: %u bytes free\n", (unsigned)ESP.getFreePsram());
  } else {
    Serial.println(F("[mem] WARNING: no PSRAM detected — check board_build.arduino.memory_type"));
  }

  stateInit();
  displayInit();     // splash appears right away, before WiFi is up
  motionInit();
  haloInit();

  connectWiFi();

  // Blocking / network work on core 0.
  xTaskCreatePinnedToCore(spotifyTask, "spotify", 16384, nullptr, 1, nullptr, 0);
  // Physical animation on core 1 (loop() also runs here and does the rendering).
  xTaskCreatePinnedToCore(motionTask,  "motion",   2048, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(haloTask,    "halo",     4096, nullptr, 1, nullptr, 1);
}

void loop() {
  displayRenderFrame();
  delay(5);          // ~ up to 60–120 fps; yields so motion/halo get their slices
}
