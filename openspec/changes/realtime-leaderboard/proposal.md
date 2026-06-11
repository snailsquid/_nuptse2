## Why

The pulse oximeter currently has no network connectivity — all data is serial-only. Adding WiFi enables live BPM/SpO2 streaming to ThingsBoard for real-time dashboards and a session-end HTTP POST to a leaderboard server for ranked delta scores.

## What Changes

- Add WiFi connectivity to ESP32 (SSID/password via config or compile-time defines)
- Add MQTT publish to ThingsBoard every 100ms: BPM, SpO2, current state
- Add HTTP POST to leaderboard server on FINISHED state entry: device_id, session metrics, delta
- Keep existing state machine, sensor loop, and serial output untouched
- Non-blocking network operations — FIFO drain must never stall

## Capabilities

### New Capabilities
- `tb-telemetry`: Live MQTT telemetry stream to ThingsBoard (BPM, SpO2, state at 10Hz)
- `leaderboard-report`: HTTP POST of session results to leaderboard server at session end
- `wifi-connectivity`: WiFi initialization, connection management, and automatic reconnection

### Modified Capabilities

*(none)*

## Impact

- **main.cpp**: Add WiFi init in `setup()`, MQTT publish in `loop()` (timed every 100ms), HTTP POST on FINISHED entry
- **platformio.ini**: Add `PubSubClient` library dependency
- **No change** to state machine, MAX30102 sensor code, beat detector, or SpO2 calculator
- Device identity derived from ESP32 MAC address (no user config needed)
