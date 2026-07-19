// ─────────────────────────────────────────────────────────────────────────────
//  spotify_client.h — talks to the Spotify Web API.
//
//  Wraps witnessmenow/spotify-api-arduino: refreshes the 1-hour access token
//  from your long-lived refresh token, polls /me/player/currently-playing, and
//  writes the result into gState. When the track changes it kicks off an album-
//  art download (album_art module). All of this runs on core 0.
// ─────────────────────────────────────────────────────────────────────────────
#pragma once

// Connect the API client and grab the first access token. Call once, after WiFi.
void spotifyInit();

// One poll cycle: refresh token if due, ask "what's playing?", update gState,
// and fetch new album art on a track change. Call every SPOTIFY_POLL_MS.
void spotifyPollOnce();
