# The Spotify Turntable

A desk object that shows what you're playing on Spotify — the album art glows as
live pixel art on a 64×64 RGB matrix, a 3D-printed record physically spins while
the music plays, and an LED halo breathes in the album's own colours.

Built on a single **ESP32-S3**, for about **₹4,000–5,000**. It's display-only —
your phone or laptop still plays the audio; this just makes it beautiful.

```
        ┌─────────────────────┐
        │ ▓▓▓  ALBUM ART  ▓▓▓ │   64×64 RGB matrix behind frosted acrylic:
        │ ▓▓▓  (pixels)   ▓▓▓ │   live cover art, fades in on track change
        │ Song ▪ Artist       │   scrolling title, Spotify-green progress bar
        │ ▰▰▰▰▰▱▱▱  1:42 3:05 │
        └─────────┬───────────┘
        ╭─────────┴────────────╮
        │   ✦  ◉ ◉ ◉  ✦        │   WS2812B halo — breathes / reacts to audio
        │  ✦  (  ⦿  )  ✦       │   3D-printed record — actually spins
        │   ✦  ◉ ◉ ◉  ✦        │   ESP32-S3 + 5V SMPS hidden in the base
        ╰──────────────────────╯
```

## Why it's not just another Spotify display

Plenty of builds do **one** of these — a glowing matrix, *or* a spinning-record
screen, *or* reactive lights. This combines all three into one object on a cheap
microcontroller. Glow + real motion + colour that tracks the music is what makes
someone stop and ask if you really built it.

## How it works

```
   ┌───────────── ESP32-S3 (dual core) ─────────────┐
   │                                                 │
   │  CORE 0                     CORE 1              │
   │  ─ WiFi + Spotify API       ─ render matrix 60fps│
   │  ─ refresh OAuth token      ─ spin the stepper   │
   │  ─ download album JPEG      ─ animate the halo    │
   │  ─ decode in 16px blocks ──▶ shared state ◀──────┘
   └─────────────────────────────────────────────────┘
```

- The **network + image decode** (the only slow, blocking work) lives on **core
  0**. The **display, motor, and lights** live on **core 1**. A 300 ms album-art
  download can't stutter the animation — they're on different CPUs.
- Full-res covers are ~800 KB and won't fit in RAM, so art is **decoded in 16×16
  blocks straight onto the panel** — no full-frame buffer.
- Playback is polled every ~3 s; the progress bar is **interpolated locally**
  between polls so it moves smoothly without hammering the API.

Source is small and modular — start at [`src/main.cpp`](src/main.cpp).

| Module | Does |
|---|---|
| [`spotify_client`](src/spotify_client.cpp) | Token refresh + `currently-playing` polling |
| [`album_art`](src/album_art.cpp) | HTTPS download + JPEG decode to a 64×64 buffer |
| [`display_ui`](src/display_ui.cpp) | Matrix rendering: art, fade-in, marquee, progress bar |
| [`motion`](src/motion.cpp) | 28BYJ-48 stepper — smooth spin on play |
| [`halo`](src/halo.cpp) | WS2812B ring — breathing or mic audio-reactive |
| [`playback_state`](src/playback_state.cpp) | Mutex-guarded state shared across cores |

## Quick start

1. **Hardware:** see [`docs/BOM.md`](docs/BOM.md) (~₹4–5k, mostly from Vishaworld +
   a panel from Robu/Amazon India).
2. **Get a Spotify refresh token:** run [`tools/getRefreshToken`](tools/getRefreshToken/getRefreshToken.ino)
   once (guided in the build guide).
3. **Configure:** copy `src/config.example.h` → `src/config.h`, fill in WiFi +
   Spotify.
4. **Flash:** open in VS Code + PlatformIO, hit Upload.
5. **Wire + build:** follow [`docs/build-guide.md`](docs/build-guide.md) phase by
   phase — each phase has a check so you're never debugging blind.
6. **Print the shell:** parts + settings in [`hardware/`](hardware/).
7. **Film it:** [`docs/video-script.md`](docs/video-script.md).

## Docs
- [Build guide](docs/build-guide.md) — start here, phased with checkpoints
- [Wiring](docs/wiring.md) — pin map, power, ghosting fixes
- [BOM](docs/BOM.md) — parts, prices, where to buy
- [Printed parts](hardware/README.md) — OpenSCAD models + assembly
- [Video script](docs/video-script.md) — portfolio clip + build video

## Built with

[ESP32-HUB75-MatrixPanel-DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA) ·
[spotify-api-arduino](https://github.com/witnessmenow/spotify-api-arduino) ·
[JPEGDEC](https://github.com/bitbank2/JPEGDEC) ·
[Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) ·
[ArduinoJson](https://arduinojson.org)

## License

MIT — see [LICENSE](LICENSE). Not affiliated with or endorsed by Spotify; uses the
public Spotify Web API. "Spotify" is a trademark of Spotify AB.
