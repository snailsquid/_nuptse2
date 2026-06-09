#include <Arduino.h>
#include <Wire.h>
#include <unity.h>
#include "MAX30100_PulseOximeter.h"

PulseOximeter pox;
bool sensorFound = false;
float lastBpm = 0;
float lastSpo2 = 0;
unsigned long readings = 0;

void onBeatDetected() {
}

void test_sensor_detection() {
  Wire.begin(21, 22);
  sensorFound = pox.begin();
  TEST_ASSERT_TRUE_MESSAGE(sensorFound, "MAX30100 sensor not found on I2C bus");
}

void test_led_current() {
  if (!sensorFound) return;
  pox.setIRLedCurrent(MAX30100_LED_CURR_27_1MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void test_sensor_readings() {
  if (!sensorFound) {
    TEST_IGNORE_MESSAGE("sensor not detected, skipping reading test");
    return;
  }

  Serial.println("place finger on sensor now...");
  unsigned long start = millis();
  while (millis() - start < 15000) {
    pox.update();
    lastBpm = pox.getHeartRate();
    lastSpo2 = pox.getSpO2();
    if (lastBpm > 0 && lastSpo2 > 0) {
      readings++;
    }
    delay(10);
  }

  Serial.print("readings: ");
  Serial.println(readings);
  TEST_ASSERT_GREATER_THAN_MESSAGE(0, readings, "no valid sensor readings within 15s — hold finger on sensor");
}

void test_bpm_range() {
  if (readings == 0) TEST_IGNORE_MESSAGE("no readings, skipping BPM range check");
  else TEST_ASSERT_GREATER_THAN_MESSAGE(0, lastBpm, "invalid BPM reading");
}

void test_spo2_range() {
  if (readings == 0) TEST_IGNORE_MESSAGE("no readings, skipping SpO2 range check");
  else TEST_ASSERT_GREATER_THAN_MESSAGE(0, lastSpo2, "invalid SpO2 reading");
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_sensor_detection);
  RUN_TEST(test_led_current);
  RUN_TEST(test_sensor_readings);
  RUN_TEST(test_bpm_range);
  RUN_TEST(test_spo2_range);
  UNITY_END();
}

void loop() {
}
