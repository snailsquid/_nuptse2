#include <Wire.h>
#include "MAX30105.h"
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


void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  Serial.println("Running...");
  delay(3000);

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
}

void loop()
{

  uint32_t ir, red, green;
  double fred, fir;
  double SpO2 = 0;

#ifdef USEFIFO
  particleSensor.check();

  while (particleSensor.available()) {
#ifdef MAX30105
   red = particleSensor.getFIFORed();
   ir  = particleSensor.getFIFOIR();
#else
   red = particleSensor.getFIFOIR();
   ir  = particleSensor.getFIFORed();
#endif

    i++;
    fred = (double)red;
    fir  = (double)ir;
    avered = avered * frate + (double)red * (1.0 - frate);
    aveir = aveir * frate + (double)ir * (1.0 - frate);
    sumredrms += (fred - avered) * (fred - avered);
    sumirrms += (fir - aveir) * (fir - aveir);

    // Heartbeat detection via IR AC zero-crossing
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
      if ( millis() > TIMETOBOOT) {
        float ir_forGraph = (2.0 * fir - aveir) / aveir * SCALE;
        float red_forGraph = (2.0 * fred - avered) / avered * SCALE;
        if ( ir_forGraph > 100.0) ir_forGraph = 100.0;
        if ( ir_forGraph < 80.0) ir_forGraph = 80.0;
        if ( red_forGraph > 100.0 ) red_forGraph = 100.0;
        if ( red_forGraph < 80.0 ) red_forGraph = 80.0;
        Serial.print("Red: "); Serial.print(red); Serial.print(","); Serial.print("Infrared: "); Serial.print(ir); Serial.print(".    ");

        if (ir < FINGER_ON){
           Serial.println("No finger detected");
           break;
        }
        if(ir > FINGER_ON){
           Serial.print("Oxygen % = ");
           Serial.print(ESpO2);
           Serial.print("%    ");
           Serial.print("BPM = ");
           Serial.println(bpm);
        }
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
#endif

}
