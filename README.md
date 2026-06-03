# Helm UI GTK4

Modern C++/GTK4 cockpit helm UI starter project for a 10 inch sailboat touchscreen.

This repository implements **Phase 0** and **Phase 1** of the plan:

- Phase 0: visual baseline, module boundaries, safety hierarchy, and architecture notes.
- Phase 1: compilable GTK4/gtkmm shell skeleton with dummy telemetry, scalable Cairo icons, screen modules, and the 4x PNG concept images stored as reference assets.

The 4x PNG files in `assets/reference/` are **visual references only**. The running UI is drawn with GTK widgets and scalable Cairo primitives; no icons are cropped from the PNGs.

## Current screens

The phase 1 shell includes these modules:

- Home
- Anchor
- AIS
- Systems
- Bilge
- Windlass

The center area behaves as an app viewport. The top/bottom bars and side sensor tiles stay persistent while the active module changes.

## Dependencies

Linux:

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build pkg-config libgtkmm-4.0-dev
```

Windows/MSYS2 UCRT64:

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-gtkmm-4.0
```

## Build

Linux:

```bash
cmake --preset linux-debug
cmake --build --preset linux-debug
./build/linux-debug/helm-ui
```

Windows/MSYS2 UCRT64 shell:

```bash
cmake --preset windows-msys2-ucrt64-debug
cmake --build --preset windows-msys2-ucrt64-debug
./build/windows-ucrt64-debug/helm-ui.exe
```

## Tests

The first test target validates the compact NMEA 0183 tokenizer/parser foundation.

```bash
ctest --preset linux-debug
```

## Design rules

- Keep the cockpit shell stable and persistent.
- Launch sub-apps only inside the center viewport.
- Keep sensor readings in a central telemetry snapshot with timestamps.
- Do not parse NMEA in UI widgets.
- Do not render from non-GTK threads.
- Do not use a header/class per NMEA 0183 sentence.
- Do not crop icons from concept PNGs; recreate icons as scalable vector/Cairo drawing.
- Windlass controls must be locked by default and press-and-hold only.
- Music/messages, when added later, must never hide safety alarms.

## Repository layout

```text
assets/reference/   4x PNG concept references
assets/themes/      theme reference data
docs/               Phase 0 design docs
src/app/            GTK application/window
src/shell/          persistent cockpit frame
src/modules/        center viewport sub-apps
src/telemetry/      latest readings and timestamps
src/graphics/       theme and scalable icons
src/nmea/           compact NMEA tokenizer/parser
```
