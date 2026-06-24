# Pointage C2 Dashboard — Design System

Visual language and design tokens for Claude Design. The triage palette is safety-critical and standardized, so treat those values as fixed. Everything else is a starting point you can refine.

## Theme

Dark, operational, instrument-grade. A command post or EOC is often low-light, the dashboard may run for hours, and it may be mirrored to a wall display. A dark shell reduces eye strain and lets the triage colors carry the signal. The triage hues are the only saturated thing on screen; the chrome stays neutral so nothing competes with acuity.

## The triage palette (fixed)

These map to standard START triage and are the most important colors in the product. They are used for map markers, tally cards, acuity chips, and timeline nodes. Always pair color with a text label or icon. Never use these four hues for anything other than acuity.

| Acuity      | Meaning                | Marker / chip | On-dark text pairing |
|-------------|------------------------|---------------|----------------------|
| Immediate   | Red. Evacuate first.   | `#FF453A`     | white text           |
| Delayed     | Yellow. Serious, stable| `#FFD60A`     | near-black text `#111` |
| Minor       | Green. Walking wounded | `#32D74B`     | near-black text `#111` |
| Expectant   | Black. Deceased        | `#8A8F98`     | white text           |

Note on Expectant. The triage category is "black," but pure black is invisible on a dark shell, so render it as a neutral slate `#8A8F98` with a distinct glyph (a small cross or diamond) and a 1px lighter outline. The icon, not the color, is what distinguishes it. Do not use pure black for this category in this theme.

Low-alpha tints for filled backgrounds (cards, selected rows), use the marker hue at roughly 14 to 18 percent alpha over the surface color.

## Brand and interaction color

- Brand / accent (interactive): clinical teal. Base `#0097A7`, brightened for dark UI to `#14B8C4`. Used for links, selected states, focus rings, primary buttons, the brand mark, and active nav. Carries through from the existing Pointage / Grafana work.
- Reserve teal for "the system," not for casualty data. Keeping the interaction color teal means it never collides with the green/yellow/red of triage.

## System status color (deliberately not triage colors)

Connectivity, battery, and gateway health must not borrow the triage hues, or operators will misread a degraded link as a Delayed casualty. Use a separate language:

- OK / healthy: teal `#14B8C4` or neutral, plus a solid icon.
- Idle / unknown: neutral grey `#636872`.
- System fault / alert: violet `#BF5AF2` rather than red, so a system problem never reads as an Immediate casualty.

Lean on icons and outline state for status, not just fills.

## Neutral shell

| Token              | Value      | Use                                  |
|--------------------|------------|--------------------------------------|
| `bg/base`          | `#0B0F14`  | app background                       |
| `surface/1`        | `#131922`  | cards, panels                        |
| `surface/2`        | `#1A2330`  | raised elements, header bar          |
| `border`           | `#232C38`  | dividers, card borders               |
| `text/primary`     | `#E6EAF0`  | primary text, big numerals           |
| `text/secondary`   | `#9AA4B2`  | labels, secondary data               |
| `text/tertiary`    | `#6B7585`  | timestamps, hints                    |

## Typography

- UI and labels: IBM Plex Sans (or Inter if Plex is unavailable). Operational, neutral, legible at distance.
- Numeric and machine data: IBM Plex Mono for tag IDs, sequence numbers, coordinates, RSSI, and timestamps. The monospace gives the operational instrument feel and keeps IDs scannable.
- Use tabular figures everywhere numbers appear, especially tally counts and tables, so columns align and counts do not jitter as they tick up.

Scale (desktop / wall):

- Tally count numeral: 56px, weight 600, tabular.
- Screen title: 20px, weight 600.
- Section / card label: 13px, weight 600, letter-spacing 0.04em, uppercase.
- Table and body: 14px.
- Micro / timestamp: 12px.

## Layout and density

- 4px base spacing grid. Common steps: 4, 8, 12, 16, 24, 32.
- Corner radius: 8px cards, 6px buttons, 4px chips and markers.
- Compact density. This is operational, not editorial. Table rows around 36 to 40px. Tight, consistent gutters.
- Incident Overview uses a fixed three-zone command layout: tally board across the top, map filling the center, alert rail on the right. The map is the largest single region.

## Map styling

- Dark basemap (Carto Dark Matter or equivalent) so triage markers pop.
- Markers are uniform size, colored by acuity, with a thin dark outline for separation against the map.
- Clusters show a count badge and take the color of their highest-acuity member.
- Persistent legend in a corner mapping color to acuity. Worst-first ordering in the legend.

## Accessibility and field constraints

- Color never stands alone. Every acuity is also a word or icon.
- Assume imperfect lighting and some colorblind users. The red/green pair plus icons and labels keeps it readable.
- High contrast for the tally numerals; they must read across a room.
- Numbers use tabular figures so a changing count does not shift layout.
