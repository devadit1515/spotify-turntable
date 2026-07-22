// ─────────────────────────────────────────────────────────────────────────────
//  playback_state.h — the single source of truth shared between the two cores.
//
//  Core 0 (spotifyTask)  WRITES  gState + gArt when a new poll/track arrives.
//  Core 1 (render loop, motionTask, haloTask)  READ  them every frame.
//
//  Two small mutexes keep it race-free without ever blocking the matrix refresh
//  (which runs on I²S DMA hardware, independent of the CPU):
//    • gStateMutex guards the metadata struct.
//    • gArtMutex   guards the decoded pixel buffer.
//  Both are held only for microseconds (a struct copy / a flag flip).
// ─────────────────────────────────────────────────────────────────────────────
#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Panel resolution — the single source of truth. This build targets a 32×32
// HUB75 panel. If you swap to a 64×64 later, change BOTH of these to 64 and the
// display code re-expands automatically (art + caption band + progress bar).
static const int ART_W = 32;
static const int ART_H = 32;

struct PlaybackState {
  char     trackId[48]    = {0};   // Spotify track id (used to detect track change)
  char     trackName[128] = {0};
  char     artistName[128]= {0};
  char     artUrl[300]    = {0};   // URL of the album-art JPEG we want to fetch
  long     progressMs     = 0;     // playback position at the moment of last sync
  long     durationMs     = 0;
  bool     isPlaying      = false;
  bool     valid          = false; // true once we've received at least one payload
  uint32_t lastSyncMs     = 0;     // millis() when progressMs was set from the API
};

extern PlaybackState     gState;
extern SemaphoreHandle_t gStateMutex;

// Decoded album art, 64×64 RGB565. gArtVersion increments each time it changes,
// so the renderer can cheaply notice "new art" without locking every frame.
extern uint16_t          gArt[ART_W * ART_H];
extern SemaphoreHandle_t gArtMutex;
extern volatile uint32_t gArtVersion;

// Call once from setup() before spawning tasks.
void stateInit();

// Locked snapshot — copy the whole struct out under the mutex, then use freely.
PlaybackState stateSnapshot();

// Estimated *current* position: the last synced progress plus the wall-clock time
// elapsed since, but only advancing while playing and clamped to the duration.
// This gives a smooth progress bar between the (slow) 3-second API polls.
long stateInterpolatedProgress(const PlaybackState &s);
