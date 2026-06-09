## Why

The MAX30102 sensor uses 18-bit data words in the FIFO (3 bytes per word, 2 words per sample = 6 bytes), but the library still parses with MAX30100 assumptions. BPM is ~2x actual and SpO2 is noise because the FIFO burst read only fetches half the data per sample.

## What Changes

- Fix FIFO burst read length to `6 * toRead` (was `3 * toRead`)  
- Remove spurious `samples = toRead / 2` — `toRead` is already the sample count
- Set `EXPECTED_PART_ID` to `0x15` (MAX30102, was `0x11`)
- Set `DEFAULT_MODE` to `SPO2_HR` (was `HRONLY`)

## Capabilities

### New Capabilities

- `bpm-accurate`: Correct BPM and SpO2 readings from MAX30102 sensor via proper 18-bit FIFO parsing

### Modified Capabilities

None — this is a driver-level fix, no spec-level requirement changes.

## Impact

- Modified library files: `MAX30100.cpp` (readFifoData), `MAX30100.h` (DEFAULT_MODE, EXPECTED_PART_ID)
- Affects the `MAX30100_Registers.h` constants `FIFO_WORD_BYTES` and `MAX30100_FIFO_DEPTH` (already correct)
- No API change — `PulseOximeter::getHeartRate()` and `getSpO2()` signatures unchanged
