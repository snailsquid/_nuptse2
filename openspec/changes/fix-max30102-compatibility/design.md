## Context

The project uses the oxullo MAX30100lib library on an ESP32 with a MAX30102 sensor. The library assumes a MAX30100 FIFO layout (4 bytes/sample: 2×16-bit), but MAX30102 stores 6 bytes/sample (2×18-bit, 3 bytes each). The sensor passes the part ID check (both return 0x15), so it "works" but produces garbage IR/Red values, which feeds noise into the beat detector and causes inflated BPM.

The signal processing pipeline (DC remover, low-pass filter, beat detector FSM, SpO2 calculator) functions correctly on properly-formatted data — only the raw FIFO read layer needs fixing.

## Goals / Non-Goals

**Goals:**
- Correctly read 18-bit left-aligned FIFO data from MAX30102
- Convert 18-bit samples to 16-bit for downstream processing
- Fix FIFO slot counting (words vs samples in 2-LED mode)
- Increase buffer sizes for deeper MAX30102 FIFO (32 vs 16)

**Non-Goals:**
- Rewriting the beat detector, filters, or SpO2 calculator
- Changing the PulseOximeter API or main.cpp
- General sensor calibration or BPM accuracy tuning beyond correct data acquisition
- Supporting MAX30101 or MAX30105

## Decisions

### Decision 1: In-place modification vs compatibility layer
**Choice**: Modify MAX30100.cpp directly
**Alternatives considered**: Create a MAX30102 subclass; use a compile-time flag
**Rationale**: The existing code has a single `readFifoData()` method that needs to change. The FIFO data format difference is fundamental — no amount of post-processing can fix it. A subclass would duplicate the entire read path. Since the project uses only MAX30102, there's no runtime MAX30100/MAX30102 switching needed.

### Decision 2: 18-bit → 16-bit conversion
**Choice**: Right-shift by 2 bits (lowest 2 bits are always 0 from 18-bit left-aligned data)
**Rationale**: MAX30100 returns 16-bit data directly. MAX30102 returns 18-bit left-aligned. Shifting right by 2 preserves the same effective resolution and dynamic range. This is the standard approach used in SparkFun's library.

### Decision 3: Buffer sizing
**Choice**: RINGBUFFER_SIZE from 16 → 32
**Rationale**: MAX30102 FIFO depth is 32 (vs 16 on MAX30100). The burst read buffer must also grow proportionally: `MAX30100_FIFO_DEPTH * 6` bytes instead of `* 4`.

## Risks / Trade-offs

- **[Breaking raw values]** Debugging mode raw IR/Red values will change amplitude (they were corrupt before, now they're correct). Any serial parsing or PC-side tools expecting old values need to re-baseline.
- **[MAX30100 untested]** The change assumes MAX30100 uses 2 bytes/word. If someone connects a real MAX30100, the 3-byte read path must be tested separately. This project uses MAX30102 only, so acceptable.
