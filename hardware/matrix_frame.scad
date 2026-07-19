// ─────────────────────────────────────────────────────────────────────────────
//  matrix_frame.scad — upright frame for the 64×64 panel + frosted diffuser.
//
//  Modelled "as installed" (standing up, front face toward +Y). Layering, front
//  to back, captures the acrylic and holds the panel with a built-in air gap:
//
//     [front bezel]  window 193  ─┐
//     [acrylic recess 199]        │  acrylic trapped between two 193 ledges
//     [gap window 193]  ←gap──────┘  panel front stops on this ledge
//     [panel pocket 197]  (open at the back — insert panel here)
//
//  A full-width tenon under the bottom edge plugs into the base's frame slot.
//  PRINT: lay it face-down (rotate so the front face is on the bed); the tenon
//  then points sideways — fine, no supports needed for the pockets.
// ─────────────────────────────────────────────────────────────────────────────
include <params.scad>

bezel    = 8;
front_t  = 3;

module_w = panel_w + 2*panel_border;
module_h = panel_h + 2*panel_border;

window_w  = panel_w + 1;        // reveal the lit area, hide the PCB border
window_h  = panel_h + 1;
acrylic_w = window_w + 6;       // 3 mm retaining lip all round
acrylic_h = window_h + 6;
pocket_w  = module_w + 1;       // holds the panel module
pocket_h  = module_h + 1;

outer_w = pocket_w + 2*bezel;
outer_h = pocket_h + 2*bezel;
frame_depth = front_t + diffuser_thick + diffuser_gap + panel_depth;

tenon_w = frame_slot_w - 2;
tenon_h = 12;

// A rectangular hole of size (w×h) in the X–Z plane, cut through a Y span.
module yslot(w, h, y0, ylen) {
  translate([-w/2, y0, (outer_h - h)/2]) cube([w, ylen, h]);
}

module frame() {
  difference() {
    // solid blank
    translate([-outer_w/2, 0, 0]) cube([outer_w, frame_depth, outer_h]);

    // through light tube (193): serves the front window AND the gap window
    yslot(window_w, window_h, -0.1, frame_depth + 0.2);

    // acrylic recess (199) in region 2
    yslot(acrylic_w, acrylic_h,
          frame_depth - front_t - diffuser_thick, diffuser_thick + 0.05);

    // panel pocket (197) from the back, region 4
    yslot(pocket_w, pocket_h, -0.1, panel_depth + 0.1);
  }

  // tenon under the bottom edge (drops into the base slot)
  translate([-tenon_w/2, frame_depth/2 - frame_slot_t/2, -tenon_h + 0.1])
    cube([tenon_w, frame_slot_t, tenon_h]);
}

frame();
