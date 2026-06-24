# Handoff: Pointage C2 — Incident Overview

## Overview
This is the **primary screen** of the Pointage command-and-control (C2) dashboard: a live MCI (mass casualty incident) triage picture for an incident commander at a command post or wall display. It answers, at a glance: how many casualties, what acuity, which agencies are tagging, where casualties are, who has been moved off-site, and what just changed.

Pointage casualties carry LoRa triage tags that beacon ID, acuity, position, battery, RSSI, and a sequence number to a gateway. This dashboard renders the latest event per tag for current state, plus an alert feed of recent changes.

## About the Design Files
The file in this bundle (`Incident Overview.dc.html`) is a **design reference created in HTML** — a working prototype showing intended look and behavior, **not production code to copy directly**. It is authored as a streaming "Design Component," so its markup uses a few custom tags (`<sc-for>`, `{{ }}` holes) and a `class Component` logic block. Do **not** port those mechanics. Instead, **recreate this design in the target codebase's environment** (the real app uses React + React-Leaflet) using its established patterns, component library, and map stack. The HTML is the source of truth for layout, color, type, spacing, and copy.

## Fidelity
**High-fidelity (hifi).** Final colors, typography, spacing, and layout. Recreate pixel-faithfully using the codebase's existing libraries. The one intentional substitution: the map basemap is faked with CSS (grid + street hints + a structure footprint) because the prototype runs offline — in the real app this is a **Carto Dark Matter** basemap rendered with **React-Leaflet**, with markers positioned by real lat/lng.

## Screen: Incident Overview
**Purpose:** Real-time situational awareness for the Incident Commander / Triage Officer. Glanceable from across a room.

### Layout
- Root: full-viewport (`100vh`, `min-height: 880px`), `background #0B0F14`, `color #E6EAF0`, font `IBM Plex Sans`. Vertical flex, `gap 12px`, `padding 14px`, `overflow: hidden`. Designed at **1920px wide** (wall display).
- Four stacked zones, top to bottom:
  1. **Header bar** — fixed height `62px`.
  2. **Agencies band** — auto height (~`72px`).
  3. **Tally board** — `display: grid; grid-template-columns: repeat(4, 1fr); gap: 12px`.
  4. **Main** — `flex: 1; display: grid; grid-template-columns: 1fr 372px; gap: 12px`. Left = map (largest region), right = priority rail.
- All panels: `background #131922` or `#1A2330`, `border 1px solid #232C38`, `border-radius 8px`.

### Components

**1. Header bar** (`#1A2330` panel, flex row, `gap 22px`, `padding 11px 18px`)
- Brand mark: 24×24 teal (`#14B8C4`) rounded square rotated 45°, glow shadow. Beside it "POINTAGE" (Plex Mono, 13px/600, letter-spacing .14em, teal) over "C2 / COMMAND" (Plex Mono, 10px, `#6B7585`).
- Vertical divider (`1px × 34px`, `#232C38`).
- Incident title "Structure Collapse — Market Hall" (19px/600). Beside it an **ACTIVE** status chip: red (`#FF453A`) text/border on `rgba(255,69,58,.14)`, with a 7px blinking dot (1.6s ease-in-out). Subline (Plex Mono 11px `#6B7585`): "INC-2026-0624-01 · IC-3 Vasquez · Cambridge, ON".
- Right cluster (`margin-left:auto`, `gap 26px`), each a labeled stat with 1px dividers between:
  - **ELAPSED**: running incident clock, Plex Mono 26px/600 tabular, format `MM:SS` (or `H:MM:SS` past an hour). Counts up live from incident declaration.
  - **CASUALTIES**: `22`, Plex Mono 26px/600 tabular.
  - **Gateways** (two rows, Plex Mono 11px): `GW-1 Command Post` with solid teal dot + "22 heard"; `GW-2 North Stack` with hollow violet (`#BF5AF2`) ring + "degraded" in violet.
  - **LAST UPDATE**: relative time, live (e.g. "13s ago").
- Stat labels: 10px/600, letter-spacing .12em, `#9AA4B2`.

**2. Agencies band** (`#131922` panel, flex row, `gap 12px`, `padding 12px 16px`)
- Left title block: "Agencies Tagging" (13px/600 uppercase, letter-spacing .04em) over "2 services on scene · 22 tags" (11px `#6B7585`).
- One block per agency (divider between), each:
  - Code in Plex Mono 14px/600 teal (`SWX`, `RWM`) + plain-text name (12px `#9AA4B2`): **SWX = Sunwest EMS**, **RWM = Regional Medical**. Subline "16 tags · 73%" / "6 tags · 27%".
  - A stacked acuity bar (`height 14px`, `border-radius 4px`, segments separated by `gap 2px`) using flex-grow weighted by count, colored by acuity. SWX weights I:3 D:4 M:7 E:2. RWM weights I:1 D:2 M:2 E:1.
  - A legend row (Plex Mono 11px) of colored dots + counts: `3 I`, `4 D`, `7 M`, `2 E`, etc.

**3. Tally board** — four acuity cards, one per acuity. Each card:
- `position: relative`, tinted background = acuity hue at ~13% alpha over surface, `border 1px` = acuity hue at ~42% alpha, `border-radius 8px`, `padding 14px 16px`, `overflow: hidden`.
- A 4px-wide left accent bar in the full acuity hue.
- Top row: acuity **shape icon** + uppercase label (13px/600, letter-spacing .05em, in the acuity hue), and a right-aligned **delta chip** (Plex Mono 12px/600, e.g. "▲ +2 / 5m" or "— 0 / 5m").
- Big count: Plex Mono **56px/600 tabular**, `#E6EAF0`, `line-height .9`.
- Transport status: a row "`N on scene`" (left, `#9AA4B2`) / "`M moved ↗`" (right, `#E6EAF0` if >0 else `#6B7585` "0 moved"), Plex Mono 11px, above a 4px progress bar (`rgba(255,255,255,.1)` track, `#E6EAF0` fill = moved/total).
- The four cards (label · count · delta · on-scene/moved):
  - Immediate · **4** · +2/5m · 3 on scene / 1 moved (25% bar)
  - Delayed · **6** · 0/5m · 6 on scene / 0 moved (0%)
  - Minor · **9** · +1/5m · 7 on scene / 2 moved (22%)
  - Expectant · **3** · 0/5m · 3 on scene / 0 moved (0%)

**Acuity shape icons** (shape encodes acuity for colorblind safety — never rely on color alone):
- Immediate → upward **triangle** (CSS borders), red.
- Delayed → **rounded square** (`border-radius 3px`), yellow.
- Minor → **circle**, green.
- Expectant → **diamond** (square rotated 45°), slate `#8A8F98` with a `1px` lighter outline (`#C2C7CF`).

**4. Live map** (left of main grid; in production = React-Leaflet + Carto Dark Matter)
- Markers: one per tag, positioned by lat/lng, uniform ~17px, **shape + color by acuity** (same shape language as tally icons), each with a thin dark outline (`box-shadow 0 0 0 1.5px rgba(11,15,20,.95)`) for separation.
- Immediate markers get a pulsing red **halo** ring (keyframe scale 1→1.7, opacity .65→0, 1.9s).
- **Stale** tags (lifecycle = stale): dashed neutral ring (`1.5px dashed #636872`) around the marker, marker dimmed to ~0.72 opacity. (RWM-0014 delayed, RWM-0019 expectant.)
- **Transported** tags: small 15px `#E6EAF0` circle badge with `↗` at top-right of the marker. (SWX-0163 immediate, SWX-0143 + SWX-0145 minor.)
- Marker `title` tooltip: `"<tag> · <acuity>[· stale][· transported]"`.
- Top-left overlay: blinking teal dot + "LIVE MAP" + center coords "43.3601, −80.3127".
- Top-right overlay: two stat tiles — **ON SCENE 19**, **MOVED OFF-SITE 3 ↗** (`rgba(19,25,34,.85)` tiles).
- Bottom-left **legend** (worst-first): Immediate, Delayed, Minor, Expectant with their shapes; plus a divided section explaining the dashed "Stale" ring and the `↗` "Transported" badge.
- Bottom-right scale bar "~40 m".
- Cluster rule (for the real map when zoomed out): a cluster takes the color of its **highest-acuity** member, with a count badge.

**5. Priority feed / alert rail** (right column, `#131922`, fixed `372px` wide, vertical flex)
- Header row: "Priority Feed" (13px/600 uppercase) + a red "2 critical" chip.
- Scrollable list; each alert row (`padding 12px 16px`, bottom border `#1c2531`):
  - 26×26 rounded-square icon tile, background + glyph color by alert kind.
  - Message (13px/600 `#E6EAF0`) + right-aligned relative time (Plex Mono 11px `#6B7585`, live).
  - Below: tag/gateway ref (Plex Mono 11px) + a detail string (11px `#6B7585`).
- Footer: "7 events · last 9 min".
- **Sort order: critical first, then warning, then info; newest within each tier.**
- Alert kinds & styling (system status colors are deliberately NOT triage hues):
  - **escalation** (critical) → glyph `↑`, red tile `rgba(255,69,58,.18)` / `#FF453A`. Detail "Delayed → Immediate".
  - **stale** (warning) → glyph `⦸`, neutral tile `rgba(99,104,114,.22)` / `#9AA4B2`.
  - **gateway** (warning) → glyph `✦`, **violet** tile `rgba(191,90,242,.18)` / `#BF5AF2` (violet so a system fault never reads as an Immediate casualty).
  - **battery / new_tag** (info) → glyphs `▮` / `+`, teal tile `rgba(20,184,196,.16)` / `#14B8C4`.

## Interactions & Behavior
- **Live clocks**: ELAPSED counts up every second from incident declaration; LAST UPDATE and every alert's relative time also tick each second. In production, drive these off real event timestamps vs. now.
- **Blink/pulse**: ACTIVE dot and LIVE dot blink (1.6s); immediate map markers pulse a halo (1.9s).
- **Real app additions** (not in the prototype but expected): clicking a marker or alert row should jump-to / focus the casualty (open Tag Detail); marker clustering on zoom-out; map pan/zoom; auto-refresh on new beacons.
- Numbers use **tabular figures** everywhere so counts don't shift layout as they tick.

## State Management
- `incident` (name, id, declared_at, last_update, commander, totals).
- `tags` — latest event per tag (current state): `{ tag_id, agency, acuity, lat, lng, battery_pct, rssi, seq, lifecycle, first_seen, last_seen }`. Plus a derived `moved`/transported flag (see note below).
- `events` — append-only beacon history (for Tag Detail timeline). Dedup key `(tag_id, seq)`.
- `alerts` — derived feed (escalations, stale, gateway, battery, new_tag).
- `gateways` — `{ id, label, status, last_seen, rssi_floor, tags_heard }`.
- Derived per render: acuity totals, per-agency acuity breakdown, on-scene vs moved counts, alert sort, relative times.

> **Note on "transported / moved off-site":** the user requires showing whether casualties have been moved off-site, but the current `lifecycle` enum (`provisioned | active | stale | lost | retired`) has **no transport state**. In the prototype this is mocked with a `moved` flag on three tags. **In production, add a real transport/disposition field** (e.g. `disposition: on_scene | en_route | transported`) to the tag/event model and drive the tally "moved" counts, the map `↗` badge, and the ON SCENE / MOVED tiles from it.

## Design Tokens
**Triage palette (FIXED — safety-critical, use only for acuity):**
- Immediate `#FF453A` (white text)
- Delayed `#FFD60A` (near-black text `#111`)
- Minor `#32D74B` (near-black text `#111`)
- Expectant `#8A8F98` (white text; render as slate + distinct shape/outline, never pure black)
- Filled tints: marker hue at ~14–18% alpha over surface.

**Brand / interaction:** clinical teal — base `#0097A7`, on-dark `#14B8C4`. Links, selected/active, focus rings, brand mark. Never on casualty data.

**System status (NOT triage hues):** OK/healthy = teal `#14B8C4`; idle/unknown = grey `#636872`; system fault = violet `#BF5AF2`. Lean on icons + outline, not just fills.

**Neutral shell:** `bg/base #0B0F14` · `surface/1 #131922` · `surface/2 #1A2330` · `border #232C38` · `text/primary #E6EAF0` · `text/secondary #9AA4B2` · `text/tertiary #6B7585`.

**Typography:** IBM Plex Sans (UI/labels), IBM Plex Mono (IDs, numbers, coords, RSSI, timestamps). Tabular figures everywhere. Scale: tally numeral 56/600 · title 19–20/600 · section label 13/600 .04em uppercase · body 14 · micro 12.

**Spacing:** 4px grid (4/8/12/16/24/32). **Radius:** 8 cards · 6 buttons · 4 chips/markers. **Density:** compact; table rows 36–40px.

## Assets
- No raster assets. Brand mark, all icons, shapes, and the faux basemap are CSS/HTML.
- Fonts: IBM Plex Sans + IBM Plex Mono via Google Fonts.
- Production map basemap: Carto Dark Matter tiles via React-Leaflet.

## Files
- `Incident Overview.dc.html` — the hifi design (open in a browser to view; it is self-contained).
- `pointage-sample-data.json` — the sample in-progress incident (incident, gateways, tags, events, alerts) the design is built from.
- `pointage-design-system.md` — full design system (palette, type, layout, map styling, a11y).
- `pointage-c2-design-brief.md` — product brief: roles, jobs, all four screens, data model.

## Remaining screens (designed in brief, not yet mocked)
Casualty / Tag List (dense sortable table), Tag Detail (acuity timeline + device health), Fleet / Device Health. See `pointage-c2-design-brief.md`.
