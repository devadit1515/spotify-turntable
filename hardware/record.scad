// ─────────────────────────────────────────────────────────────────────────────
//  record.scad — the 3D-printed vinyl record.
//
//  Print grooved-side UP (as modelled). No supports needed — the centre hole is
//  a plain vertical bore. Optional: print the label area in a 2nd colour, or glue
//  on a printed-paper label. Press the record onto the coupler's top boss.
// ─────────────────────────────────────────────────────────────────────────────
include <params.scad>

groove_count  = 20;      // decorative concentric grooves (tuned for a 60 mm disc)
groove_w      = 0.35;    // groove width
groove_depth  = 0.4;
groove_margin = 4;       // gap kept clear at the label edge and the outer rim

module groove(r) {
  translate([0, 0, record_thick - groove_depth])
    difference() {
      cylinder(r = r + groove_w/2, h = groove_depth + 0.1);
      translate([0, 0, -0.05]) cylinder(r = r - groove_w/2, h = groove_depth + 0.2);
    }
}

module record() {
  r_in  = label_dia/2 + groove_margin;
  r_out = record_dia/2 - groove_margin;
  difference() {
    // disc
    cylinder(d = record_dia, h = record_thick);

    // label recess (top)
    translate([0, 0, record_thick - label_depth])
      cylinder(d = label_dia, h = label_depth + 0.1);

    // centre hole (press-fit onto coupler boss)
    translate([0, 0, -0.1]) cylinder(d = record_hole, h = record_thick + 0.2);

    // grooves
    for (i = [0 : groove_count - 1]) {
      r = r_in + (r_out - r_in) * i / (groove_count - 1);
      groove(r);
    }
  }
}

record();
