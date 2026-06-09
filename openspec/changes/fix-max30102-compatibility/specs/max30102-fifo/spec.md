## ADDED Requirements

### Requirement: MAX30102 FIFO word assembly
The system SHALL read 3 bytes per FIFO word when using a MAX30102 sensor, instead of 2 bytes per word.

#### Scenario: Word read size
- **WHEN** reading from the FIFO data register
- **THEN** each word SHALL be assembled from 3 bytes (18-bit left-aligned)

#### Scenario: 16-bit conversion
- **WHEN** converting an 18-bit left-aligned word to 16-bit
- **THEN** the value SHALL be right-shifted by 2 bits

### Requirement: Correct slot counting
The system SHALL correctly compute the number of samples available in FIFO, accounting for the word-to-sample ratio in SpO2 mode (2 words per sample).

#### Scenario: FIFO available calculation
- **WHEN** checking FIFO write/read pointers
- **THEN** the available sample count SHALL be (WRITE_PTR - READ_PTR) / 2 for SpO2 mode
- **THEN** the total bytes to read SHALL be available_samples * 6 (3 bytes × 2 channels)

### Requirement: Adequate buffer sizing
The system SHALL use buffers large enough to hold all samples from a full MAX30102 FIFO (32 samples vs 16 in MAX30100).

#### Scenario: Buffer capacity
- **WHEN** the FIFO is full with 32 samples in SpO2 mode
- **THEN** the readouts buffer SHALL accommodate at least 32 entries without overflow
- **THEN** the burst read buffer SHALL accommodate at least 192 bytes (32 × 6)

### Requirement: Backward compatibility note
The system SHALL NOT break compatibility with MAX30100 sensors. All changes SHALL be conditional on detecting word-size mismatch or SHALL work correctly for both 4-byte and 6-byte sample formats.

#### Scenario: MAX30100 still works
- **WHEN** a MAX30100 sensor is connected
- **THEN** the system SHALL continue to read 4-byte samples correctly
