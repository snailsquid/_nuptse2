#include <Wire.h>
#include "MAX30105.h"
#include "display.h"
MAX30105 particleSensor;

double avered    = 0;
double aveir     = 0;
double sumirrms  = 0;
double sumredrms = 0;
int    i         = 0;
int    Num       = 100;
int    Temperature;
int    temp;
float  ESpO2;
double FSpO2     = 0.7;
double frate     = 0.95;

unsigned long lastBeatTime = 0;
int    bpm       = 0;
float  irPrevAC  = 0;
#define TIMETOBOOT 3000
#define SCALE      88.0
#define SAMPLING   100
#define FINGER_ON  30000
#define USEFIFO
#define LED_R      26
#define LED_G      27
#define LED_B      25
#define BTN_PIN    32

State state = IDLE;

unsigned long stateEntryTime = 0;
unsigned long lastFlashToggle = 0;
bool flashLedOn = false;

double calSum = 0;
int calCount = 0;
double calAvg = 0;
unsigned long calculateStartTime = 0;
double delta = 0;

unsigned long btnDebounceStart = 0;
bool btnDebounceActive = false;

bool fingerOnPrev = false;

static const char* stateNames[] = { "IDLE", "CALIBRATE", "READY", "CALCULATE", "FINISHED", "FAIL" };


void setLED(bool r, bool g, bool b) {
    digitalWrite(LED_R, r ? LOW : HIGH);
    digitalWrite(LED_G, g ? LOW : HIGH);
    digitalWrite(LED_B, b ? LOW : HIGH);
}

void transitionTo(State newState) {
    state = newState;
    stateEntryTime = millis();
    lastFlashToggle = 0;
    flashLedOn = false;
    btnDebounceActive = false;

    if (newState == CALIBRATE) {
        calSum = 0;
        calCount = 0;
    }
    if (newState == CALCULATE) {
        calculateStartTime = stateEntryTime;
    }
    if (newState == FINISHED) {
        btnDebounceActive = false;
    }
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  setLED(0, 0, 0);

  Serial.println("Running...");
  Serial.println("LED test: R");
  setLED(1, 0, 0); delay(500);
  Serial.println("LED test: G");
  setLED(0, 1, 0); delay(500);
  Serial.println("LED test: B");
  setLED(0, 0, 1); delay(500);
  Serial.println("LED test: Y");
  setLED(1, 1, 0); delay(500);
  setLED(0, 0, 0);
  delay(1000);

  while (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30102 was not found. Please check wiring/power/solder jumper at MH-ET LIVE MAX30102 board. ");
  }

  byte ledBrightness = 0x7F;
  byte sampleAverage = 4;
  byte ledMode       = 2;
  int sampleRate     = 200;
  int pulseWidth     = 411;
  int adcRange       = 16384;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.enableDIETEMPRDY();

  initDisplay();
  transitionTo(IDLE);
}

void loop()
{
  static uint32_t ir = 0, red = 0;
  double fred, fir;
  double SpO2 = 0;
  particleSensor.check();

  while (particleSensor.available()) {
    red = particleSensor.getFIFORed();
    ir  = particleSensor.getFIFOIR();

    i++;
    fred = (double)red;
    fir  = (double)ir;
    avered = avered * frate + (double)red * (1.0 - frate);
    aveir = aveir * frate + (double)ir * (1.0 - frate);
    sumredrms += (fred - avered) * (fred - avered);
    sumirrms += (fir - aveir) * (fir - aveir);

    double irAC = fir - aveir;
    if (irPrevAC < 0 && irAC >= 0) {
      unsigned long now = millis();
      if (lastBeatTime != 0) {
        unsigned long interval = now - lastBeatTime;
        bpm = (interval > 0) ? (60000.0 / interval) : 0;
      }
      lastBeatTime = now;
    }
    irPrevAC = (float)irAC;

    if ((i % SAMPLING) == 0) {
      if (millis() > TIMETOBOOT) {
        float ir_forGraph = (2.0 * fir - aveir) / aveir * SCALE;
        float red_forGraph = (2.0 * fred - avered) / avered * SCALE;
        if (ir_forGraph > 100.0) ir_forGraph = 100.0;
        if (ir_forGraph < 80.0) ir_forGraph = 80.0;
        if (red_forGraph > 100.0) red_forGraph = 100.0;
        if (red_forGraph < 80.0) red_forGraph = 80.0;

        Serial.print("["); Serial.print(stateNames[state]); Serial.print("] ");
        Serial.print("Red: "); Serial.print(red); Serial.print(","); Serial.print("Infrared: "); Serial.print(ir); Serial.print(".    ");
        Serial.print("BPM = "); Serial.print(bpm);

        if (state == IDLE) {
          Serial.print("    No finger");
        } else if (state == CALIBRATE) {
          Serial.print("    Calibrating... count: "); Serial.print(calCount);
        } else if (state == READY) {
          Serial.print("    Ready");
        } else if (state == CALCULATE) {
          unsigned long elapsed = (millis() - calculateStartTime) / 1000;
          if (elapsed < 60) {
            Serial.print("    Time: "); Serial.print(60 - elapsed); Serial.print("s");
          }
        } else if (state == FINISHED) {
          Serial.print("    Delta: "); Serial.print(delta); Serial.print(" bpm");
        } else if (state == FAIL_STATE) {
          Serial.print("    Fail");
        }
        Serial.println();
      }
    }

    if ((i % Num) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
      SpO2 = -23.3 * (R - 0.4) + 100;
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;
      sumredrms = 0.0; sumirrms = 0.0; i = 0;
      break;
    }
    particleSensor.nextSample();
  }

  unsigned long now = millis();

  bool fingerOn = (ir > FINGER_ON);
  bool fingerRising = fingerOn && !fingerOnPrev;
  bool fingerFalling = !fingerOn && fingerOnPrev;
  fingerOnPrev = fingerOn;

  bool btnReading = (digitalRead(BTN_PIN) == LOW);
  bool btnClicked = false;

  if (btnReading) {
    if (!btnDebounceActive) {
      btnDebounceActive = true;
      btnDebounceStart = now;
    } else if (now - btnDebounceStart >= 50) {
      btnClicked = true;
      btnDebounceActive = false;
    }
  } else {
    btnDebounceActive = false;
  }

  switch (state) {
    case IDLE:
      setLED(1, 0, 0);
      if (fingerRising) {
        transitionTo(CALIBRATE);
      }
      break;

    case CALIBRATE:
      setLED(1, 1, 0);
      if (bpm >= 30 && bpm <= 150) {
        calSum += bpm;
        calCount++;
      }
      if (now - stateEntryTime >= 15000) {
        calAvg = (calCount > 0) ? calSum / calCount : 0;
        transitionTo(READY);
      }
      if (fingerFalling) {
        transitionTo(FAIL_STATE);
      }
      break;

    case READY:
      setLED(0, 0, 1);
      if (btnClicked && fingerOn) {
        transitionTo(CALCULATE);
      }
      if (fingerFalling) {
        transitionTo(FAIL_STATE);
      }
      break;

    case CALCULATE:
      setLED(0, 1, 0);
      if (now - stateEntryTime >= 60000) {
        delta = bpm - calAvg;
        transitionTo(FINISHED);
      }
      if (fingerFalling) {
        transitionTo(FAIL_STATE);
      }
      break;

    case FINISHED:
      if (now - stateEntryTime < 5000) {
        if (now - lastFlashToggle >= 500) {
          flashLedOn = !flashLedOn;
          lastFlashToggle = now;
          setLED(0, flashLedOn, 0);
        }
      } else if (btnClicked) {
        transitionTo(IDLE);
      }
      break;

    case FAIL_STATE:
      if (now - stateEntryTime < 2000) {
        if (now - lastFlashToggle >= 250) {
          flashLedOn = !flashLedOn;
          lastFlashToggle = now;
          setLED(flashLedOn, 0, 0);
        }
      } else {
        transitionTo(IDLE);
      }
      break;
  }

  updateDisplay(state, bpm, delta, calculateStartTime, calCount);
}
