# Phase 0 — Visual Baseline and Architecture Lock

## Visual baseline

The images in `assets/reference/` are the design reference:

- `helm_mockup_concept_board_4x.png`
- `helm_mockup_wide_composite_4x.png`

They define the desired style:

- dark navy cockpit background
- thin cyan/blue panel borders
- large white numerals
- muted labels
- green OK states
- yellow caution states
- orange warning states
- red danger states
- large touch targets
- instrument-like graphics instead of phone-like UI

The PNGs are **not** source UI assets. The app recreates the look with GTK, CSS, and scalable Cairo drawing.

## Screen model

The app is a persistent helm shell plus center-launched sub-apps:

```text
Top status bar
Left sensor tiles | Center app viewport | Right sensor tiles
Bottom navigation bar
Global alarm overlay
```

The center viewport hosts modules:

- Home
- Navigation
- Sail/Wind
- Anchor
- AIS
- Systems
- Bilge
- Windlass
- Coastal
- Motion/INS
- Media
- Messages
- Alarms

Phase 1 implements the skeleton and placeholder versions of the most important modules.

## Safety priority

Highest priority overlays always win:

1. MOB
2. AIS collision risk
3. Grounding/shallow water/shore danger
4. Anchor drag
5. High-water bilge alarm
6. Engine/electrical/windlass fault
7. Navigation and sailing performance
8. Messages
9. Music

Media and messaging must never obscure alarms.

## NMEA 0183 rule

The project uses one compact NMEA parser:

- one tokenizer
- one `parse_nmea0183_line()` function
- branch by sentence formatter
- private helpers inside `Nmea0183.cpp`
- no public decoder class per sentence
- no header per sentence type

AIS binary payload decoding may live separately because AIS VDM/VDO payloads are a different binary protocol embedded inside NMEA sentences.

## Icon rule

Icons are recreated as scalable drawing primitives. In phase 1 they are procedural Cairo icons. Later, we can add SVG loading while keeping Cairo fallbacks.
