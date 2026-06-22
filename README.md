# lora_bench_tag

Benchtop triage-tag firmware for the **tag** side of the LoRa bench rig.
(The receiver side lives in a separate `lora_bench_receiver` repo.)

## Overview

`lora_bench_tag/lora_bench_tag.ino` reads four tear-strip inputs on a
**Heltec WiFi LoRa 32 V4** and reports a triage acuity level over the native
USB-CDC serial port at 115200 baud. The number of torn (pulled) strips maps
directly to a triage level/colour.

## Hardware

| Strip | Pin label | GPIO | Colour |
|-------|-----------|------|--------|
| 1 | b56 | 42 | GREEN |
| 2 | b55 | 41 | YELLOW |
| 3 | b51 | 1  | RED (see note) |
| 4 | b52 | 38 | BLACK |

Notes from bring-up:

- Original strip 3 (b53 = GPIO39) and strip 4 (b52 = GPIO38) were the intended
  RED/BLACK pins. The active sketch uses verified-good pins `{42, 41, 1, 38}`.
- **GPIO1 (b51)** is used as strip 3 due to pin malfunctions, but should be
  left open for battery monitoring in a future build.
- GPIO5 (b47) is shorted and b48 is intermittent — both avoided.
- Grounds land on **b62 / i62** (confirmed GND).

## Tag identity

Each tag prints its own number so the receiver can tell devices apart. The full
48-bit factory MAC is printed once at startup, and the tag id leads every line:

```
MAC: A0B1C2D3E4F5
Tag: D3E4F5
...
Tag: D3E4F5  torn=2  level=2  ACTIVATED - YELLOW
```

The number is controlled by the `TAG_NUMBER` define:

- `TAG_AUTO` (default) — derives a stable, unique 6-hex-digit id from the low 3
  bytes of the ESP32's factory-programmed MAC. No per-device editing needed.
- A fixed value (e.g. `#define TAG_NUMBER 1`) — hand-assigns a human-friendly
  6-digit zero-padded number (`Tag: 000001`); set it before flashing each device.

## Behaviour

- Strip **seated = grounded = intact** → reads `LOW`.
- Strip **pulled = torn** → reads `HIGH` (via `INPUT_PULLUP`).
- 50 ms debounce per strip.
- The torn count is printed only when it changes:

| torn | level | meaning |
|------|-------|---------|
| 0 | 0 | STANDBY (not activated) |
| 1 | 1 | ACTIVATED - GREEN |
| 2 | 2 | ACTIVATED - YELLOW |
| 3 | 3 | ACTIVATED - RED |
| 4 | 4 | ACTIVATED - BLACK |

## Build

Open `lora_bench_tag/lora_bench_tag.ino` in the Arduino IDE, select the
**Heltec WiFi LoRa 32 V4** board, and upload. Open the Serial Monitor at
115200 baud to watch acuity transitions.
