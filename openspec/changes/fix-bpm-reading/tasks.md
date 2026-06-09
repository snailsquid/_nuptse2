## 1. Drive-level register fixes

- [x] 1.1 Fix `readFifoData()`: change burst read length from `FIFO_WORD_BYTES * toRead` to `FIFO_WORD_BYTES * 2 * toRead` and remove `samples = toRead / 2`
- [x] 1.2 Verify MAX30102 detects: `EXPECTED_PART_ID` is already `0x15`, confirm no regression

## 2. Application-layer config

- [x] 2.1 Verify `DEFAULT_MODE` is `SPO2_HR` in `MAX30100.h` — already set, ensure `main.cpp` does not override to `HRONLY`

## 3. Verification

- [ ] 3.1 Upload to hardware and read serial: confirm BPM reads 50-120 with finger, SpO2 94-100%
- [ ] 3.2 Confirm 0 BPM / 0 SpO2 when no finger
- [ ] 3.3 Confirm OVF_COUNTER stays at 0 during normal operation
