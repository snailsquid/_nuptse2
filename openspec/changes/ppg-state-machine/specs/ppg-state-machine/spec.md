## ADDED Requirements

### Requirement: State machine transitions
The system SHALL implement 5 states: idle, calibrate, ready, calculate, finished. Each with defined transitions based on finger detection, button press, timers.

#### Scenario: idle finger-on transitions to calibrate
- **WHEN** the system is in idle state and finger transitions from off to on
- **THEN** the system SHALL enter calibrate state

#### Scenario: all finger-off except finished transition to fail
- **WHEN** the system is in calibrate, ready, or calculate state and finger transitions from on to off
- **THEN** the system SHALL enter fail state

#### Scenario: finished transition to idle
- **WHEN** the 2s flash completes in finished state
- **THEN** the system SHALL enter idle state

#### Scenario: ready button-press starts calculate
- **WHEN** the system is in ready state, finger is on, and button is clicked
- **THEN** the system SHALL enter calculate state

---

### Requirement: idle state
In idle state, the system SHALL indicate no finger detected and remain passive.

#### Scenario: idle LED is red
- **WHEN** the system is in idle state
- **THEN** the RGB LED SHALL be red

#### Scenario: idle button does nothing
- **WHEN** the system is in idle state and the button is pressed
- **THEN** the system SHALL remain in idle state

#### Scenario: idle serial output
- **WHEN** the system is in idle state
- **THEN** serial SHALL output "No finger" at regular intervals

---

### Requirement: calibrate state
In calibrate state, the system SHALL collect BPM readings for 15 seconds and compute an average. This is a free-run timer — valid BPM values in [30, 150] are accumulated and averaged once at the end.

#### Scenario: calibrate LED is yellow
- **WHEN** the system is in calibrate state
- **THEN** the RGB LED SHALL be yellow

#### Scenario: calibrate button does nothing
- **WHEN** the system is in calibrate state and the button is pressed
- **THEN** the system SHALL remain in calibrate state

#### Scenario: calibrate rejects out-of-range BPM
- **WHEN** a BPM value is below 30 or above 150
- **THEN** that value SHALL be excluded from the average

#### Scenario: calibrate averages valid BPM at end
- **WHEN** the 15-second free-run timer expires
- **THEN** the system SHALL compute the arithmetic mean of all valid BPM values collected during calibration

#### Scenario: calibrate transitions to ready
- **WHEN** the 15-second timer expires
- **THEN** the system SHALL enter ready state

---

### Requirement: ready state
In ready state, the system SHALL wait for the user to place a finger (if lifted) and press the button to start a measurement.

#### Scenario: ready LED is blue
- **WHEN** the system is in ready state
- **THEN** the RGB LED SHALL be blue

#### Scenario: ready serial output
- **WHEN** the system is in ready state
- **THEN** serial SHALL output "Ready" to indicate measurement can begin

---

### Requirement: calculate state
In calculate state, the system SHALL measure BPM for 60 seconds after button press. Button presses during calculate are ignored.

#### Scenario: calculate LED is green
- **WHEN** the system is in calculate state
- **THEN** the RGB LED SHALL be green

#### Scenario: calculate has 60-second timer
- **WHEN** the system enters calculate state
- **THEN** a 60-second timer SHALL start

#### Scenario: calculate ignores subsequent button clicks
- **WHEN** the system is in calculate state and the button is pressed
- **THEN** the system SHALL remain in calculate state

#### Scenario: calculate transitions to finished on timer expiry
- **WHEN** the 60-second timer expires
- **THEN** the system SHALL enter finished state

---

### Requirement: finished state
In finished state, the system SHALL display the delta between calibration average and final BPM for 2 seconds with a flashing green LED, then return to idle.

#### Scenario: finished LED flashes green
- **WHEN** the system is in finished state
- **THEN** the RGB LED SHALL flash green at 500ms intervals for 2 seconds

#### Scenario: finished delta calculation
- **WHEN** the system enters finished state
- **THEN** the delta SHALL be computed as: last recorded BPM minus calibration average BPM (may be negative)

#### Scenario: finished serial output
- **WHEN** the system is in finished state
- **THEN** serial SHALL output the delta value

#### Scenario: finished transitions to idle after flash
- **WHEN** the 2-second flash period completes
- **THEN** the system SHALL enter idle state

---

### Requirement: fail state
The fail state SHALL indicate the user lifted their finger unexpectedly. Displays a 1-second red flash then returns to idle.

#### Scenario: fail LED is flashing red
- **WHEN** the system enters fail state
- **THEN** the RGB LED SHALL flash red at 500ms intervals for 1 second

#### Scenario: fail serial output
- **WHEN** the system is in fail state
- **THEN** serial SHALL output "Fail" or equivalent message

#### Scenario: fail transitions to idle
- **WHEN** the 1-second flash period completes
- **THEN** the system SHALL enter idle state

---

### Requirement: serial debug output
The system SHALL continuously stream PPG data and state information over serial for debugging.

#### Scenario: serial streams during all states
- **WHEN** the system is in any state
- **THEN** raw IR/Red values and current BPM SHALL be output over serial

#### Scenario: serial includes state label
- **WHEN** any serial output is produced
- **THEN** the current state name SHALL be included in the output
