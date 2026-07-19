#include "motion.h"
#include "pins.h"
#include "config.h"
#include "playback_state.h"

// 28BYJ-48 half-step sequence (8 states). Half-stepping is smoother and quieter
// than full-stepping — worth it for a display piece. ~4096 half-steps/rev after
// the 1:64 internal gearbox.
static const uint8_t HALF_STEP[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1},
};
static const float STEPS_PER_REV = 4096.0f;

static void writePhase(int phase) {
  digitalWrite(PIN_MOTOR_IN1, HALF_STEP[phase][0]);
  digitalWrite(PIN_MOTOR_IN2, HALF_STEP[phase][1]);
  digitalWrite(PIN_MOTOR_IN3, HALF_STEP[phase][2]);
  digitalWrite(PIN_MOTOR_IN4, HALF_STEP[phase][3]);
}

static void coilsOff() {
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, LOW);
  digitalWrite(PIN_MOTOR_IN4, LOW);
}

void motionInit() {
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  pinMode(PIN_MOTOR_IN3, OUTPUT);
  pinMode(PIN_MOTOR_IN4, OUTPUT);
  coilsOff();
}

void motionTask(void *param) {
  const float topSpeed = (RECORD_RPM / 60.0f) * STEPS_PER_REV;   // steps/sec at full RPM
  const float ramp     = topSpeed / 0.8f;                        // reach top in ~0.8 s

  float    curSpeed = 0.0f;   // steps/sec, eased toward target
  float    accum    = 0.0f;   // fractional steps owed
  int      phase    = 0;
  bool     coilsLive = false;
  uint32_t last     = micros();

  for (;;) {
    // Non-blocking peek at play state; if the mutex is busy just keep our pace.
    bool playing = (curSpeed > 0.0f);
    if (gStateMutex && xSemaphoreTake(gStateMutex, 0) == pdTRUE) {
      playing = gState.isPlaying && gState.valid;
      xSemaphoreGive(gStateMutex);
    }

    uint32_t now = micros();
    float dt = (now - last) / 1e6f;
    last = now;
    if (dt > 0.05f) dt = 0.05f;   // clamp after any scheduling hiccup

    // Ease current speed toward the target (top when playing, 0 when paused).
    float target = playing ? topSpeed : 0.0f;
    if      (curSpeed < target) curSpeed = min(curSpeed + ramp * dt, target);
    else if (curSpeed > target) curSpeed = max(curSpeed - ramp * dt, target);

    if (curSpeed > 0.5f) {
      accum += curSpeed * dt;
      int guard = 0;
      while (accum >= 1.0f && guard++ < 32) {   // guard: never emit a huge burst
        phase = (phase + 1) & 7;
        writePhase(phase);
        coilsLive = true;
        accum -= 1.0f;
      }
    } else if (coilsLive) {
      coilsOff();                 // fully stopped → release the coils (cool/quiet)
      coilsLive = false;
      accum = 0.0f;
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
