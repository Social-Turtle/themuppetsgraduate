// MuppetEngine — puppet hat servo controller
// Target: Seeed Studio XIAO RA4M1
//
// Wiring:
//   D9  → left servo signal
//   D10 → right servo signal
//   D8  → left servo power-gate transistor base (optional)
//   D7  → right servo power-gate transistor base (optional)
//
// If power gating is not implemented, leave D7/D8 unconnected — the
// servos will simply be always-powered and the gate writes are harmless.

#include <Servo.h>

// ─── Pin assignments ──────────────────────────────────────────────────────
const int PIN_SERVO_LEFT  = D9;
const int PIN_SERVO_RIGHT = D10;
const int PIN_GATE_LEFT   = D8;  // HIGH = servo motor powered
const int PIN_GATE_RIGHT  = D7;

// ─── Motion tuning ───────────────────────────────────────────────────────
const int   REST_ANGLE        = 15;   // arm-down resting position (degrees)
const int   WAVE_ANGLE        = 160;  // arm-up position (degrees)
const int   WAVE_STEP_MS      = 0;   // ms per degree during a normal wave
const int   SPAZ_HOLD_MS      = 80;   // ms to hold each extreme during spaz

// ─── Behavior tuning ─────────────────────────────────────────────────────
const int   MIN_WAVE_CYCLES   = 2;
const int   MAX_WAVE_CYCLES   = 5;
const int   SPAZ_CYCLES       = 15;
const int   SPAZ_CHANCE_PCT   = 5;   // % chance of spaz instead of single-arm wave
const float MEAN_INTERVAL_SEC = 8.0f; // mean seconds between events (Poisson rate)

// ─── Globals ─────────────────────────────────────────────────────────────
Servo servoLeft;
Servo servoRight;
unsigned long nextEventMs = 0;

// ─── Motion primitives ───────────────────────────────────────────────────

void sweepTo(Servo &srv, int fromAngle, int toAngle, int stepMs) {
  int dir = (toAngle > fromAngle) ? 1 : -1;
  for (int pos = fromAngle; pos != toAngle; pos += dir) {
    srv.write(pos);
    delay(stepMs);
  }
  srv.write(toAngle);
}

// ─── Actions ─────────────────────────────────────────────────────────────

void waveArm(bool leftArm, int cycles) {

  int   gatePin = leftArm ? PIN_GATE_LEFT  : PIN_GATE_RIGHT;
  Servo &srv    = leftArm ? servoLeft      : servoRight;
  digitalWrite(gatePin, HIGH);
  delay(50); // let motor power stabilize before moving

  for (int i = 0; i < cycles; i++) {
    sweepTo(srv, REST_ANGLE, WAVE_ANGLE, WAVE_STEP_MS);
    sweepTo(srv, WAVE_ANGLE, REST_ANGLE, WAVE_STEP_MS);
  }

  delay(80); // settle at rest before cutting power
  digitalWrite(gatePin, LOW);
}

void spazArms() {
  digitalWrite(PIN_GATE_LEFT,  HIGH);
  digitalWrite(PIN_GATE_RIGHT, HIGH);
  delay(50);

  for (int i = 0; i < SPAZ_CYCLES; i++) {
    servoLeft.write(WAVE_ANGLE);
    servoRight.write(WAVE_ANGLE);
    delay(SPAZ_HOLD_MS);
    servoLeft.write(REST_ANGLE);
    servoRight.write(REST_ANGLE);
    delay(SPAZ_HOLD_MS);
  }

  delay(80);
  digitalWrite(PIN_GATE_LEFT,  LOW);
  digitalWrite(PIN_GATE_RIGHT, LOW);
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

  pinMode(PIN_GATE_LEFT,  OUTPUT);
  pinMode(PIN_GATE_RIGHT, OUTPUT);
  digitalWrite(PIN_GATE_LEFT,  LOW);
  digitalWrite(PIN_GATE_RIGHT, LOW);

  servoLeft.attach(PIN_SERVO_LEFT);
  servoLeft.write(REST_ANGLE);
  servoRight.attach(PIN_SERVO_RIGHT);
  servoRight.write(REST_ANGLE);

  randomSeed(analogRead(A0)); // floating pin for entropy

  unsigned long wait = poissonDelayMs(MEAN_INTERVAL_SEC);
  Serial.print("First event in ");
  Serial.print(wait / 1000.0, 1);
  Serial.println("s");
  nextEventMs = millis() + wait;
}

void loop() {
  if (millis() < nextEventMs) return;

  if (random(100) < SPAZ_CHANCE_PCT) {
    spazArms();
  } else {
    bool leftArm = (random(2) == 0);
    int  cycles  = (int)random(MIN_WAVE_CYCLES, MAX_WAVE_CYCLES + 1);
    waveArm(leftArm, cycles);
  }

  nextEventMs = millis() + poissonDelayMs(MEAN_INTERVAL_SEC);
}
