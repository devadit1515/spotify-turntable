// ─────────────────────────────────────────────────────────────────────────────
//  album_art.h — download + decode the current track's cover into gArt.
//
//  Called from the Spotify poll task (core 0) whenever the track changes.
//  Downloads the JPEG over HTTPS, decodes it (in blocks, low-RAM) into a 64×64
//  RGB565 buffer, then swaps it into gArt under gArtMutex and bumps gArtVersion.
// ─────────────────────────────────────────────────────────────────────────────
#pragma once

// Returns true if new art was decoded and published to gArt.
// On any failure the previous gArt is left untouched.
bool albumArtFetch(const char *url);
