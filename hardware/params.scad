// ─────────────────────────────────────────────────────────────────────────────
//  params.scad — shared dimensions for every printed part. Edit HERE, once.
//
//  Almost everything is driven off measured hardware. The two things you MUST
//  verify with calipers on your actual parts before printing the base/frame:
//    • panel_w / panel_h / panel_depth  (your 64×64 module's real size)
//    • the 28BYJ-48 mounting-tab hole spacing (usually 35 mm)
//  Defaults below are typical values for a P3 64×64 panel + a stock 28BYJ-48.
// ─────────────────────────────────────────────────────────────────────────────

// Global smoothness
$fn = 120;

// ── RGB matrix panel (P3 64×64) ──────────────────────────────────────────────
panel_px     = 64;
panel_pitch  = 3.0;                 // P3 = 3 mm pixel pitch
panel_w      = panel_px * panel_pitch;   // ≈ 192 mm (measure yours!)
panel_h      = panel_px * panel_pitch;   // ≈ 192 mm
panel_depth  = 14;                  // module thickness incl. LEDs (no connectors)
panel_border = 2;                   // dead PCB border around the lit area

// ── Acrylic diffuser ─────────────────────────────────────────────────────────
diffuser_thick = 2.0;               // frosted acrylic sheet thickness
diffuser_gap   = 3.0;               // air gap panel→acrylic (the "glow" distance)

// ── 28BYJ-48 stepper ─────────────────────────────────────────────────────────
motor_body_dia   = 28.5;            // round body
motor_tab_span   = 35.0;            // centre-to-centre of the two mounting holes
motor_tab_hole   = 4.2;            // M4 screw clearance (tabs are ~4.2 mm)
shaft_dia        = 5.0;             // output shaft
shaft_flat       = 3.0;             // across-flats of the D
shaft_len        = 9.0;             // usable shaft length
motor_shaft_offset = 8.0;           // shaft is offset this far from the body centre

// ── Coupler (motor shaft → record riser) ─────────────────────────────────────
coupler_dia      = 14;              // outer riser diameter
coupler_h        = 14;              // lifts the record above the deck
coupler_boss_dia = 8;               // top boss that plugs into the record
coupler_boss_h   = 4;

// ── Record ───────────────────────────────────────────────────────────────────
// Sized to nest INSIDE the 24-LED ring (~86 mm OD, ~68 mm inner hole) with a
// glowing halo gap all round.
record_dia    = 60;
record_thick  = 2.4;
label_dia     = 22;                 // printed-label recess
label_depth   = 0.6;
record_hole   = coupler_boss_dia + 0.3;  // press-fit onto the coupler boss

// ── WS2812B halo ring (24-LED, ~86 mm OD) ────────────────────────────────────
halo_ring_dia   = 74;               // LED-circle diameter of the 24-LED ring (measure yours!)
halo_channel_w  = 12;               // channel width — holds the ~9 mm-wide ring PCB
halo_channel_d  = 4;                // channel depth

// ── Turntable base / deck ────────────────────────────────────────────────────
// Deck must be wider than the panel (≈192 mm) so the frame slot fits and the
// whole thing doesn't tip. 210 gives a comfortable margin either side.
deck_w      = 210;                  // footprint width  (> panel_w for stability)
deck_d      = 150;                  // footprint depth
deck_top    = 3;                    // top plate thickness
wall_h      = 45;                   // internal height to hide SMPS + ESP32 + wiring
wall_t      = 2.4;                  // wall thickness
frame_slot_w = panel_w + 2*panel_border + 2;   // slot the matrix frame plugs into
frame_slot_t = 8;                   // slot depth (front-to-back) — matches frame tenon
