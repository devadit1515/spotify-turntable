// ─────────────────────────────────────────────────────────────────────────────
//  coupler.scad — motor-shaft → record riser.
//
//  Bottom: a D-shaped bore that grips the 28BYJ-48's flatted shaft (no slip).
//  Top:    a boss that press-fits the record's centre hole, lifting the record
//          clear of the deck so the motor hides underneath.
//  Print with the shaft opening on the bed (boss up) — no supports.
// ─────────────────────────────────────────────────────────────────────────────
include <params.scad>

// Negative shape matching a D-shaft: a cylinder with two opposite flats.
module d_bore(dia, flat, h) {
  intersection() {
    cylinder(d = dia, h = h);
    translate([0, 0, h/2]) cube([flat, dia + 2, h + 0.2], center = true);
  }
}

module coupler() {
  difference() {
    union() {
      cylinder(d = coupler_dia, h = coupler_h);                          // riser
      translate([0, 0, coupler_h])
        cylinder(d = coupler_boss_dia, h = coupler_boss_h);              // record boss
    }
    // grip the motor shaft (from the bottom)
    translate([0, 0, -0.1]) d_bore(shaft_dia + 0.3, shaft_flat + 0.35, shaft_len + 0.1);
    // small central relief up through the boss (ejector / glue escape)
    translate([0, 0, shaft_len]) cylinder(d = 2.5, h = coupler_h + coupler_boss_h);
  }
}

coupler();
