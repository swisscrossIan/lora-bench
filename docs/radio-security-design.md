# Radio & Security Design Notes

Status: **planning** — captured before the radio work starts. Nothing here is
implemented yet. Current firmware emits JSON over USB serial only; there is no
LoRa/BLE/WiFi transmit code, and no encryption.

The goal of this note is to lock in the decisions we've already made so they
aren't relitigated (or lost) once the radio work begins.

---

## 1. Transports and what each carries

Data should be transferable over **LoRa**, **BLE**, and **WiFi**. These differ
enormously in bandwidth, so they do **not** all carry the same payload:

| Transport | Budget | Carries |
|-----------|--------|---------|
| LoRa      | Tiny (see §2) | Compact binary frame: `tag_id`, status/triage flags, GPS, battery |
| BLE / WiFi| Large | Full record incl. NFC user info, diagnostics, anything held locally |

One source of truth on the device, multiple encoders: keep the current state in
local storage (§5) and let each transport serialize whatever subset it can carry.

---

## 2. Payload size — LoRa is the hard constraint

The current JSON record is fine for serial/BLE/WiFi but **must not** go over LoRa:

- A realistic active record with GPS + NFC is already **~250–280 bytes** of JSON.
- Raw LoRa (SX1262) caps a single packet at **255 bytes**.
- LoRaWAN application payload is much smaller — roughly **51 bytes** at long
  range (SF12) up to ~222 at short range (SF7).

So the LoRa link uses a **compact binary frame (~25–30 bytes)**, not JSON:
`tag_id` as 6 raw bytes (not 12 hex chars), status/triage/disposition packed
into 1–2 flag bytes, lat/lon as 4-byte fixed-point each, battery 1 byte.

The real size wildcard is **NFC free-text** — it's unbounded. NFC does **not**
go over LoRa; it's held locally for BLE/WiFi.

---

## 3. Ownership and per-agency isolation

Decided and implemented in `supabase/schema.sql`:

- A tag's owning agency (`client_id`) and issuer (`owner_id`) are recorded in the
  device registry, set at provisioning.
- The tag transmits **only its `tag_id`**. The backend resolves `client_id`
  server-side (the upsert trigger stamps it onto each event). So agency identity
  **never travels over LoRa.**

Two distinct isolation layers — don't conflate them:

- **RLS (database):** controls who can query which rows. *Not yet enabled* — the
  schema ships a commented policy sketch; turn it on once auth maps a user → client.
- **AES (air):** controls who can read packets off the RF. RF is broadcast, so a
  receiver in range hears every agency's tags regardless of payload — confidentiality
  on the air requires payload encryption (§4). RLS does nothing for this.

Some deployments won't share a DB at all (e.g. a police agency wants its own
instance). That changes nothing about the air layer: separate instances still
share the RF medium, so cross-agency confidentiality over LoRa still depends on §4.

---

## 4. Encryption (when we turn it on)

Symmetric **AES on each side**, with a per-device (or per-agency) key held by both
the tag and that agency's receiver. Two paths:

1. **LoRaWAN (preferred).** Inherit it: LoRaWAN encrypts the app payload with
   AES-128 (CTR) under a per-device `AppSKey` and authenticates every frame with a
   MIC (AES-CMAC) under the `NwkSKey`, plus a frame counter for replay protection.
   Different devices/agencies → different keys → one agency cannot decrypt another's
   traffic. Use a network server (ChirpStack self-hosted, or TTN).

2. **Raw point-to-point LoRa.** Roll it carefully: **AES-GCM or AES-CCM**
   (authenticated modes) — **never ECB, never bare CTR.** GCM/CCM gives
   encryption + MIC in one.

**Authentication = the real anti-spoof.** A plaintext `owner_id` in the payload
does NOT prevent spoofing (it's sniffable and replayable). The MIC from an
authenticated mode is what actually rejects forged packets — so encryption-with-
authentication solves eavesdropping and spoofing together.

The hard part is **keys & nonces, not the cipher:**

- **Keys:** provision a per-device key into NVS at onboarding. ESP32-S3 has
  hardware AES (fast/low-power) plus flash encryption + eFuse to protect keys at
  rest; an ATECC608 secure element is an option for hardware-held keys.
- **Nonce uniqueness (critical):** CTR/GCM are catastrophic on (key, nonce) reuse.
  Derive the nonce from `device_id + seq` and **persist `seq` in NVS across
  reboots** — a power cycle that resets the counter reuses nonces.
- **Replay protection:** include the monotonic counter in the authenticated data;
  the receiver rejects counters it has already seen.

**Size:** CTR adds 0 bytes; a MIC adds ~4 bytes (LoRaWAN's size). Negligible on a
~28-byte frame — encryption is not what pressures the payload budget.

---

## 5. Local storage on the tag

The board (ESP32-S3, ~8 MB flash) has ample room for the small datasets involved:

- Keep the device's current state — structured triage fields **and** the NFC/user
  blob — in **NVS** (or a small LittleFS ring buffer).
- "Both data sets" (the compact LoRa set + the fuller BLE/WiFi set) cost well under
  a kilobyte; storage is a non-issue.
- **Store-and-forward** (buffer events while out of range, flush on reconnect) fits
  easily — 8 MB holds many thousands of records.

Also held locally: the tag's own `client_id`/`owner_id` (so it knows its agency and
can report it over BLE/WiFi), and NFC tap-to-store user info.

---

## 6. TODO when radio work starts

- [ ] Define the compact binary LoRa frame format (§2) and a versioned header.
- [ ] Add SX1262 transmit code (Heltec V4 / ESP32-S3).
- [ ] Ingestion/parser layer: JSON/binary → flat `events` columns (note: this
      component is not in the repo yet; it's what writes `events.client_id` etc).
- [ ] NVS: persist `seq` across reboots; store `client_id`/`owner_id`; NFC blob.
- [ ] Choose LoRaWAN vs raw P2P (§4); wire up key provisioning.
- [ ] Enable RLS in `supabase/schema.sql` once auth maps user → client.
