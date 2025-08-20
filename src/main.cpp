#include <Arduino.h>
#include <AccelStepper.h>
#include <Preferences.h>
#include <TMCStepper.h>

// -------- Pins (your latest mapping) --------
#define ENC1_A 1   // JOG CLK
#define ENC1_B 2   // JOG DT
#define ENC2_A 42  // SPEED CLK
#define ENC2_B 41  // SPEED DT
#define ENC3_A 48  // DIST CLK
#define ENC3_B 47  // DIST DT

#define BTN_RUN 21        // RUN button (to GND)
#define BTN_JOG_HOLD 40   // <-- NEW: jog encoder push (to GND). If flaky, try 40.

#define STEP_PIN 4
#define DIR_PIN 5
#define EN_PIN 15
#define LED_PIN 48 // LED on Freenove Wroom

// -------- TMC2209 UART --------
#define RX_PIN 16
#define TX_PIN 17
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00

HardwareSerial TMCSerial(2);
TMC2209Stepper driver(&TMCSerial, R_SENSE, DRIVER_ADDRESS);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
Preferences prefs;

// ================== Quadrature encoder (robust) ==================
struct QuadEnc {
  uint8_t pinA, pinB;
  uint8_t prev;
  int8_t  qsum;
  bool    reverse;
  uint32_t lastDetentUs;
};

static const int8_t QLUT[16] = {
   0, -1, +1,  0,
  +1,  0,  0, -1,
  -1,  0,  0, +1,
   0, +1, -1,  0
};
static const int DETENT_QUARTERS = 4;
static const uint32_t DETENT_GAP_US = 3000;

void encInit(QuadEnc &e, uint8_t a, uint8_t b, bool rev=false) {
  e.pinA = a; e.pinB = b; e.reverse = rev;
  pinMode(a, INPUT_PULLUP);
  pinMode(b, INPUT_PULLUP);
  uint8_t A = digitalRead(a), B = digitalRead(b);
  e.prev = (A ? 2 : 0) | (B ? 1 : 0);
  e.qsum = 0; e.lastDetentUs = 0;
}
int8_t encUpdate(QuadEnc &e) {
  uint8_t A = digitalRead(e.pinA), B = digitalRead(e.pinB);
  uint8_t curr = (A ? 2 : 0) | (B ? 1 : 0);
  uint8_t idx  = (e.prev << 2) | curr;
  e.prev = curr;
  int8_t q = QLUT[idx];
  if (e.reverse) q = -q;
  if (!q) return 0;
  e.qsum += q;
  if (e.qsum >= DETENT_QUARTERS) {
    e.qsum = 0;
    uint32_t now = micros();
    if (now - e.lastDetentUs >= DETENT_GAP_US) { e.lastDetentUs = now; return +1; }
    return 0;
  }
  if (e.qsum <= -DETENT_QUARTERS) {
    e.qsum = 0;
    uint32_t now = micros();
    if (now - e.lastDetentUs >= DETENT_GAP_US) { e.lastDetentUs = now; return -1; }
    return 0;
  }
  return 0;
}

// ---------------- Instances & params ----------------
QuadEnc encJog, encSpeed, encDist;

// Buttons debounce
const uint32_t BTN_DEBOUNCE_MS = 40;
const uint32_t BTN_MIN_GAP_MS  = 120;
bool btn_run_last = true;
uint32_t btn_run_lastChangeMs = 0;
uint32_t btn_run_lastFiredMs  = 0;

// NEW: jog-hold debounce/state
bool btn_jog_last = true;
uint32_t btn_jog_lastChangeMs = 0;
bool jog_hold_active = false;

// Motion params
int speed_sps      = 1600;
int distance_steps = 1600;
int JOG_PER_DETENT = 10;

unsigned long lastSaveMs = 0;
bool pendingSave = false;
bool runningPreset = false;
uint8_t queuedRuns = 0;

// ---------------- Helpers ----------------
void saveIfNeeded() {
  if (pendingSave && millis() - lastSaveMs > 400) {
    prefs.putInt("speed", speed_sps);
    prefs.putInt("dist",  distance_steps);
    pendingSave = false;
    Serial.printf("[NVS] Saved: speed=%d sps, dist=%d\n", speed_sps, distance_steps);
  }
}

void planOneRun() {
  runningPreset = true;
  stepper.setMaxSpeed(speed_sps);
  stepper.move(distance_steps);
  float secs = (float)abs(distance_steps) / max(1, speed_sps);
  Serial.printf("RUN -> %d steps @ %d sps (~%.2fs)\n", distance_steps, speed_sps, secs);
}

void requestRun() {
  // Don't start preset moves while manually jogging
  if (jog_hold_active) return;
  if (!runningPreset && stepper.distanceToGo() == 0) {
    planOneRun();
  } else {
    if (queuedRuns < 10) queuedRuns++;
    Serial.printf("(queued +1) total queued=%u\n", queuedRuns);
  }
}

void tryDequeueRun() {
  if (queuedRuns > 0) {
    queuedRuns--;
    planOneRun();
  }
}

void jogSteps(int det) {
  if (!det || jog_hold_active) return; // ignore detents while holding jog
  stepper.setMaxSpeed(speed_sps);
  stepper.move(det * JOG_PER_DETENT);
  Serial.printf("JOG %+d det -> %+d steps (speed %d)\n", det, det * JOG_PER_DETENT, speed_sps);
}

// ================== Setup ==================
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  encInit(encJog,   ENC1_A, ENC1_B, false);
  encInit(encSpeed, ENC2_A, ENC2_B, false);
  encInit(encDist,  ENC3_A, ENC3_B, false);

  pinMode(BTN_RUN, INPUT_PULLUP);
  pinMode(BTN_JOG_HOLD, INPUT_PULLUP);   // NEW

  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);

  stepper.setAcceleration(8000);
  stepper.setMaxSpeed(4000);

  prefs.begin("dripctrl", false);
  speed_sps      = prefs.getInt("speed", speed_sps);
  distance_steps = prefs.getInt("dist",  distance_steps);
  Serial.printf("Boot: speed=%d sps, distance=%d steps\n", speed_sps, distance_steps);

  Serial.println("Initializing TMC2209...");
  TMCSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  driver.begin();
  driver.toff(5);
  driver.rms_current(800);
  driver.microsteps(16);
  driver.en_spreadCycle(false);
  driver.pwm_autoscale(true);
  driver.ihold(8);

  digitalWrite(EN_PIN, LOW);
  Serial.println("Ready.");
}

// ================== Loop ==================
void loop() {
  // --- Encoders ---
  int dj = encUpdate(encJog);
  if (dj) jogSteps(dj);

  int ds = encUpdate(encSpeed);
  if (ds) {
    speed_sps += ds * 50;
    if (speed_sps < 100)  speed_sps = 100;
    if (speed_sps > 6000) speed_sps = 6000;
    lastSaveMs = millis(); pendingSave = true;
    Serial.printf("SPEED -> %d sps\n", speed_sps);
  }

  int dd = encUpdate(encDist);
  if (dd) {
    distance_steps += dd * 20;
    if (distance_steps < 10)    distance_steps = 10;
    if (distance_steps > 40000) distance_steps = 40000;
    lastSaveMs = millis(); pendingSave = true;
    Serial.printf("DIST -> %d steps\n", distance_steps);
  }

  // --- RUN button: debounce + queue ---
  bool btn_run_now = (digitalRead(BTN_RUN) == LOW);
  uint32_t nowMs = millis();
  if (btn_run_now != btn_run_last && (nowMs - btn_run_lastChangeMs) > BTN_DEBOUNCE_MS) {
    btn_run_last = btn_run_now;
    btn_run_lastChangeMs = nowMs;
    if (btn_run_now && (nowMs - btn_run_lastFiredMs) > BTN_MIN_GAP_MS) {
      btn_run_lastFiredMs = nowMs;
      requestRun();
    }
  }

  // --- JOG HOLD button (press-and-hold for continuous forward) ---
  bool btn_jog_now = (digitalRead(BTN_JOG_HOLD) == LOW);
  if (btn_jog_now != btn_jog_last && (nowMs - btn_jog_lastChangeMs) > BTN_DEBOUNCE_MS) {
    btn_jog_last = btn_jog_now;
    btn_jog_lastChangeMs = nowMs;

    if (btn_jog_now) {
      // start continuous forward jog
      jog_hold_active = true;
      queuedRuns = 0;            // drop queued moves
      runningPreset = false;     // cancel preset state
      stepper.stop();            // clear any planned move
      stepper.setSpeed(abs(speed_sps)); // steps/sec, positive = forward
      Serial.printf("JOG HOLD: start @ %d sps\n", speed_sps);
    } else {
      // stop continuous jog
      jog_hold_active = false;
      stepper.setSpeed(0);
      Serial.println("JOG HOLD: stop");
    }
  }

  // --- Motion service ---
  if (jog_hold_active) {
    // continuous speed while held
    stepper.runSpeed();
  } else {
    // normal queued/preset moves
    if (stepper.distanceToGo() != 0) {
      stepper.setMaxSpeed(speed_sps);
      stepper.run();
    } else if (runningPreset) {
      runningPreset = false;
      Serial.println("RUN complete");
      tryDequeueRun();
    }
  }

  // --- Save settings after knobs settle ---
  saveIfNeeded();
}
