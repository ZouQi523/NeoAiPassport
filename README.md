<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Countdown Bomb — NeoAiPassport

An offline countdown-and-defuse experiment for FoloToy AI Passport, developed on
`feature/countdown-bomb` using the official firmware layout and BSP.

## Play

1. On the home screen, a short UP/DOWN press adds/subtracts one minute on release.
   Holding for about 0.5 seconds adjusts five minutes once, without an extra short step.
   The default is five minutes; the range is 1–180 minutes.
2. Press OK to start. The remaining time uses three fixed segments, minutes : seconds : hundredths
   (for example `04:31:86`, with 10 ms per final digit step), against a monotonic deadline.
   All three groups use equal-width centered columns with equal spacing.
3. Press OK again to open the three-wire page. The countdown continues there.
4. UP cuts red, DOWN cuts yellow, and OK cuts blue. One wire is randomly selected
   as correct for the round, giving an uninformed choice a one-in-three chance.
5. A correct choice shows success. A wrong choice or expiry plays the explosion
   animation with the supplied explosion sound. Success returns home after 1.8 seconds;
   explosion returns after 2.4 seconds with the selected duration,
   waiting for another start press.

Each adjustment gesture produces one short or long action. Start and wire
selection act immediately on depression; holding or releasing never repeats them. The press that opens the wire page does not cut a wire.
Power cycling resets to five minutes. The home screen dims after 30 seconds idle;
a running countdown remains visible. The home screen keeps the application title above the timer, +/− controls, a red play icon, and battery level.
Timer digits have no unit captions or instruction text. Wrong-wire
choices and expiry play the supplied explosion effect once through the speaker.

## Implementation and verification

- `main/bomb_game.*`: hardware-independent game and startup-release guard.
- `main/bomb_ui.*`: a redesigned LVGL interface, independent from the demo UI.
- `main/bomb_sound.*`: queued, chunked explosion playback on an independent worker.
- `components/bsp/`: official display, ADC button, codec and battery drivers, reused unchanged.
- `assets/fonts/`: licensed Chinese subset and emitted glyph/pixel checks.
- `tests/bomb_ui/`: the same interface rendered on the host for visual checks.

[Requirements, defaults, tests and firmware identity](docs/assets/countdown-bomb.md)
contain the full handoff. Screenshots are software-rendered previews, not device
photos. A successful host test/build does not establish on-device acceptance.
The previous Microphone project remains stopped.

## Build

Read [AGENTS.md](AGENTS.md), [official documentation](docs/README.md), and the
[local environment record](docs/assets/readme.md). ESP-IDF **5.5.3** is required.
Run from this root after activation:

```sh
./tools/validate.sh
```

The gate runs repository checks, emitted-font checks, host tests, firmware
compilation, merged-image validation, and matching debug-archive retention.
It produces `build/FoloToy-AI-Passport-full.bin` for flashing at **0x0**.
A merged flash replaces the current application and may reset stored settings;
flashing requires explicit approval for the exact image and target.

The original repository history and `origin` are preserved. Tools, backups and
build outputs remain ignored. Nothing is committed, pushed, or published automatically.

## License

Original project code uses [MIT](LICENSE), retaining upstream copyright.
The Chinese font and derived glyphs use [SIL OFL 1.1](assets/fonts/OFL.txt).

The explosion recording was supplied by the user; its provenance is recorded in
[asset notes](assets/README.md#countdown-bomb-explosion).
