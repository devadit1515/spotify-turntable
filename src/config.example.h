// ─────────────────────────────────────────────────────────────────────────────
//  config.example.h  →  copy to  config.h  and fill in your own values.
//
//  config.h is git-ignored on purpose: it holds your WiFi password, your Spotify
//  client secret, and your long-lived refresh token. NEVER commit the real file.
//
//  How to get the Spotify values (one-time, ~5 min):
//    1. Go to https://developer.spotify.com/dashboard  → Create app.
//         - Redirect URI: http://<your-esp32-ip>/callback   (any http URI works
//           for the one-time token grab; the getRefreshToken sketch prints it).
//         - Note the Client ID and Client Secret.
//         - Under "API/SDKs" tick "Web API".
//    2. Flash tools/getRefreshToken (see docs/build-guide.md, Phase 1). It opens
//         a tiny web page on the ESP32, you log in once, and it prints your
//         REFRESH TOKEN to the serial monitor.
//    3. Paste all three values below. Done — the firmware mints 1-hour access
//         tokens from the refresh token forever after.
// ─────────────────────────────────────────────────────────────────────────────
#pragma once

// ── WiFi ─────────────────────────────────────────────────────────────────────
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// ── Spotify ──────────────────────────────────────────────────────────────────
#define SPOTIFY_CLIENT_ID       "your_spotify_client_id"
#define SPOTIFY_CLIENT_SECRET   "your_spotify_client_secret"
#define SPOTIFY_REFRESH_TOKEN   "your_long_lived_refresh_token"
// Optional market/country code (helps some region-locked tracks resolve art).
#define SPOTIFY_MARKET          "IN"

// ── Behaviour tuning ─────────────────────────────────────────────────────────
// How often to ask Spotify "what's playing?" (ms). 2–5 s is well within limits.
#define SPOTIFY_POLL_MS         3000

// Matrix brightness 0–255. Cap ~180 (≈70%) to limit current draw + heat.
#define MATRIX_BRIGHTNESS       160

// Record spin speed at the motor output, in RPM (28BYJ-48 tops out ~15 RPM).
#define RECORD_RPM              12.0f

// Halo: set to 1 only if you soldered the INMP441 mic (audio-reactive glow).
// 0 = progress-driven "breathing" pulse (no extra hardware needed).
#define HALO_USE_MIC            0
#define HALO_LED_COUNT          24
