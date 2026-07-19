# Bill of materials

Prices are approximate INR (mid-2026), GST-inclusive where known. Vishaworld
stocks the boards/power/motor/wiring but **no displays or LED rings** — those come
from Robu / rareComponents / Amazon India. Verify at checkout; stock moves.

## Core build

| # | Part | Where | ₹ | Notes |
|---|---|---|---|---|
| 1 | **ESP32-S3-N8R2** dev board (8 MB flash, 2 MB PSRAM) | Vishaworld | 795 | PSRAM matters — needed for the image/TLS buffers. |
| 2 | **64×64 P3 HUB75** RGB matrix panel | Robu / rareComponents | 2000–2650 | The single biggest cost. Generic P3 is fine. |
| 3 | **5V / 5A** SMPS (metal, e.g. Maxicom) | Vishaworld | 350 | Cheapest. Bare screw terminals — you wire mains in (see item 9). |
| 4 | **28BYJ-48** stepper + **ULN2003** driver board | Vishaworld | 173 | Spins the record. Comes as a pair. |
| 5 | **WS2812B ring**, 24 LED (~86 mm OD) | Robu / Amazon IN | 150–250 | The halo — bigger ring so a 60 mm record nests inside it (`HALO_LED_COUNT 24`). |
| 6 | Frosted **acrylic** sheet (~A5, 2 mm) | Amazon IN / local | 150 | The diffuser — turns pixels into glow. |
| 7 | **1000 µF** electrolytic capacitor (16 V) | Vishaworld | 15 | Across the panel's 5V/GND — tames current inrush. |
| 8 | **Hookup wire**, thick (20–22 AWG, red/black) | Vishaworld | 50 | Carries 5V/GND to the panel (not the ribbon). |
| 9 | **Mains power cord** (3-core) — reuse an old cable, or the ₹85 C13 cord cut down | reuse / Vishaworld | 0–85 | Cut & strip the end into the SMPS input. ⚠️ mains — keep enclosed. |
| 10 | **M3/M4 screws + standoffs** assortment | Vishaworld | 50 | Mount the motor, ESP32, and panel. |
| 11 | **Dupont jumper wires** (F-F / M-F assortment) | Vishaworld | 60 | Panel / ULN2003 / ring signal connections. |
| 12 | **PLA filament** for printed parts | own / print service | ~150 | ~120 g total across all parts. |
| | **Core subtotal** | | **≈ ₹4000–4800** | |

## Optional upgrades

| # | Part | Where | ₹ | Why |
|---|---|---|---|---|
| A | **INMP441** I²S microphone | Robu / Amazon IN | 200 | True audio-reactive halo (set `HALO_USE_MIC 1`). |
| B | **74HCT245** buffer breakout | Robu / Amazon IN | 60 | Insurance against panel ghosting on longer ribbons. |

**With both upgrades: ≈ ₹4260–5010** — still under the ₹5.5k ceiling.

## Sourcing links (verify live)
- ESP32-S3, SMPS, 28BYJ-48+ULN2003, caps, wire, screws → **[vishaworld.com](https://vishaworld.com)** (search each term).
- 64×64 P3 panel → [Robu](https://robu.in) · [rareComponents](https://rarecomponents.com/store/rgb-matrix-p3-64x64) (~₹2653).
- WS2812B ring, INMP441, 74HCT245, acrylic → Robu / Robocraze / Amazon India.

## What you also need (probably already own)
- USB-C cable for flashing the ESP32-S3.
- A computer with [PlatformIO](https://platformio.org) (VS Code extension) or Arduino IDE.
- A 3D printer, or a local printing service for the four parts in [`../hardware/`](../hardware/).
- A Spotify account (Free works for reading "now playing"; a phone/laptop plays the actual audio).
