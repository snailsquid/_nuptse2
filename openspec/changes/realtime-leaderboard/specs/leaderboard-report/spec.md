## ADDED Requirements

### Requirement: Session-end HTTP POST to leaderboard
The system SHALL POST session results to the leaderboard server when a measurement session completes successfully.

#### Scenario: POST on FINISHED state entry
- **WHEN** the state machine enters the FINISHED state
- **THEN** the system SHALL send an HTTP POST request to the leaderboard server

#### Scenario: POST payload structure
- **WHEN** the system sends a session report
- **THEN** the payload SHALL be JSON containing `device_id` (string), `bpm` (int), `delta` (float), `spo2` (float), `cal_avg` (float), and `ts` (unix millis)

#### Scenario: POST timeout
- **WHEN** the leaderboard server does not respond within 2 seconds
- **THEN** the system SHALL abort the request and log the failure to serial
- **AND** SHALL NOT retry (session data is ephemeral)

#### Scenario: POST failure is non-fatal
- **WHEN** the HTTP POST fails (timeout, connection refused, or any error)
- **THEN** the system SHALL continue normal state machine operation
- **AND** SHALL log the delta to serial as fallback

### Requirement: Leaderboard server URL configuration
The leaderboard server URL SHALL be configurable at compile time.

#### Scenario: Compile-time URL
- **WHEN** building the firmware
- **THEN** the leaderboard server URL SHALL be set via a preprocessor define
- **AND** SHALL use the format `http://<host>:<port>/api/score`
