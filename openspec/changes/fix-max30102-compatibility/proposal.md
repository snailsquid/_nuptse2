## Why

The MAX30100lib library by oxullo is being used with a MAX30102 sensor. The sensor works (passes part ID check) but FIFO data is misread — the library assumes 4 bytes/sample (2×16-bit for IR+Red) but MAX30102 stores 6 bytes/sample (2×18-bit, 3 bytes each). This corrupts both IR and Red readings, causing the beat detector to see noise and report BPM that's far too high.

## What Changes

- Modify `MAX30100::readFifoData()` to read 3 bytes per FIFO word instead of 2
- Change sample assembly: 18-bit left-aligned → 16-bit right-shifted by 2
- Increase internal buffer sizes: `RINGBUFFER_SIZE` from 16 to 32 (MAX30102 has deeper FIFO)
- Fix FIFO slot counting: `toRead` reflects words, not samples — halve it in SpO2 mode
- No changes to PulseOximeter, BeatDetector, Filters, or SpO2Calculator — they work on already-assembled values

## Capabilities

### New Capabilities
- `max30102-fifo`: MAX30102 FIFO data reading — 18-bit word assembly, correct slot counting, proper buffer sizing

### Modified Capabilities

*(none — no existing specs)*

## Impact

- **Affected file**: `src/MAX30100.cpp` — `readFifoData()` method only
- **Affected header**: `src/MAX30100.h` — `RINGBUFFER_SIZE` constant
- **No change** to PulseOximeter, BeatDetector, Filters, SpO2Calculator, or `main.cpp`
- Sensor continues to enumerate as-is (same I2C address 0x57, same part ID 0x15)
