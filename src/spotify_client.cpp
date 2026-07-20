#include "spotify_client.h"
#include "config.h"
#include "playback_state.h"
#include "album_art.h"

#include <WiFiClientSecure.h>
#include <SpotifyArduino.h>

// Dedicated TLS client for the API. Album art uses its own client so the two
// HTTPS connections never stomp on each other.
static WiFiClientSecure spotifyTLS;

// IMPORTANT: this is a file-scope (static-storage) object, NOT `new`. Static
// objects are zero-initialised before their constructor runs, so SpotifyArduino's
// internal _refreshToken pointer is genuinely NULL when the constructor calls
// setRefreshToken(). The library does `delete _refreshToken` there without ever
// initialising it — with `new` (and ESP32 heap poisoning on) that pointer is
// garbage and the delete corrupts the heap → crash. Zero-init side-steps the bug.
static SpotifyArduino spotify(spotifyTLS,
                              SPOTIFY_CLIENT_ID,
                              SPOTIFY_CLIENT_SECRET,
                              SPOTIFY_REFRESH_TOKEN);

static uint32_t        lastRefreshMs = 0;
static const uint32_t  TOKEN_REFRESH_INTERVAL = 50UL * 60UL * 1000UL;  // 50 min

// A pending art fetch discovered inside the parse callback, executed afterwards.
static char  pendingArtUrl[300]  = {0};
static bool  pendingArtFetch     = false;
static char  lastArtTrackId[48]  = {0};

static void copyStr(char *dst, size_t cap, const char *src) {
  if (!src) { dst[0] = 0; return; }
  strncpy(dst, src, cap - 1);
  dst[cap - 1] = '\0';
}

// Runs on core 0 during JSON parsing. Keep it light: the char* fields in `track`
// are only valid for the duration of this call, so we copy what we need out.
static void onCurrentlyPlaying(CurrentlyPlaying track) {
  // Compact track id from trackUri ("spotify:track:ID") — used to spot changes.
  const char *uri = track.trackUri ? track.trackUri : "";
  const char *sep = strrchr(uri, ':');
  const char *id  = sep ? sep + 1 : uri;

  // Smallest album image (last in the array) is ~64×64 — perfect for decoding
  // straight onto the panel with no scaling.
  const char *artUrl = "";
  if (track.numImages > 0 && track.albumImages[track.numImages - 1].url) {
    artUrl = track.albumImages[track.numImages - 1].url;
  }

  const char *artist = (track.numArtists > 0 && track.artists[0].artistName)
                         ? track.artists[0].artistName : "";

  if (gStateMutex && xSemaphoreTake(gStateMutex, portMAX_DELAY) == pdTRUE) {
    copyStr(gState.trackId,    sizeof(gState.trackId),    id);
    copyStr(gState.trackName,  sizeof(gState.trackName),  track.trackName);
    copyStr(gState.artistName, sizeof(gState.artistName), artist);
    copyStr(gState.artUrl,     sizeof(gState.artUrl),     artUrl);
    gState.progressMs = track.progressMs;
    gState.durationMs = track.durationMs;
    gState.isPlaying  = track.isPlaying;
    gState.lastSyncMs = millis();
    gState.valid      = true;
    xSemaphoreGive(gStateMutex);
  }

  // New track (vs. the last one we fetched art for)? queue a download.
  if (artUrl[0] != '\0' && strcmp(id, lastArtTrackId) != 0) {
    copyStr(pendingArtUrl,  sizeof(pendingArtUrl),  artUrl);
    copyStr(lastArtTrackId, sizeof(lastArtTrackId), id);
    pendingArtFetch = true;
  }
}

void spotifyInit() {
  // Spotify rotates its API and CDN certificates, so a pinned cert breaks over
  // time. setInsecure() skips validation — acceptable for a personal desk device
  // (see docs/build-guide.md for the PKCE/proxy hardening path).
  spotifyTLS.setInsecure();

  Serial.println(F("[spotify] refreshing access token..."));
  if (spotify.refreshAccessToken()) {
    Serial.println(F("[spotify] access token OK"));
    lastRefreshMs = millis();
  } else {
    Serial.println(F("[spotify] TOKEN REFRESH FAILED — check client id/secret/refresh token"));
  }
}

void spotifyPollOnce() {
  // Proactive refresh (access tokens live ~60 min).
  if (millis() - lastRefreshMs > TOKEN_REFRESH_INTERVAL) {
    if (spotify.refreshAccessToken()) lastRefreshMs = millis();
  }

  pendingArtFetch = false;
  int status = spotify.getCurrentlyPlaying(onCurrentlyPlaying, SPOTIFY_MARKET);

  switch (status) {
    case 200:
      // Metadata already applied in the callback.
      break;
    case 204:
      // Valid "nothing playing" — keep the last art up, just mark it paused.
      if (gStateMutex && xSemaphoreTake(gStateMutex, portMAX_DELAY) == pdTRUE) {
        gState.isPlaying = false;
        xSemaphoreGive(gStateMutex);
      }
      break;
    case 401:
      Serial.println(F("[spotify] 401 — token expired, refreshing"));
      if (spotify.refreshAccessToken()) lastRefreshMs = millis();
      break;
    case 429:
      Serial.println(F("[spotify] 429 rate-limited — backing off 5s"));
      vTaskDelay(pdMS_TO_TICKS(5000));
      break;
    default:
      Serial.printf("[spotify] poll returned %d\n", status);
      break;
  }

  // Heavy work (HTTPS download + JPEG decode) happens here, AFTER the parser has
  // released its buffers — never inside the callback.
  if (pendingArtFetch) {
    Serial.printf("[spotify] new track, fetching art: %s\n", pendingArtUrl);
    if (!albumArtFetch(pendingArtUrl)) {
      Serial.println(F("[spotify] art fetch failed (keeping previous image)"));
    }
    pendingArtFetch = false;
  }
}
