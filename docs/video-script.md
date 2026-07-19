# Filming: the portfolio clip + the build video

Two deliverables from one shoot. The **hero clip** is the 3–4 second loop that goes
in your maker portfolio. The **build video** is the longer YouTube walkthrough.

## Shooting notes (read first)
- **Lower `MATRIX_BRIGHTNESS` to ~90–110 for camera.** LED matrices that look
  right to the eye blow out to white blobs on video. Test on your actual camera.
- Shoot in a **dim room** so the glow reads and the halo pops. One soft side light
  on the record so its grooves catch light; let the panel light itself.
- **Manual exposure + manual white balance.** Auto everything will hunt and
  flicker against the LEDs.
- **Shutter ~1/50–1/120.** Too fast and you'll photograph the matrix's PWM scan as
  dark bands. If you see rolling bands, nudge shutter until they vanish.
- Shoot **4K** even if you deliver 1080p — you'll want to crop/reframe.

---

## The hero clip (3–4 s, silent loop for the portfolio)

Goal: in four seconds, a stranger thinks *"wait, they built that?"* Motion +
glow + the album art syncing is the whole story. No text needed.

| t | Shot | Detail |
|---|---|---|
| 0.0–1.2s | **Slow push-in**, low angle, record spinning in foreground, glowing panel soft-focus behind | Start with the record filling frame, panel bokeh behind it |
| 1.2–2.6s | **Rack focus** from the record to the panel as the album art **fades in** on a track change | Time it so a new track lands here — the fade-in sells "it's live" |
| 2.6–4.0s | **Rise + settle** on the full object: panel sharp with art + scrolling title, record spinning, halo breathing | End on the wide "beauty" frame; make the last frame match the first so it loops |

Trigger the track-change fade by skipping to the next song on your phone right
before the 1.2s mark. Do several takes and keep the one where the fade lands
cleanly.

**Deliver:** a seamless 4s loop, no audio (portfolios autoplay muted), graded
slightly cool with lifted blacks so the glow feels premium.

---

## The build video (longer walkthrough)

Structure that keeps people watching and shows engineering judgement — the part
admissions readers actually care about.

1. **Cold open (0:00–0:10):** the finished object doing its thing. Hook first.
2. **The idea (0:10–0:40):** "A turntable that shows what's playing on Spotify —
   glowing album art, a record that actually spins, lights that react." Say what
   makes it different: most builds do *one* of glow / spin / reactive; this does
   all three on a ₹5k ESP32.
3. **How it works (0:40–2:00):** the real content. Walk the data path on screen:
   *ESP32 asks Spotify what's playing → downloads the cover → decodes it in
   blocks so it fits in memory → paints the matrix, while the second CPU core
   spins the motor and drives the lights.* Show the serial log scrolling. This is
   where you demonstrate you understand your own system.
4. **Build montage (2:00–4:00):** wiring the panel, the getRefreshToken step,
   first light, adding the motor, the enclosure printing + assembly. Timelapse the
   prints. Show one thing that went wrong and how you fixed it (ghosting → clock
   phase, or a brownout → bigger cap). Honesty reads as competence.
5. **Payoff (4:00–4:40):** skip through a few tracks, each cover fading in, colours
   shifting the halo. Let it breathe.
6. **Outro (4:40–end):** cost, "code and print files are linked," what you'd do
   next (custom PCB, bigger panel). Point to the repo.

### B-roll worth grabbing while you have it apart
- Macro of the ribbon cable seating into the panel.
- The refresh-token web page on your phone.
- First-light reaction (leave the camera running the first time art appears).
- Timelapse of each part printing.
- Close macro of the fade-in transition and the progress bar advancing.

### On-screen / voiceover beats that signal engineering
- "Full album art is 800 KB — too big for the chip's memory — so it's decoded in
  16-pixel blocks straight onto the panel."
- "Networking runs on one CPU core, the display and motor on the other, so a slow
  download never stutters the animation."
- "The record's a stepper — a real 33 RPM is beyond it, so it runs a slower spin
  that honestly looks better."

Keep it to your own words on camera — the above are the *ideas* to convey, not a
script to read.
