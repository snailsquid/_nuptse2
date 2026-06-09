## ADDED Requirements

### Requirement: BPM reading within ±5 bpm of reference

The system SHALL report heart rate within ±5 bpm of a reference pulse oximeter when a static finger is placed on the MAX30102 sensor.

#### Scenario: Correct BPM with stationary finger
- **WHEN** a finger is placed on the sensor and held still for 10 seconds
- **THEN** `getHeartRate()` returns a value between 50 and 150
- **AND** the value stays within ±5 bpm of the previous reading

#### Scenario: BPM returns 0 when no finger
- **WHEN** no finger is on the sensor
- **THEN** `getHeartRate()` returns 0.0

### Requirement: SpO2 reading within ±3% of reference

The system SHALL report SpO2 within ±3% of a reference pulse oximeter when a static finger is placed on the sensor.

#### Scenario: Correct SpO2 with stationary finger
- **WHEN** a finger is placed on the sensor and held still for 10 seconds
- **THEN** `getSpO2()` returns a value between 92 and 100

#### Scenario: SpO2 returns 0 when no finger
- **WHEN** no finger is on the sensor
- **THEN** `getSpO2()` returns 0

### Requirement: No spurious beats from sensor noise

The system SHALL NOT report heartbeats faster than 220 bpm from vibration or sensor noise.

#### Scenario: No false beats on desk
- **WHEN** the device is resting on a desk with no finger on the sensor
- **THEN** `getHeartRate()` returns 0.0 after 5 seconds of settling

### Requirement: Sample loss below 1%

The system SHALL consume FIFO data at a rate exceeding the sensor sampling rate so that the FIFO overflow counter stays at zero during normal operation.

#### Scenario: No overflow during stable reading
- **WHEN** finger is stationary on the sensor
- **THEN** the `OVF_COUNTER` register stays at 0 across consecutive reads spaced 100ms apart
