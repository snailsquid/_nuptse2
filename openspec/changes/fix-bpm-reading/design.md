## Context

The MAX30102 FIFO stores each sample as two 18-bit words (IR + RED), 3 bytes each = 6 bytes total. The current `readFifoData()`:
1. Reads WR/RD pointers to calculate `toRead` (correct)
2. Burst reads `FIFO_WORD_BYTES * toRead` bytes (correct, equals `3 * toRead`)
3. But then does `samples = toRead / 2` — wrong. `toRead` is already the sample count from the pointer diff.

Result: for N samples, it reads 3N bytes but only parses N/2 samples. Lost samples corrupt the beat detector's DC removal filter, producing garbage BPM/SpO2.

Additionally:
- `EXPECTED_PART_ID` is `0x11` (MAX30100) but MAX30102 returns `0x15` — sensor never detected
- `DEFAULT_MODE` is `HRONLY` — no RED LED data for SpO2

## Goals / Non-Goals

**Goals:**
- Correct BPM and SpO2 readings from MAX30102
- Proper FIFO parsing for 18-bit word format
- Support MAX30102 part ID detection

**Non-Goals:**
- Not rewriting the MAX30100 library from scratch
- Not changing the `PulseOximeter` public API
- Not adding MAX30102-specific features (temperature, proximity)

## Decisions

1. **FIFO sample count**: Remove `samples = toRead / 2`. `toRead` from pointer diff already represents number of 18-bit word pairs (samples) available.
2. **Burst read length**: 6 bytes per sample (3 bytes IR + 3 bytes RED). `FIFO_WORD_BYTES` is already 3.
3. **LED current scaling**: Keep `* 17` multiplier (MAX30102 PA registers use 0-255 scale vs MAX30100's 0-15).
4. **Default mode**: `SPO2_HR` — enables both IR and RED LEDs. Firmware user can switch to `HRONLY` if SpO2 not needed.

## Risks / Trade-offs

- [Risk] Library is a `.pio/libdeps/` dependency — `pio run` may overwrite changes on `--clean`. **Mitigation**: Copied to `lib/` or documented reapply step.
- [Risk] `toRead` could theoretically be 1 (one sample) — `readFifoData` allocates buffer for 32×6=192 bytes min, fine.
- [Trade-off] 18-bit values right-shifted by 2 to fit 16-bit — loses 2 bits of precision, but MAX30100 library originally did this too for 14-bit mode.
