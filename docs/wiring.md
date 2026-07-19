# Wiring

Everything runs off a single **5V / 5A** supply. The ESP32-S3 is 3.3V logic; the
HUB75 panel wants 5V logic but usually accepts 3.3V on short cables — start
direct, add a level shifter only if you see ghosting (see the end of this file).

## Pin map (ESP32-S3 → everything)

These match [`src/pins.h`](../src/pins.h). Change them there if you rewire.

### HUB75 64×64 panel (the panel's IDC header is labelled)
| Panel pin | ESP32-S3 GPIO | Panel pin | ESP32-S3 GPIO |
|---|---|---|---|
| R1 | 4  | R2 | 7  |
| G1 | 5  | G2 | 15 |
| B1 | 6  | B2 | 16 |
| A  | 18 | B  | 8  |
| C  | 9  | D  | 10 |
| E  | 14 | CLK| 13 |
| LAT (STB) | 17 | OE | 21 |
| GND | GND (several — connect at least 2) | | |

> The panel's **5V** goes to the SMPS rail directly, **not** through the ESP32.

### 28BYJ-48 stepper (via ULN2003 driver board)
| ULN2003 | ESP32-S3 GPIO |
|---|---|
| IN1 | 11 |
| IN2 | 12 |
| IN3 | 38 |
| IN4 | 39 |
| `+`  (5V) | 5V rail |
| `−`  (GND) | GND |

The motor's white 5-pin JST plugs into the ULN2003 board — no thought required.

### WS2812B halo ring
| Ring | Connect to |
|---|---|
| DIN | GPIO **40** (add a 330 Ω resistor in series if you have one) |
| 5V  | 5V rail |
| GND | GND |

### INMP441 mic — OPTIONAL (only if `HALO_USE_MIC 1`)
| INMP441 | ESP32-S3 |
|---|---|
| SCK | GPIO 41 |
| WS  | GPIO 47 |
| SD  | GPIO 48 |
| L/R | GND (selects the left channel) |
| VDD | 3V3 |
| GND | GND |

## Power distribution

Power comes from a **bare 5V/5A SMPS**. Mains (from a cut-and-stripped 3-core
cord) wires into its input screw terminals; the 5V/GND output terminals feed the
whole build.

```
   mains ─▶ 5V/5A SMPS ──┬──────────────── 5V rail ─────────────────┐
   (L/N/E)  (5V out)      │   ┌── 1000 µF cap ──┐                    │
                          │   │  (panel 5V/GND) │                    │
              ▼        ▼  ▼   ▼                 ▼                    ▼
          ESP32-S3   HUB75 panel   ULN2003 +/−        WS2812B ring   (INMP441 VDD→3V3)
          5V/VIN     5V                                5V
              │        │        │                 │                  │
              └────────┴────────┴─────────────────┴──── COMMON GND ──┘
```

⚠️ **Mains safety:** wire the SMPS input only when unplugged, get L/N/E right, and
mount the SMPS **inside the base** so nothing live is touchable while powered.

Rules that keep it from browning out or letting smoke escape:

- **One common ground.** Every GND — SMPS, panel, ESP32, ULN2003, ring, mic —
  ties together. This is the single most common failure.
- **Feed the ESP32 from the rail**, into its `5V`/`VIN` pin. Do **not** also power
  it over USB at the same time (back-feeding 5V into a live USB port is bad). When
  flashing over USB, unplug the SMPS.
- **1000 µF electrolytic across the panel's 5V/GND**, close to the panel. The
  panel yanks a lot of current on bright frames; the cap absorbs the spike.
- **Thick wires for 5V/GND to the panel** (the ribbon carries data, not power).
  A 64×64 P3 panel can pull ~3.5–4 A at full white; the firmware caps brightness
  to ~63% (`MATRIX_BRIGHTNESS 160`) to keep it and the SMPS cool.

## If the panel ghosts, smears, or looks shifted

1. Set `mxconfig.clkphase = true;` in [`display_ui.cpp`](../src/display_ui.cpp) (a
   one-pixel shift fix).
2. Add a **74HCT245** buffer between the ESP32 and the panel — put at least the
   **CLK** line through it, powered at 5V. This fixes most ghosting on longer
   ribbons. Wire the ESP32 outputs to the `A` side, panel inputs to the `B` side,
   `DIR` high, `OE` low.
3. Some panels use an FM6126A driver chip and show garbage until told so —
   uncomment `mxconfig.driver = HUB75_I2S_CFG::FM6126A;`.
