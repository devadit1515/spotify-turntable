// ─────────────────────────────────────────────────────────────────────────────
//  display_ui.h — everything drawn on the 64×64 RGB matrix.
//
//  Layout (full-bleed album art with an overlaid caption band, à la a phone
//  now-playing screen):
//
//    rows  0–48  album art (full colour)
//    rows 49–63  caption band: art darkened ~25% for legibility
//        row 51  scrolling "Title — Artist" marquee (white)
//    rows 62–63  progress bar (Spotify-green fill on a dim track)
//
//  On a track change the new cover fades in over ~400 ms. Double-buffered, so
//  the text overlay never tears.
// ─────────────────────────────────────────────────────────────────────────────
#pragma once

void displayInit();          // configure + start the panel. Call once in setup().
void displayRenderFrame();   // draw one frame. Call continuously from loop() (core 1).
