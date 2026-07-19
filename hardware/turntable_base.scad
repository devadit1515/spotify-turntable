// ─────────────────────────────────────────────────────────────────────────────
//  turntable_base.scad — the deck that hides the electronics and carries the
//  motor, the LED-ring channel, and the slot the matrix frame plugs into.
//
//  Open-bottom box: drop the ESP32-S3 + 5V SMPS inside, route wiring, then close
//  with a laser-cut/printed bottom plate or just adhesive feet.
//
//  HONESTY NOTE: the 28BYJ-48 mount uses SLOTTED screw holes (±3 mm) because the
//  exact shaft-to-tab geometry varies between motors. Test-fit your motor and
//  nudge `motor_y` / the slot positions if needed. Everything else is driven by
//  params.scad.
// ─────────────────────────────────────────────────────────────────────────────
include <params.scad>

// Placement of the record/motor centre and the frame slot on the deck.
motor_x = 0;
motor_y = -22;                 // toward the front, so the frame sits behind it
frame_y = deck_d/2 - 12;       // frame slot near the back edge

top_z = wall_h + deck_top;     // z of the deck's top surface

module shell() {
  difference() {
    translate([-deck_w/2, -deck_d/2, 0]) cube([deck_w, deck_d, top_z]);
    // hollow interior (open bottom)
    translate([-(deck_w - 2*wall_t)/2, -(deck_d - 2*wall_t)/2, -0.1])
      cube([deck_w - 2*wall_t, deck_d - 2*wall_t, wall_h + 0.1]);
  }
}

// Boss under the deck to mount the stepper. Body clearance + two slotted ears.
module motor_mount() {
  bx = motor_x - motor_shaft_offset;   // body centre is offset from the shaft
  by = motor_y;
  // platform
  translate([bx, by, wall_h - 6])
    difference() {
      cylinder(d = motor_body_dia + 16, h = 6);
      translate([0, 0, -0.1]) cylinder(d = motor_body_dia + 1, h = 6.2);  // body pocket
    }
}

module motor_cutouts() {
  bx = motor_x - motor_shaft_offset;
  by = motor_y;
  // shaft passes up through the deck at the record centre
  translate([motor_x, motor_y, wall_h - 0.1]) cylinder(d = coupler_dia + 3, h = deck_top + 0.2);
  // two slotted M4 screw holes on the ear axis (±3 mm adjustment)
  for (s = [-1, 1])
    translate([bx, by + s * motor_tab_span/2, wall_h - 6.1])
      hull() {
        translate([-3, 0, 0]) cylinder(d = motor_tab_hole, h = 8);
        translate([ 3, 0, 0]) cylinder(d = motor_tab_hole, h = 8);
      }
}

// Recessed circular channel the WS2812B ring drops into, around the record.
module ring_channel() {
  translate([motor_x, motor_y, top_z - halo_channel_d])
    difference() {
      cylinder(r = halo_ring_dia/2 + halo_channel_w/2, h = halo_channel_d + 0.2);
      translate([0, 0, -0.1]) cylinder(r = halo_ring_dia/2 - halo_channel_w/2, h = halo_channel_d + 0.4);
    }
}

// Slot through the top plate that the matrix frame's tenon slides into.
module frame_slot() {
  translate([-frame_slot_w/2, frame_y - frame_slot_t/2, wall_h - 0.1])
    cube([frame_slot_w, frame_slot_t, deck_top + 0.2]);
}

// Notch in the back wall for the DC barrel jack + USB access.
module cable_notch() {
  translate([-18, deck_d/2 - wall_t - 0.1, 8])
    cube([36, wall_t + 0.4, 16]);
}

module base() {
  difference() {
    union() {
      shell();
      motor_mount();
    }
    motor_cutouts();
    ring_channel();
    frame_slot();
    cable_notch();
  }
}

base();
