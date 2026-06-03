# Phase 4 — First usable cockpit screens

This overlay turns the shell into the first usable cockpit UI rather than only a skeleton.

## Added screens

- **NAV** — route/waypoint tactical placeholder using live position, COG/SOG, and depth.
- **SAIL** — large wind dial with TWA/AWA vectors, target bands, VMG, and a simple performance estimate.
- **ALARMS** — global alarm center for depth, AIS, anchor, bilge, windlass, battery, and NMEA link warnings.

## Upgraded existing screens

- **HOME** remains the primary helm overview with compass, heading, COG/SOG, wind, anchor, and AIS tiles.
- **ANCHOR** shows swing circle, anchor distance/radius, depth, and windlass state.
- **AIS** remains a threat-view placeholder until the AIS target engine arrives in Phase 5.
- **SYSTEMS** shows electrical, bilge, windlass, and AIS status cards.
- **BILGE** shows pump/high-water/current/runtime status and timed/hold-to-run controls.
- **WINDLASS** remains locked by default with explicit press-and-hold control affordances.

## Navigation bar

The Phase 4 bottom navigation is now:

```text
HOME | NAV | SAIL | ANCHOR | AIS | SYSTEMS | ALARMS
```

Bilge and windlass are still implemented as modules, but they are primarily accessed through Systems for this phase. They remain compiled and registered so later overlays can expose them directly or through a MORE drawer.

## Safety hierarchy represented in UI

The alarm center currently evaluates:

- shallow/depth warning
- AIS danger target summary
- anchor drag warning/alarm
- bilge high-water/fault/pump-running state
- windlass fault
- low house battery
- NMEA link error/stale state

The alarm screen is intentionally separate from media/messages. Future media/messages modules must not obscure these alarms.

## Still intentionally deferred

- Real chart rendering and shoreline/depth-contour calculations.
- Real AIS binary target decoding and CPA/TCPA engine.
- Real anchor set/radius persistence.
- Real bilge/windlass output control wiring.
- Alarm acknowledge/silence persistence.

The Phase 4 code provides UI affordances and module boundaries for those features without pretending to control physical equipment yet.
