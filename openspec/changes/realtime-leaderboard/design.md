## Context

The ESP32 runs a 200Hz sensor loop (FIFO drain → BPM/SpO2 → state machine → serial). The `loop()` function is synchronous and must complete every 5ms to avoid FIFO overflow. Currently there is zero networking — only serial output.

Adding WiFi + MQTT + HTTP introduces blocking operations that could stall the sensor loop. This design must ensure the 200Hz sensor cycle is never interrupted.

The MAX30102 FIFO holds 32 samples. At 200Hz, a full FIFO represents ~160ms of data. A stall exceeding ~150ms causes sample loss. WiFi reconnect can take 2-10 seconds.

## Goals / Non-Goals

**Goals:**
- WiFi connects in `setup()` before entering the state machine
- MQTT publish to ThingsBoard every 100ms: `{ bpm, spo2, state, delta }`
- HTTP POST to leaderboard on FINISHED state entry (one per session)
- Device identity derived from ESP32 MAC address
- Automatic WiFi reconnection if link drops
- Zero sensor sample loss — network ops never block FIFO drain
- Existing state machine, button, LED, serial unchanged

**Non-Goals:**
- Not implementing the leaderboard server itself (separate concern)
- Not adding OTA, web server, Bluetooth, or any other network feature
- Not encrypting MQTT (TLS adds ~100KB flash, use plain TCP on local network)
- Not storing WiFi credentials in NVS (compile-time defines for now)
- Not handling MQTT QoS > 0 (fire-and-forget telemetry is acceptable)

## Decisions

### Decision 1: WiFi in setup(), reconnect on millis() timer
**Choice**: `WiFi.begin()` in `setup()` with a 15-second timeout. If it fails, the device still enters the state machine and operates offline. In `loop()`, a `millis()` timer checks WiFi status every 5 seconds. If disconnected, it calls `WiFi.reconnect()` — non-blocking, returns immediately. The MQTT publish simply skips if WiFi is down.
**Alternatives**: Blocking reconnect loop (stalls sensor), FreeRTOS task (overkill for one sensor).
**Rationale**: WiFi can take seconds to reconnect. Blocking the 200Hz loop for that long drops every sample in the FIFO. A lazy reconnect approach means brief gaps in telemetry rather than corrupted measurements.

### Decision 2: PubSubClient for MQTT — sync but fast
**Choice**: `PubSubClient` library (standard Arduino). Calls `client.publish()` directly inside the 100ms timer. Average publish latency for a 100-byte JSON payload over WiFi is <5ms. `client.loop()` is called every iteration but filtered to run at most every 50ms.
**Alternatives**: AsyncMqttClient (interrupt-driven, harder to debug), MQTT via ESP-NOW (different protocol).
**Rationale**: PubSubClient is simple, well-tested, and at 100ms intervals (not every 5ms loop iteration), the ~5ms publish time is acceptable. The 50ms `loop()` filter keeps the TCP stack alive without hammering it.

### Decision 3: HTTP POST via WiFiClient + HTTPClient
**Choice**: Arduino `HTTPClient` library. Called synchronously on entry to FINISHED state. A 2-second timeout prevents hanging if the server is unreachable. The POST happens once per 60s measurement — blocking for up to 2s is acceptable since the measurement is complete.
**Alternatives**: AsyncHTTPRequest (not needed), raw WiFiClient (more code).
**Rationale**: Session-end POST happens once per minute at most. A 500ms HTTP round-trip is fine. The sensor loop won't be running during the POST because the session is over.

### Decision 4: Device identity from MAC address
**Choice**: `uint64_t chipid = ESP.getEfuseMac()` formatted as `NUPTSE-%04X%08X`. No user configuration needed. Unique per chip.
**Alternatives**: Compile-time string, NVS storage, WiFi AP name.
**Rationale**: Zero setup. Guaranteed unique. Leaderboard can map friendly names on its side.

### Decision 5: Rate-limited MQTT publish timer
**Choice**: A `unsigned long lastMqttPublish = 0` timestamp. On every `loop()` iteration, if `millis() - lastMqttPublish >= 100`, build JSON and call `client.publish()`. This ensures exactly 10Hz telemetry regardless of loop timing jitter.
**Alternatives**: Publish on every SAMPLING tick (variable rate), FreeRTOS timer.
**Rationale**: Simple, deterministic, no extra dependencies. Matches the 100ms interval the user specified.

### Decision 6: ThingsBoard topic and payload format
**Choice**: Topic `v1/devices/me/telemetry` with JSON payload `{"bpm":72,"spo2":98.2,"state":"CALCULATE"}`. Authentication via access token in the MQTT connect call. No timestamp field — ThingsBoard auto-stamps on arrival.
**Alternatives**: `v1/gateway/telemetry` (gateway mode, not needed for single device).
**Rationale**: Standard ThingsBoard device telemetry format. Simplest integration path.

## Risks / Trade-offs

- **[FIFO overflow during WiFi reconnect]** If WiFi drops and `WiFi.reconnect()` takes 5s, the FIFO overflows (~800 samples lost). **Mitigation**: MQTT publish simply skips when WiFi is down. No FIFO stall. Gap in telemetry is acceptable.
- **[MQTT publish latency variability]** At peak, JSON serialization + `client.publish()` could spike to 15ms. **Mitigation**: At 10Hz, there's 85ms of slack in each 100ms window. Only the publish iteration stalls slightly; subsequent iterations recover.
- **[Unreachable leaderboard server]** HTTP POST with 2s timeout blocks the loop. **Mitigation**: The session is already complete — no sensor data is being read during FINISHED state. The 2s timeout prevents permanent hang. Delta is logged to serial as fallback.
- **[Flash/RAM usage]** PubSubClient + WiFi stack adds ~120KB flash and ~15KB RAM. **Mitigation**: ESP32 has 4MB flash / 520KB RAM — plenty of headroom. Confirm with `pio run --environment esp32dev`.
- **[SSID/password in source]** Stored as `#define WIFI_SSID "..."` and `#define WIFI_PASS "..."` in main.cpp. **Mitigation**: Acceptable for personal device. Could use `platformio.ini` build flags or NVS for production.
