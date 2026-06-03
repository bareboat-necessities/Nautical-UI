# Implementation Phases

## Phase 0 — Visual baseline and architecture lock

Completed in this repository.

- Reference PNGs stored under `assets/reference/`.
- Design rules documented.
- Module boundaries defined.
- NMEA parser rule locked: one tokenizer, one parser function.

## Phase 1 — GTK4 shell skeleton

Completed in this repository.

- CMake project.
- GTK4/gtkmm window.
- Persistent helm shell.
- Top status bar.
- Bottom navigation.
- Side sensor tiles.
- Center viewport.
- Dummy telemetry source.
- Scalable Cairo marine icons.
- Home, Anchor, AIS, Systems, Bilge, and Windlass module skeletons.

## Phase 2 — Telemetry model hardening

Next:

- Expand `TelemetrySnapshot`.
- Add real timestamp aging policies per sensor.
- Add config-driven stale timeouts.
- Add source metadata.
- Add thread-safe event ingest path.

## Phase 3 — Live NMEA TCP input

Next:

- Connect `NmeaTcpClient` to the parser.
- Feed parsed updates into `TelemetryStore`.
- Support TCP reconnect and stats.
- Add more NMEA parser tests.

## Phase 4 — First usable cockpit screens

- Home with compass/wind/depth overview.
- Anchor watch with real GPS distance/radius logic.
- AIS list/radar placeholder using real target data.
- Bilge status.
- Windlass locked control screen.

## Phase 5 — AIS target engine

- AIS payload decoder.
- Target aging.
- Nearest vessels.
- CPA/TCPA.
- Threat sorting.
- AIS alarm overlay.

## Phase 6 — Anchor watch

- Set anchor here.
- Set radius.
- Arm/disarm.
- Swing circle.
- GPS stale guard.
- Drag warning/alarm states.

## Phase 7 — Bilge and windlass systems

- Bilge current/runtime/high-water alarms.
- Windlass lock/arm state.
- Press-and-hold up/down.
- Big STOP.
- Chain counter/scope calculations.

## Phase 8 — Coastal awareness

- Distance to shore.
- Distance to shallow contour.
- Tide now/high/low.
- Currents.
- Under-keel clearance.

## Phase 9 — Motion/INS module

- Roll, pitch, yaw.
- Heave.
- Rate of turn.
- Sea-state estimates.

## Phase 10 — Media/messages

- Media mini-player.
- Messages with sailing mode.
- Alarms always override.

## Phase 11 — Polish/performance/packaging

- Redraw throttling.
- Icon cache.
- Rain mode.
- Day/night theme.
- GitHub Actions.
- Installer packaging.


## Phase 2 completed in overlay

- `TelemetryStore` now has incremental NMEA update application.
- Every major sensor value keeps a latest value, update timestamp, source, and quality state.
- Central stale/missing detection was added.
- UI tiles now distinguish missing/stale values from live values.

## Phase 3 completed in overlay

- Added `NmeaTcpClient` for background TCP NMEA 0183 input.
- Kept the compact parser rule: one tokenizer and one parser function branching by formatter.
- Added `--source tcp-nmea0183://host:port`, `--nmea-tcp=host:port`, and `HELM_UI_NMEA_SOURCE`.
- Added parser/store tests and fixed longitude parsing for `dddmm.mmmm`.
