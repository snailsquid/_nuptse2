## 1. State machine skeleton

- [x] 1.1 Define `enum State { IDLE, CALIBRATE, READY, CALCULATE, FINISHED, FAIL }` and state variable
- [x] 1.2 Wrap existing FIFO drain + BPM detection in all-state loop (always runs)
- [x] 1.3 Implement `switch(state)` in `loop()` with empty stubs for each state

## 2. Idle state

- [x] 2.1 Set LED red, output "No finger" on serial
- [x] 2.2 Detect finger-on rising edge (IR crossing FINGER_ON threshold): transition to CALIBRATE
- [x] 2.3 Button read: transition to IDLE (noop)

## 3. Calibrate state

- [x] 3.1 Set LED yellow, start 15s free-run timer on entry
- [x] 3.2 Accumulate BPM values in [30, 150] range: `calSum += bpm; calCount++`
- [x] 3.3 On 15s expiry: compute `calAvg = (calCount > 0) ? calSum / calCount : 0`, transition to READY
- [x] 3.4 Serial: output "Calibrating..." + running count

## 4. Ready state

- [x] 4.1 Set LED blue, output "Ready" on serial
- [x] 4.2 On button click (debounced) + finger on: transition to CALCULATE
- [x] 4.3 Finger-off edge: transition to FAIL

## 5. Calculate state

- [x] 5.1 Set LED green, start 60s timer on entry
- [x] 5.2 Stream PPG + BPM + remaining time on serial
- [x] 5.3 Ignore button clicks
- [x] 5.4 On 60s expiry: compute `delta = bpm - calAvg`, transition to FINISHED

## 6. Finished state

- [x] 6.1 Flash LED green at 500ms intervals for 2s
- [x] 6.2 Serial: output "Delta: <value> bpm"
- [x] 6.3 After 2s flash, wait for finger lift-off: transition to IDLE

## 7. Fail state

- [x] 7.1 Flash LED red at 500ms intervals for 2s
- [x] 7.2 Serial: output "Fail"
- [x] 7.3 After 1s: transition to IDLE

## 8. Button debounce

- [x] 8.1 Implement 50ms debounce: button press confirmed only after `digitalRead(BTN_PIN) == LOW` for 50ms consecutive

## 9. Finger edge detection

- [x] 9.1 Track `bool fingerOn` with rising/falling edge detection on IR threshold
- [x] 9.2 Wire edge events into state transition checks

## 10. Verification

- [x] 10.1 Build with PlatformIO — no compilation errors
- [ ] 10.2 Flash to ESP32 and verify: idle (red) → finger on → calibrate (yellow, 15s) → ready (blue) → button → calculate (green, 60s) → finished (flash green, delta) → idle
- [ ] 10.3 Verify finger-off in calibrate/ready/calculate triggers fail (flash red 1s) → idle
- [ ] 10.4 Verify delta is correct (negative when final < calibration, positive when final > calibration)
- [ ] 10.5 Verify calibration rejects BPM outside [30, 150] range
