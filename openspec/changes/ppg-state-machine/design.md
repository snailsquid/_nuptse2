## Context

The current `loop()` is a flat, continuous read-process-print cycle. No state awareness means:
- BPM values before the DC removal filter settles are displayed as real
- No user interaction model — button is defined but unused
- No session concept — readings are displayed but not captured meaningfully
- Finger detection is binary but doesn't gate behavior beyond serial message

The ESP32 has `millis()` for non-blocking timing, GPIO for button/LED, and the MAX30102 streams at 200Hz with FIFO reads in the main loop. The state machine must not miss sensor samples, so the loop body must still drain the FIFO every iteration regardless of state.

## Goals / Non-Goals

**Goals:**
- 5-state machine: idle → calibrate → ready → calculate → finished, with fail as universal interrupt
- Non-blocking: all timing via `millis()`, no `delay()` in state transitions
- Button debounce: reject switch bounce on BTN_PIN (32)
- Edge detection on finger IR threshold for state transitions
- Calibration: 15s free-run, average valid BPMs (30-150)
- Measurement: 60s timed session, button click ignored during calculate
- Delta display: final_bpm - calibration_avg, shown via LED flash + serial
- RGB LED reflects state: idle=red, calibrate=yellow, ready=blue, calculate=green, finished=flash green, fail=flash red
- Serial streaming preserved: raw IR/Red + BPM + state label every sample

**Non-Goals:**
- Not modifying MAX30102 library, beat detector, or SpO2 code
- Not adding persistent storage (no EEPROM/SD logging)
- Not adding Bluetooth or WiFi connectivity
- Not changing the SpO2 calculation pipeline

## Decisions

### Decision 1: Enum + switch pattern
**Choice**: `enum State { IDLE, CALIBRATE, READY, CALCULATE, FINISHED, FAIL }` with a state variable and `switch` in `loop()`.
**Alternatives**: Function pointer table, state struct with virtual methods, table-driven FSM.
**Rationale**: Single-file embedded code. Enum + switch is the simplest, most readable pattern for 6 states. No dynamic dispatch overhead. The compiler optimizes a switch on a small enum to a jump table.

### Decision 2: Non-blocking timing via millis()
**Choice**: All state transitions use `millis()` + start-time variables. No `delay()` calls.
**Alternatives**: Hardware timers, FreeRTOS tasks, delay().
**Rationale**: `delay()` blocks sensor FIFO reads. Hardware timers add complexity for no benefit at 6-state granularity. `millis()` handles 15s, 60s, 1s, and 2s durations easily. The 200Hz sensor loop continues uninterrupted.

### Decision 3: Button debounce — simple counter
**Choice**: Reject button press unless `digitalRead(BTN_PIN) == LOW` for 50ms consecutive.
**Alternatives**: Interrupt-based, Bounce2 library, RC filter.
**Rationale**: The button is polled in the main loop at 200Hz — 50ms is ~10 samples of debounce. No extra library needed. State transitions are not time-critical (human interaction scale).

### Decision 4: Finger edge detection
**Choice**: Track `bool fingerOn` and compare with `ir > FINGER_ON` each loop iteration. Only trigger state transitions on rising/falling edges, not continuous state.
**Alternatives**: Poll-only (re-trigger on every cycle), interrupt on IR threshold.
**Rationale**: Edge detection prevents re-entering states on every loop iteration. Polling is fine at 200Hz — no need for interrupts.

### Decision 5: Calibration — accumulate then average
**Choice**: `calSum += bpm; calCount++` for valid BPMs (30-150) over 15s free-run. At expiry: `calAvg = calSum / calCount`.
**Alternatives**: Sliding window, median filter, exponential moving average.
**Rationale**: The user specified "free run, just once at the end." Simple accumulation matches the requirement exactly. Zero-copy, minimal RAM.

### Decision 6: Delta display
**Choice**: `delta = bpm - calAvg` computed on entry to finished state. Displayed via serial and green flash duration (2s).
**Rationale**: Matches user spec. Delta can be negative (HR lower during measurement than calibration) and that's intentional.

### Decision 7: Sensor FIFO drain in all states
**Choice**: The FIFO drain loop (`check()` / `available()` / `getFIFORaw()`) runs on every `loop()` iteration regardless of state. State-specific behavior controls what happens with the data (display, accumulate, ignore).
**Rationale**: FIFO overflow corrupts sensor timing. At 200Hz, even a few milliseconds of blocked reads drops samples. The sensor should always be drained.

## Risks / Trade-offs

- **[Serial perf]** Streaming raw IR/Red at 200Hz over 115200 baud is ~35% utilization. Adding state labels increases it slightly but stays under 50%. **Mitigation**: Acceptable for debug. Production could reduce serial verbosity.
- **[Floating point]** BPM calculation and calibration average use `double`. ESP32 has hardware FPU, so no issue. **Mitigation**: None needed.
- **[Edge case]** If calibration collects zero valid BPMs (e.g., sensor never outputs 30-150), division by zero on `calSum / calCount`. **Mitigation**: Guard: `calAvg = (calCount > 0) ? calSum / calCount : 0`.
- **[LED flash timing]** 500ms flash interval with 200Hz loop means toggling every 100 iterations. `millis()` is checked every loop. **Mitigation**: Use `% 1000` to keep code simple and non-blocking.
- **[Sensor failure]** If MAX30102 disconnects, IR reads may be 0 — looks like "no finger" and keeps state in idle. **Mitigation**: Acceptable — fail-safe behavior.
