## 1. Constants and Buffers

- [x] 1.1 Increase `RINGBUFFER_SIZE` from 16 to 32 in `MAX30100.h`
- [x] 1.2 Define `FIFO_WORD_BYTES` constant (3 for MAX30102) and `FIFO_MAX_SAMPLES` (32) in header or registers file
- [x] 1.3 Clarify `MAX30100_FIFO_DEPTH` — kept at `0x20` (32) for MAX30102's 32-word FIFO, with added `FIFO_WORD_BYTES` and `FIFO_MAX_SAMPLES` constants. The mask `& (DEPTH-1)` correctly wraps pointer diff at 32.

## 2. FIFO Read Implementation

- [x] 2.1 Rewrite `readFifoData()` to read 3 bytes per FIFO word instead of 2
- [x] 2.2 Assemble 18-bit left-aligned words: `(b0 << 16) | (b1 << 8) | b2`, then right-shift by 2 to get 16-bit value
- [x] 2.3 Fix slot counting: compute samples as `toRead / 2` (SpO2 mode uses 2 words per sample)
- [x] 2.4 Update burst read buffer size to `MAX30100_FIFO_DEPTH * 6` (32 × 6 = 192 bytes)

## 3. Verification

- [x] 3.1 Build with PlatformIO — confirm no compilation errors
- [ ] 3.2 Flash to ESP32 and stream raw IR/Red values — verify readings look like a proper PPG waveform (not noise)
- [ ] 3.3 Compare displayed BPM against a reference (finger pulse check, or known rest HR)
