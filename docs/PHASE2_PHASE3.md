# Phase 2 + Phase 3 Increment

This overlay adds the real telemetry model and compact NMEA 0183 TCP ingest layer.

## Phase 2: telemetry model

Implemented:

- `TelemetryStore` now supports incremental updates instead of only full snapshot replacement.
- Each sensor value uses `TimedValue<T>` with:
  - latest value
  - monotonic update timestamp
  - quality state
  - source label
- `TelemetrySnapshot` now includes a `SourceState` section for the NMEA 0183 link.
- Staleness detection is centralized in `TelemetryStore::refresh_staleness()`.
- UI tiles now show `--` for missing data and warning states for stale data.

Key files:

```text
src/telemetry/TelemetryTypes.h
src/telemetry/TelemetryStore.h
src/telemetry/TelemetryStore.cpp
```

## Phase 3: compact NMEA 0183 TCP input

Implemented:

- `NmeaTcpClient` reads NMEA 0183 lines from a TCP stream in a background thread.
- `Nmea0183` remains compact: one tokenizer, one parser entry point, private helper functions.
- No header-per-sentence and no decoder class per message type.
- Parser emits `nmea::TelemetryUpdate` objects that are applied to `TelemetryStore`.
- Multi-part AIS VDM/VDO payload assembly remains in the central NMEA parser; full AIS decoding is still Phase 5.
- The longitude parser was fixed to correctly parse NMEA `dddmm.mmmm` fields.

Key files:

```text
src/nmea/Nmea0183.h
src/nmea/Nmea0183.cpp
src/nmea/NmeaTcpClient.h
src/nmea/NmeaTcpClient.cpp
```

## Running with live NMEA TCP

Dummy telemetry remains the default:

```bash
./helm-ui
```

Live TCP NMEA input:

```bash
./helm-ui --source tcp-nmea0183://127.0.0.1:10110
```

Alternative forms:

```bash
./helm-ui --source=tcp-nmea0183://127.0.0.1:10110
./helm-ui --nmea-tcp=127.0.0.1:10110
HELM_UI_NMEA_SOURCE=tcp-nmea0183://127.0.0.1:10110 ./helm-ui
```

The app filters its own `--source` / `--nmea-tcp` options before passing arguments to GTK, avoiding GTK's unknown-option errors.

## Sentence support in this increment

```text
RMC  position, SOG, COG
GGA  fix quality, satellites, position
VTG  COG/SOG
HDT  true heading
HDG  magnetic heading
MWV  apparent/true wind angle and speed
MWD  true wind direction and speed
DPT  depth + offset
DBT  depth in meters
VDM  AIS receive payload assembly
VDO  own-vessel AIS payload assembly
```

## Tests added

```text
tests/nmea_tokenizer_test.cpp
tests/nmea_parser_test.cpp
tests/telemetry_store_test.cpp
```

The tests validate tokenizer behavior, lat/lon conversion, wind/depth/heading parsing, and telemetry staleness behavior.
