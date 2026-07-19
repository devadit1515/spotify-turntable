# Printed parts (OpenSCAD)

All parts are parametric. **Edit [`params.scad`](params.scad) once** — every other file reads from it. Open a part in the free [OpenSCAD](https://openscad.org) app, press **F5** to preview / **F6** to render, then **Export → STL** and slice.

> ⚠️ Before printing the base and frame, measure your **actual** 64×64 panel (`panel_w`, `panel_h`, `panel_depth`) and your **28BYJ-48 tab spacing** with calipers and update `params.scad`. The defaults are typical, not guaranteed.

| Part | File | Print orientation | Supports | Notes |
|---|---|---|---|---|
| Vinyl record | `record.scad` | Grooved side **up** | No | Center hole press-fits the coupler boss. Optional 2-colour label. |
| Coupler / riser | `coupler.scad` | Shaft opening **on bed** | No | D-bore grips the motor shaft; boss lifts the record. |
| Turntable base | `turntable_base.scad` | **Open side down** | Minimal (motor boss) | Houses ESP32-S3 + SMPS. Slotted motor holes give ±3 mm fit. |
| Matrix frame | `matrix_frame.scad` | Front face **on bed** | No | Captures the acrylic; panel inserts from the back. |

## Recommended slicer settings
- **Material:** PLA (PETG for the base if it'll sit in sun/heat).
- **Layer height:** 0.2 mm. **Walls:** 3 perimeters. **Infill:** 15–20 %.
- **Record:** print slow on the top layers for crisp grooves; a smooth/textured PEI bed gives a nice finish.
- **Coupler:** 4 perimeters (it takes the motor torque). If the D-bore is tight, scale bore clearance in `params.scad` (`+0.3` → `+0.4`).

## Assembly order
1. Screw the **28BYJ-48** to the base's motor boss (slotted holes). Shaft pokes up through the deck's centre hole.
2. Press the **coupler** onto the shaft, then press the **record** onto the coupler boss (a dab of glue if loose).
3. Drop the **WS2812B ring** into the circular channel; feed its 3 wires down into the cavity.
4. Mount the **panel** into the **frame** from the back; seat the **acrylic** in the front recess. Retain the panel with tape/foam or a back cover.
5. Plug the frame's **tenon** into the base slot (panel facing the front). Add a rear screw or kickstand if it feels tippy — the panel is tall.
6. Mount **ESP32-S3** and the **5V SMPS** inside the base; route the panel ribbon + power up to the frame. Close the bottom with a plate or adhesive feet.

See [`../docs/wiring.md`](../docs/wiring.md) for the electrical side and [`../docs/build-guide.md`](../docs/build-guide.md) for the full phased build.
