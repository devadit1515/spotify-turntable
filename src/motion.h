// ─────────────────────────────────────────────────────────────────────────────
//  motion.h — the physically spinning record (28BYJ-48 stepper via ULN2003).
//
//  Runs as its own core-1 FreeRTOS task. Spins the record while gState.isPlaying
//  is true, ramping smoothly up to RECORD_RPM on play and easing down to a stop
//  on pause. Coils are de-energised when fully stopped.
// ─────────────────────────────────────────────────────────────────────────────
#pragma once

void motionInit();              // configure the four ULN2003 pins
void motionTask(void *param);   // FreeRTOS task entry point
