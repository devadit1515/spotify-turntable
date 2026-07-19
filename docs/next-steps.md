# Next steps (when the parts arrive)

The firmware compiles clean and the parts are being ordered. Here's the pickup
plan once the box lands — full detail is in [build-guide.md](build-guide.md).

## Parts locked in
- ESP32-S3-N8R2 (₹795, Vishaworld)
- 64×64 P3 HUB75 panel (Robu / rareComponents) — **longest lead time, order first**
- 5V/5A Maxicom SMPS (₹350) + molded 3-pin cord (cut & strip the C13 end)
- 28BYJ-48 + ULN2003 (₹173)
- 24-LED WS2812 ring, 86 mm (₹125)
- 1000 µF 16V cap, 22 AWG red/black wire, F-F + F-M jumpers
- Frosted acrylic (~A5) + PLA for the printed parts
- Screws: buy a local M2.5/M3/M4 assortment after printing (test-fit first)

## First session when parts arrive
1. **Measure** your real panel (`panel_w/h/depth`) + motor tab spacing → update
   `hardware/params.scad` before printing anything.
2. **Flash + first light** (build-guide Phase 2): wire only the panel + 5V, get
   the Spotify splash → album art. Nothing else connected yet.
3. Then add **motor** (Phase 3), **halo** (Phase 4), **enclosure** (Phase 5).
4. Each phase has a pass/fail check — don't move on until it passes.

## Before flashing
- Run `tools/getRefreshToken` once to get your Spotify refresh token.
- Fill `src/config.h` (WiFi + Spotify). It's git-ignored — never committed.

## Portfolio polish (later)
- Add a hero photo / GIF to the top of the README once it's glowing.
- Lower `MATRIX_BRIGHTNESS` to ~90–110 before filming (LEDs blow out on camera).
