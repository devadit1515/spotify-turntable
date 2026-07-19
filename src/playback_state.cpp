#include "playback_state.h"

PlaybackState     gState;
SemaphoreHandle_t gStateMutex = nullptr;

uint16_t          gArt[ART_W * ART_H];
SemaphoreHandle_t gArtMutex = nullptr;
volatile uint32_t gArtVersion = 0;

void stateInit() {
  gStateMutex = xSemaphoreCreateMutex();
  gArtMutex   = xSemaphoreCreateMutex();
  // Start with a neutral dark-grey art buffer so the panel isn't garbage before
  // the first track loads.
  for (int i = 0; i < ART_W * ART_H; i++) gArt[i] = 0x2104;  // ~dark grey in RGB565
}

PlaybackState stateSnapshot() {
  PlaybackState copy;
  if (gStateMutex && xSemaphoreTake(gStateMutex, portMAX_DELAY) == pdTRUE) {
    copy = gState;
    xSemaphoreGive(gStateMutex);
  }
  return copy;
}

long stateInterpolatedProgress(const PlaybackState &s) {
  long p = s.progressMs;
  if (s.isPlaying) {
    p += (long)(millis() - s.lastSyncMs);
  }
  if (s.durationMs > 0 && p > s.durationMs) p = s.durationMs;
  if (p < 0) p = 0;
  return p;
}
