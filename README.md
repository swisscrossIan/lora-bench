# lora_bench_tag

Benchtop triage-tag firmware for the **tag** side of the LoRa bench rig.
(The receiver side lives in a separate `lora_bench_receiver` repo.)

## Overview

`lora_bench_tag/lora_bench_tag.ino` reads four tear-strip inputs on a
**Heltec WiFi LoRa 32 V4**, plus deactivate/transport control wires, and emits
a uniform snake_case status record over the native USB-CDC serial port at
115200 baud. The number of torn (pulled) strips maps directly to a triage
level/colour. The record format is ready to forward to an API.

## Hardware

| Strip | Pin label | GPIO | Colour |
|-------|-----------|------|--------|
| 1 | b56 | 42 | GREEN |
| 2 | b55 | 41 | YELLOW |
| 3 | b51 | 1  | RED (see note) |
| 4 | b52 | 38 | BLACK |

Control inputs (also `INPUT_PULLUP`, pulled = `HIGH`):

| Function | Pin label | GPIO | Effect when pulled |
|----------|-----------|------|--------------------|
| Deactivate | i49 | 48 | `tag_status` → `inactive` |
| Transport  | i50 | 47 | `disposition` → `transported` |

Notes from bring-up:

- Original strip 3 (b53 = GPIO39) and strip 4 (b52 = GPIO38) were the intended
  RED/BLACK pins. The active sketch uses verified-good pins `{42, 41, 1, 38}`.
- **GPIO1 (b51)** is used as strip 3 due to pin malfunctions, but should be
  left open for battery monitoring in a future build.
- GPIO5 (b47) is shorted and b48 is intermittent — both avoided.
- Grounds land on **b62 / i62** (confirmed GND).

## Tag identity

The `tag` id is the ESP32's full 48-bit factory **MAC** (12 hex digits). It is
globally unique with no per-device configuration. A human-friendly name is
mapped to this id in the database rather than on the device.

## Output

A record is emitted only when state changes, as snake_case `key: value` lines
delimited by a blank line (ready for an API push):

```
tag: A0B1C2D3E4F5
type: medical
torn: 2
level: 2
tag_status: active
triage_status: yellow
disposition: on_scene
```

| Field | Values |
|-------|--------|
| `tag` | full MAC (12 hex digits) |
| `type` | `medical` |
| `torn` | number of strips torn (0–4) |
| `level` | same as `torn` |
| `tag_status` | `active` / `inactive` |
| `triage_status` | `none`, `green`, `yellow`, `red`, `black` |
| `disposition` | `on_scene` / `transported` |

## Behaviour

- Inputs are `INPUT_PULLUP`: **seated = grounded = intact = `LOW`**,
  **pulled = `HIGH`**. 50 ms debounce per pin.
- `torn` counts torn strips; `level` mirrors it. `triage_status` maps
  level 0→`none`, 1→`green`, 2→`yellow`, 3→`red`, 4→`black`.
- `tag_status` starts `inactive`; the first strip pull makes it `active`.
  Pulling the **i49** deactivate wire forces it back to `inactive`.
- `disposition` starts `on_scene`; pulling the **i50** transport pin sets it to
  `transported`.
- The i49/i50 controls track the wire **live** — reseating the wire reverts the
  state (e.g. reseat i50 and `disposition` returns to `on_scene`).

## Build

Open `lora_bench_tag/lora_bench_tag.ino` in the Arduino IDE, select the
**Heltec WiFi LoRa 32 V4** board, and upload. Open the Serial Monitor at
115200 baud to watch status records.
