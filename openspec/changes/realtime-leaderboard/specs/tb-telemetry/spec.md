## ADDED Requirements

### Requirement: Live MQTT telemetry to ThingsBoard
The system SHALL publish real-time telemetry data to ThingsBoard via MQTT at a fixed 10Hz rate while WiFi is connected.

#### Scenario: Telemetry payload format
- **WHEN** the system publishes MQTT telemetry
- **THEN** the topic SHALL be `v1/devices/me/telemetry`
- **AND** the payload SHALL be a JSON object containing `bpm` (int), `spo2` (float), and `state` (string)

#### Scenario: 10Hz publish rate
- **WHEN** the system is running and WiFi is connected
- **THEN** MQTT telemetry SHALL be published every 100ms (±20ms jitter)

#### Scenario: MQTT connect with access token
- **WHEN** the system connects to the ThingsBoard MQTT broker
- **THEN** it SHALL authenticate using a ThingsBoard device access token as the MQTT username

#### Scenario: No data loss from network blocking
- **WHEN** the MQTT connection is unavailable
- **THEN** the system SHALL skip the publish without blocking the sensor loop

### Requirement: WiFi connectivity management
The system SHALL connect to WiFi at startup and automatically recover from disconnection.

#### Scenario: WiFi connects at boot
- **WHEN** the system starts
- **THEN** it SHALL attempt to connect to WiFi with configured SSID and password before entering the state machine

#### Scenario: WiFi is optional for operation
- **WHEN** WiFi fails to connect at startup
- **THEN** the system SHALL proceed with the state machine anyway (offline mode)

#### Scenario: Automatic WiFi reconnect
- **WHEN** WiFi disconnects during operation
- **THEN** the system SHALL attempt to reconnect on a 5-second interval timer
- **AND** SHALL NOT block the sensor FIFO drain during reconnection

### Requirement: Device identity
The system SHALL use its MAC address as the unique device identifier.

#### Scenario: Device ID format
- **WHEN** any network request is made
- **THEN** the device ID SHALL be derived from `ESP.getEfuseMac()`
- **AND** SHALL be formatted as `NUPTSE-<hex>`
