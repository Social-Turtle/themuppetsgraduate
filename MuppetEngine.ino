// MuppetEngine — puppet hat servo controller
// Target: Seeed Studio XIAO RA4M1
//
// Wiring:
//   D9  → servo signal
//   D8  → servo power-gate transistor base (optional)
//
// If power gating is not implemented, leave D8 unconnected — the
// servo will simply be always-powered and the gate write is harmless.

#include <Servo.h>

// ─── Pin assignments ──────────────────────────────────────────────────────
const int PIN_SERVO = D9;
const int PIN_GATE  = D8;  // HIGH = servo motor powered

// ─── Motion tuning ───────────────────────────────────────────────────────
const int  REST_ANGLE      = 135;  // arm-down resting position (degrees)
const int  WAVE_ANGLE      = 45;   // arm-up position (degrees)
const int  WAVE_STEP_MS    = 5;    // ms per degree on the raise/lower sweeps

// ─── Quick-wave tuning (only used when QUICK_WAVE = true) ────────────────
const bool QUICK_WAVE      = true; // raise → one flick at top → lower
const int  QUICK_RANGE     = 20;   // degrees of flick each side of WAVE_ANGLE
const int  QUICK_STEP_MS   = 2;    // ms per degree during quick oscillation

// ─── Behavior tuning ─────────────────────────────────────────────────────
const int   MIN_WAVE_CYCLES   = 3;
const int   MAX_WAVE_CYCLES   = 6;
const float MEAN_INTERVAL_SEC = 11.0f; // mean seconds between events (Poisson rate)

// ─── Globals ─────────────────────────────────────────────────────────────
Servo servo;
unsigned long nextEventMs = 0;

// ─── Motion primitives ───────────────────────────────────────────────────

void sweepTo(int fromAngle, int toAngle, int stepMs) {
  int dir = (toAngle > fromAngle) ? 1 : -1;
  for (int pos = fromAngle; pos != toAngle; pos += dir) {
    servo.write(pos);
    delay(stepMs);
  }
  servo.write(toAngle);
}

// ─── Actions ─────────────────────────────────────────────────────────────

void waveArm(int cycles) {
  digitalWrite(PIN_GATE, HIGH);
  delay(50); // let motor power stabilize before moving

  if (QUICK_WAVE) {
    sweepTo(REST_ANGLE, WAVE_ANGLE, WAVE_STEP_MS); // raise once
    int peakLow  = WAVE_ANGLE + QUICK_RANGE;
    int peakHigh = WAVE_ANGLE - QUICK_RANGE;
    for (int r = 0; r < cycles; r++) {
      sweepTo(WAVE_ANGLE, peakLow,  QUICK_STEP_MS); // flick down
      sweepTo(peakLow,    peakHigh, QUICK_STEP_MS); // swing up
      sweepTo(peakHigh,  WAVE_ANGLE, QUICK_STEP_MS); // settle at centre
    }
    sweepTo(WAVE_ANGLE, REST_ANGLE, WAVE_STEP_MS); // lower once
  } else {
    for (int i = 0; i < cycles; i++) {
      sweepTo(REST_ANGLE, WAVE_ANGLE, WAVE_STEP_MS);
      sweepTo(WAVE_ANGLE, REST_ANGLE, WAVE_STEP_MS);
    }
  }

  servo.write(REST_ANGLE); // ensure we always end down before gating off
  delay(80); // settle at rest before cutting power
  digitalWrite(PIN_GATE, LOW);
}

// ─── Timing ──────────────────────────────────────────────────────────────

// Exponential inter-arrival time for a Poisson process: -ln(U) / lambda
unsigned long poissonDelayMs(float meanSec) {
  float u = (float)random(1, 32767) / 32767.0f; // uniform in (0, 1]
  return (unsigned long)(-log(u) * meanSec * 1000.0f);
}

// ─── Arduino entrypoints ─────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  Serial.println("MuppetEngine starting...");

  pinMode(PIN_GATE, OUTPUT);
  digitalWrite(PIN_GATE, LOW);

  servo.attach(PIN_SERVO);
  servo.write(REST_ANGLE);

  randomSeed(analogRead(A0)); // floating pin for entropy

  unsigned long wait = poissonDelayMs(MEAN_INTERVAL_SEC);
  Serial.print("First event in ");
  Serial.print(wait / 1000.0, 1);
  Serial.println("s");
  nextEventMs = millis() + wait;
}

void loop() {
  if (millis() < nextEventMs) return;

  int cycles = (int)random(MIN_WAVE_CYCLES, MAX_WAVE_CYCLES + 1);
  Serial.print("Wave x");
  Serial.println(cycles);
  waveArm(cycles);

  nextEventMs = millis() + poissonDelayMs(MEAN_INTERVAL_SEC);
}
