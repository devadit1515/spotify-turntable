// ─────────────────────────────────────────────────────────────────────────────
//  halo.h — the WS2812B ring around the record.
//
//  Colour is sampled from the current album art, so the ring glows in the
//  record's own palette. Two behaviours, chosen by HALO_USE_MIC in config.h:
//    0 → "breathing" pulse + rotating comet (no extra hardware)
//    1 → true audio-reactive VU ring from an INMP441 I²S microphone
//
//  Runs as its own core-1 FreeRTOS task.
// ─────────────────────────────────────────────────────────────────────────────
#pragma once

void haloInit();
void haloTask(void *param);
