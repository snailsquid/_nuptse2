## 1. WiFi connectivity

- [x] 1.1 Add `#include <WiFi.h>` and `#define WIFI_SSID` / `#define WIFI_PASS` in main.cpp
- [x] 1.2 Add `WiFi.begin()` in `setup()` with 15-second timeout before proceeding to state machine
- [x] 1.3 Add `lastWifiCheck` timer in `loop()`: check `WiFi.status()` every 5s, call `WiFi.reconnect()` if disconnected
- [x] 1.4 Add device identity: `ESP.getEfuseMac()` formatted as `NUPTSE-%04X%08X`, stored as a global `deviceId` string

## 2. MQTT telemetry to ThingsBoard

- [x] 2.1 Add `PubSubClient` to `platformio.ini` lib_deps
- [x] 2.2 Add `#include <PubSubClient.h>` and `#define TB_TOKEN` / `#define TB_SERVER` / `#define TB_PORT`
- [x] 2.3 Add `PubSubClient mqttClient(wifiClient)` and `WiFiClient wifiClient` globals
- [x] 2.4 Add `mqttClient.setServer()` and `mqttClient.connect()` in `setup()` after WiFi connects
- [x] 2.5 Add `lastMqttPublish` timer in `loop()`: if `millis() - lastMqttPublish >= 100`, build JSON `{"bpm":bpm,"spo2":ESpO2,"state":stateNames[state]}` and call `mqttClient.publish("v1/devices/me/telemetry", payload)`
- [x] 2.6 Add `mqttClient.loop()` call in `loop()` at most every 50ms
- [x] 2.7 Guard all MQTT operations: skip if `!mqttClient.connected()` — never block sensor loop

## 3. HTTP POST to leaderboard on FINISHED

- [x] 3.1 Add `#include <HTTPClient.h>` and `#define LB_URL "http://..."` in main.cpp
- [x] 3.2 On FINISHED state entry: build JSON payload with `device_id`, `bpm`, `delta`, `spo2`, `cal_avg`, `ts`
- [x] 3.3 Call `http.begin(LB_URL)`, `http.POST(payload)`, with 2-second timeout
- [x] 3.4 On success or failure: log outcome to serial; never block state machine transition
- [x] 3.5 Finish with `http.end()` — POST is one-shot per session, no retry

## 4. Verification

- [x] 4.1 Build with PlatformIO — no compilation errors
- [ ] 4.2 Flash to ESP32 and verify: serial shows WiFi connecting, then "WiFi connected" / "MQTT connected" messages
- [ ] 4.3 Verify ThingsBoard dashboard receives telemetry: BPM, SpO2, state updates at 10Hz
- [ ] 4.4 Run a full measurement cycle and verify HTTP POST reaches leaderboard server with correct session data
- [ ] 4.5 Verify offline behavior: disconnect WiFi during measurement — sensor loop continues, telemetry gaps, session report fires when WiFi reconnects
