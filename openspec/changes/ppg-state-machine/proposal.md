## Why

The current main.cpp is a flat loop that continuously streams BPM readings with no state management. There's no calibration, no user interaction model, and no way to get a clean measurement. Adding a 5-state machine with button control, finger detection, calibration, and a timed measurement session makes this a usable pulse oximeter rather than a raw sensor firehose.

## What Changes

- Replace flat `loop()` with a 5-state state machine: `idle`, `calibrate`, `ready`, `calculate`, `finished`
- Add a `fail` state for unexpected finger-off
- Button input on pin 32 triggers `calculate` transition (from `ready` with finger on)
- RGB LED (pins 25/26/27) reflects current state: red, yellow, blue, green, flash green, flash red
- 15-second free-run calibration in `calibrate` state, averaging BPM values in [30, 150] range
- 60-second timed measurement in `calculate` state
- Delta displayed on serial and LED flash at end: `final_bpm - calibration_avg`
- Continuous serial streaming preserved for debugging

## Capabilities

### New Capabilities
- `ppg-state-machine`: 5-state finger oximeter state machine with calibration, button trigger, timed measurement, LED feedback, and fail handling

### Modified Capabilities

*(none)*

## Impact

- **main.cpp**: Complete rewrite of `loop()`, restructured state machine replaces flat read-process-print cycle
- **No change** to sensor library (MAX30102), filters, beat detector, or SpO2 calculator
- Serial output format changes: state labels added, delta shown at finish
