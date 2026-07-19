# Build guide

Work through this in order. Each phase ends with a **check** — don't move on until
it passes. That's how you avoid debugging five things at once.

---

## Phase 0 — Toolchain

1. Install [VS Code](https://code.visualstudio.com) and the **PlatformIO IDE**
   extension.
2. Open this folder (`spotify-turntable/`) in VS Code. PlatformIO reads
   [`platformio.ini`](../platformio.ini) and downloads the ESP32 toolchain +
   libraries on first build (a few minutes).
3. Plug in the ESP32-S3 over USB-C. Note the serial port PlatformIO detects.

**Check:** the PlatformIO toolbar (bottom of VS Code) shows a ✔ Build, → Upload,
and 🔌 port. Hit **Build** — it should compile the firmware with no errors (it
won't do anything useful yet without your credentials, but it must compile).

---

## Phase 1 — Spotify credentials + refresh token

The device reads your "now playing" through the Spotify Web API. That needs a
one-time login to mint a long-lived **refresh token**.

1. Go to the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard)
   → **Create app**. Name it anything. Tick **Web API**. Save.
2. Copy the **Client ID** and **Client Secret**.
3. Open [`tools/getRefreshToken/getRefreshToken.ino`](../tools/getRefreshToken/getRefreshToken.ino)
   in the Arduino IDE (install the **spotify-api-arduino** library by Brian Lough
   via Library Manager). Fill in WiFi + Client ID/Secret at the top. Upload.
4. Open Serial Monitor @ 115200. It prints the board's IP and the exact **Redirect
   URI** to register, e.g. `http://192.168.1.42/callback`.
5. Back in the dashboard, add that Redirect URI **exactly**. Save.
6. In a browser on the same WiFi, open `http://<board-ip>/`. Log in / authorise.
   The page and Serial Monitor print your **refresh token**.

**Check:** you have three strings — Client ID, Client Secret, and a long refresh
token.

> Security note: the client secret ends up in the ESP32's flash. That's fine for a
> personal device on your desk. If you ever share the hardware, switch to the
> Authorization-Code-with-PKCE flow (no secret on device) or put the token
> exchange behind a tiny Cloudflare Worker. Not needed for a portfolio build.

---

## Phase 2 — Configure + first light

1. Copy [`src/config.example.h`](../src/config.example.h) to `src/config.h` (a
   pre-filled `config.h` already exists — just edit it).
2. Fill in `WIFI_SSID`, `WIFI_PASSWORD`, and the three Spotify values.
3. Wire **only the panel + its 5V supply** for now (see
   [`wiring.md`](wiring.md)). Leave the motor, ring, and mic disconnected.
4. **Unplug the SMPS**, connect USB, and **Upload**. Then reconnect the SMPS to
   power the panel (USB alone can't).

**Check:** the panel shows the green **SPOTIFY / turntable** splash with blinking
dots. Then, once WiFi connects and you're playing something on Spotify (from any
device on your account), the album art appears with the scrolling title and a
green progress bar that advances. The Serial Monitor logs each poll and
`[art] new cover decoded` on track changes.

If the panel is blank or garbled, see [wiring.md → ghosting](wiring.md#if-the-panel-ghosts-smears-or-looks-shifted)
and confirm the 5V supply + common ground.

---

## Phase 3 — The spinning record

1. Power off. Wire the **28BYJ-48 → ULN2003 → GPIO 11/12/38/39** and the ULN2003
   `+/−` to the 5V rail and common ground.
2. Power up with a track playing.

**Check:** the motor spins slowly and steadily (~12 RPM) while music plays, ramps
up smoothly on play, and eases to a stop within a second of pausing. Put your hand
near it after it stops — the coils should be **off** (cool, no hum). Reverse the
direction by swapping the sequence order in
[`motion.cpp`](../src/motion.cpp) if you prefer the other way.

> The 28BYJ-48 tops out around 15 RPM — a true 33⅓ isn't physically possible with
> this motor, and the slower spin genuinely looks more premium. Tune `RECORD_RPM`
> in `config.h`.

---

## Phase 4 — The halo

1. Power off. Wire the **WS2812B ring**: DIN → GPIO 40, 5V → rail, GND → common.
2. Power up.

**Check:** the ring glows in the current cover's colour, breathing gently and
with a soft comet drifting around it while music plays; dim and still when paused.

**Optional audio-reactive:** solder the INMP441 (see wiring), set
`HALO_USE_MIC 1` in `config.h`, re-flash. The ring now pulses to the actual sound
in the room — clap near it to confirm.

---

## Phase 5 — Enclosure

Print the four parts and assemble per [`../hardware/README.md`](../hardware/README.md).
Measure your real panel + motor first and update
[`../hardware/params.scad`](../hardware/params.scad). Seat the frosted acrylic in
the frame's front recess — that diffuser is what turns the raw pixel grid into a
soft, premium glow, so don't skip it.

**Check:** everything mounts, the record clears the deck and spins without
rubbing, the ring sits flush in its channel, and the frame stands stable (add a
rear screw or kickstand if the tall panel feels tippy).

---

## Phase 6 — Tune + film

- Dial `MATRIX_BRIGHTNESS` for your room and camera (lower for video — bright LED
  matrices blow out on camera).
- Adjust `RECORD_RPM` and the halo feel to taste.
- Shoot the build using [`video-script.md`](video-script.md).

---

## Troubleshooting quick table

| Symptom | Likely cause | Fix |
|---|---|---|
| `no PSRAM detected` on boot | Board isn't N8R2, or memory type wrong | Set `board_build.arduino.memory_type` in `platformio.ini` to match your board |
| Splash forever, never loads art | WiFi or token wrong | Check Serial: WiFi IP? `token OK`? Re-run Phase 1 if refresh fails |
| Art loads but colours look swapped | JPEG endianness | Change `RGB565_BIG_ENDIAN` → `RGB565_LITTLE_ENDIAN` in `album_art.cpp` |
| Panel ghosts / smears | 3.3V logic on a long ribbon | `clkphase = true`, or add a 74HCT245 on CLK |
| ESP32 reboots on bright frames | Brownout | Bigger/closer cap, thicker 5V wires, lower brightness, don't power from USB |
| Motor buzzes but doesn't turn | Coil order | Swap two of the IN wires, or check the ULN2003 plug seating |
| Progress bar jumps around | Multiple devices controlling playback | Expected — it re-syncs each poll |
