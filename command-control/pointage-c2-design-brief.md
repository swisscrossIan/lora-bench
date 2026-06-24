# Pointage C2 Dashboard — Design Brief

A context file for Claude Design. Pair this with `pointage-design-system.md` and `pointage-sample-data.json`.

## What this is

Pointage is a LoRa-based mass casualty incident (MCI) triage tag system. Each casualty gets a physical tag (a low-power LoRa device). A responder sets acuity by tearing a strip on the tag. The tag beacons its ID, acuity, position, and a sequence number over long-range radio (915 MHz LoRa) back to a gateway, which feeds a backend and this dashboard.

This dashboard is the command and control (C2) surface. It is the live picture an incident commander sees at the command post: how many casualties, what acuity, where they are, what just changed, and what needs to move first.

The edge environment usually has no cellular or internet. That is the whole reason the radio layer exists. Assume the dashboard itself runs at a command post or emergency operations center with connectivity to the gateway, possibly on a laptop and possibly mirrored to a wall display.

## Who uses it

Three operator roles share one product. Design for the first as primary.

1. Incident Commander / Triage Officer. Wants the casualty tally and the map at a glance, from across a room. Cares about totals, acuity distribution, and trend.
2. Transport / Evacuation Officer. Wants an ordered worklist of who to move next, worst acuity first, with location.
3. Logistics / Comms. Wants device and gateway health: battery, signal, stale tags, link status.

## The job to be done

Maintain real-time situational awareness of casualty count, acuity mix, location, and change, so command can allocate resources and sequence evacuation. Everything else is secondary to that.

## Screens

### 1. Incident Overview (primary, the default screen)

A three-zone command layout. This is the screen that matters most and the one to mock first.

- Top bar: incident name, running incident clock (time since declared), total casualty count, gateway/connection status, and a last-update timestamp.
- Tally board: four large acuity count cards (Immediate, Delayed, Minor, Expectant). Each shows a big tabular numeral, the category label, and a short-window delta (for example, "+2 in last 5 min"). This is the heart of MCI command and must be glanceable from distance.
- Live map: a dark basemap with one marker per tag, colored by acuity, clustered when zoomed out. A cluster takes the color of its highest-acuity member, because command cares about the worst case inside a cluster. Includes a legend and a worst-first sort. (Implemented with React-Leaflet in the real app.)
- Priority / alert rail: a right-hand column listing recent changes and things that need attention, newest first. Examples: a tag that escalated to Immediate, a gateway link degrading, a tag going stale. This is what the evac officer scans.

### 2. Casualty / Tag List

A dense, sortable, filterable table of every tag. Columns: tag ID, acuity (as a colored chip), last seen (relative), position, battery, signal (RSSI), lifecycle state, sequence number. Default sort is acuity worst-first, then most-recently-changed. Filter chips along the top for acuity and lifecycle. This is the workhorse view for triage and transport officers.

### 3. Tag Detail

One casualty. Shows the full acuity history as a vertical timeline (so an escalation from Delayed to Immediate is visible), a small location track, and device health (battery trend, last RSSI, last sequence number, lifecycle state, owning agency). This is where you confirm "did this tag really escalate or is it a glitch."

### 4. Fleet / Device Health (logistics)

A table of devices and gateways rather than casualties. Battery, RSSI, last-seen, lifecycle (provisioned, active, stale, lost, retired), owning agency, and gateway link status. Lower priority for the mock but worth a frame.

## Core components to design

- Acuity tally card (four variants, one per acuity, big tabular count, label, delta).
- Map marker and cluster badge, colored by acuity.
- Acuity chip (small inline label used in tables and the alert rail).
- Priority / alert row (icon, message, tag reference, timestamp, optional jump-to-map).
- Tag table row, including acuity chip, relative time, battery and signal micro-indicators.
- Event timeline node (acuity change, with from/to and time).
- Connection / gateway status indicator (kept visually distinct from triage colors, see the design system).
- Incident header bar with running clock.

## Data model (so the mock reflects the real shape)

Two tables back this, mirrored from the Supabase schema.

`devices`: one row per physical tag or gateway. Fields include device/tag ID (agency-prefixed, e.g. `SWX-0142`), owning agency, lifecycle, battery, last RSSI, first seen, last seen.

`events`: append-only beacons. Fields include tag ID, sequence number (`seq`), acuity, latitude, longitude, battery, RSSI, timestamp. The dedup key is `(tag_id, seq)`, so the same beacon heard by two gateways is one event. The dashboard generally renders the latest event per tag for current state, and the full event stream for a tag's timeline.

Acuity is a four-level enum following standard START triage:

- `immediate` — Red. Life-threatening, evacuate first.
- `delayed` — Yellow. Serious but stable.
- `minor` — Green. Walking wounded.
- `expectant` — Black. Deceased or unsurvivable.

Lifecycle is one of: `provisioned`, `active`, `stale` (no recent beacon), `lost`, `retired`.

## Tone and priorities

This is an operational instrument, not a marketing dashboard. Favor density, legibility at distance, and fast scanning over whitespace and decoration. Dark theme. The triage colors are the only loud thing on the screen, and they are load-bearing, so nothing else should compete with them. Never rely on color alone to convey acuity; always pair with a label or icon, because responders may be in poor lighting or colorblind.

## What to mock first

Lead with the Incident Overview screen at desktop / wall width, then the Casualty List, then Tag Detail. Fleet Health last. Use the sample data so counts, the map, and the alert rail all read as a real, in-progress incident.
