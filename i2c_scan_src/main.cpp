#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define SENSOR_ADDR 0x57

static const uint8_t REG_NAMES[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0xFE, 0xFF
};
static const char* REG_LABELS[] = {
  "STATUS", "FIFO_DATA", "FIFO_THR_MSB", "FIFO_THR_LSB",
  "FIFO_OVF_CNTR", "FIFO_RD_PTR", "FIFO_WR_PTR", "OVF_COUNTER",
  "FIFO_CONFIG", "MODE_CONFIG", "SPO2_CONFIG", "LED_CONFIG",
  "LED1_PA(RED)", "LED2_PA(IR)", "PILOT_PA", "MULTI_LED_CTRL",
  "TEMP_INTR", "TEMP_FRAC", "REV_ID", "PART_ID"
};

uint8_t readReg(uint8_t dev, uint8_t reg) {
  Wire.beginTransmission(dev);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(dev, (uint8_t)1);
  if (Wire.available()) return Wire.read();
  return 0xFF;
}

void scanBus() {
  Serial.println("\n-- I2C Bus Scan --");
  int count = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  0x%02X", addr);
      if (++count % 8 == 0) Serial.println();
    }
  }
  Serial.printf("\n  Found %d device(s)\n", count);
}

void dumpSensor() {
  Serial.println("\n-- MAX30102 Register Dump --");
  for (int i = 0; i < (int)(sizeof(REG_NAMES)/sizeof(REG_NAMES[0])); i++) {
    uint8_t v = readReg(SENSOR_ADDR, REG_NAMES[i]);
    Serial.printf("  0x%02X  %-16s = 0x%02X (%d)\n",
      REG_NAMES[i], REG_LABELS[i], v, v);
  }
}

void testI2cSpeed() {
  Serial.println("\n-- I2C Speed Test (burst read from FIFO_DATA) --");
  unsigned long t0 = micros();
  for (int i = 0; i < 100; i++) {
    readReg(SENSOR_ADDR, 0x01);
  }
  unsigned long dt = micros() - t0;
  Serial.printf("  100 reads in %lu us (%.1f us/read)\n", dt, dt / 100.0);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n========== MAX30102 I2C Connectivity Test ==========");
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  scanBus();
  dumpSensor();
  testI2cSpeed();
  Serial.println("\n==================== Test Done ====================");
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last > 2000) {
    last = millis();
    uint8_t part = readReg(SENSOR_ADDR, 0xFF);
    uint8_t rev  = readReg(SENSOR_ADDR, 0xFE);
    uint8_t mode = readReg(SENSOR_ADDR, 0x09);
    uint8_t fifo = readReg(SENSOR_ADDR, 0x08);
    uint8_t wr   = readReg(SENSOR_ADDR, 0x06);
    uint8_t rd   = readReg(SENSOR_ADDR, 0x05);
    Serial.printf("[%lu] PART=0x%02X  REV=0x%02X  MODE=0x%02X  FIFO_CFG=0x%02X  WR=%d  RD=%d\n",
      millis(), part, rev, mode, fifo, wr, rd);
  }
}
