<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Assets

This directory stores reusable fonts, images, music, sound effects, and battery profiles, organized by asset type.

Keep each asset in the matching subdirectory and document its destination, naming, integration method, and source/license. Do not mix binary assets with Markdown documentation.

## Fonts

Store reusable font files and generated font sources in `fonts/`.

- Use descriptive names that include the family, weight, size, and format when relevant.
- Document the source, license, character range, conversion command, and expected destination.
- Check Flash and internal-RAM impact before adding a font; the ESP32-C3 has no PSRAM.
- Do not commit fonts whose license does not permit redistribution.

### Countdown Bomb Chinese font

- `fonts/bomb-ui-subset.ttf`: retained Noto Sans SC Regular subset from Google Fonts; source URLs and SHA-256 are in `fonts/bomb-font-source.json`.
- `fonts/bomb_font_18.c`: 18 px, uncompressed 4 bpp LVGL 9 font covering all Chinese text in `main/bomb_text.h` and printable ASCII: 149 glyphs and 14,084 bitmap bytes.
- The font and derived glyph data use [SIL OFL 1.1](fonts/OFL.txt), retaining Adobe's copyright. The application font is named Bomb UI.
- `fonts/bomb-font-inventory.json` records code points, metrics, and per-glyph pixel hashes.
- Normal firmware builds do not need Pillow. To regenerate, use an isolated Python environment with Pillow 12.3.0:

```sh
python tools/generate_bomb_font.py
python tools/check_bomb_font.py
```

`--fetch` explicitly retrieves a text-subset TTF and OFL from Google Fonts; the
default uses the retained source. The checker reparses emitted C data and checks
every pixel, the actual cmap, and a missing-glyph negative control. All states
still require Chinese rendering checks on the device. Previews use real LVGL
software rendering and are not photographs.

## Images

Store reusable source images and generated display assets in `images/`.

| File | Dimensions and format | Use and source |
| --- | --- | --- |
| [`images/countdown-bomb-preview.png`](images/countdown-bomb-preview.png) | 768 × 720, PNG | Software-rendered snapshots of the application's real LVGL pages: setup, maximum time, countdown, wires, success and explosion. Not a device photo. |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160, JPEG | Product hero image embedded in both project README files to foreground AI Passport and its open, maker-oriented identity. |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724, PNG RGBA | Optional technical infographic retained as a reference asset; it is no longer used as the homepage hero. Generated for this repository with the built-in image generation tool on 2026-09-17; the six labels and values were checked against the documented hardware contract. |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336, PNG RGBA | Transparent black wordmark extracted from the repository's original `images/logo.png`; embedded in both project README files for light backgrounds. |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336, PNG RGBA | White version of the extracted wordmark, used by the README `<picture>` element when GitHub is in dark mode. |

- Use descriptive names and document dimensions, pixel format, conversion steps, and destination.
- Prefer formats suitable for the 240 × 320 RGB565 display and account for Flash and internal RAM.
- Preserve editable sources where licensing permits, and record the source and license.
- Never commit device QR secrets, credentials, or personal data in images.

## Music and sound effects

Store reusable music and sound-effect sources in `music/`.

- Document the source, license, sample rate, bit depth, channels, conversion command, and destination.
- Prefer 16 kHz, 16-bit mono PCM when it matches the current BSP audio path.
- Check Flash and internal-RAM cost before embedding audio; stream or chunk long recordings.
- Do not commit media without redistribution permission.

### Countdown Bomb explosion

- [`music/xm4167.mp3`](music/xm4167.mp3) is the user's supplied explosion recording,
  originally named `xm4167.mp3`: 1.896 s, MP3, 48 kHz stereo. Its SHA-256 is
  `2a0d66f873746fbc1db034a48e950493c47fb9608efdb4dde6f2ab7978c5526d`.
  No separate license was supplied. It is not covered by the project's code MIT license.
- [`music/bomb-explosion.pcm`](music/bomb-explosion.pcm) is the embedded, signed
  little-endian 16 kHz / 16-bit mono version: 61,440 bytes, 1.920 s. The entire
  source is retained without trimming or speed changes; peaks are attenuated by
  0.878188 to a maximum of 24,000, never boosted. Existing trailing silence plus
  alignment padding provides more than 128 ms of silent DMA drain at the end.
  PCM SHA-256: `7f174cd62f72279e94dac4e7b134c593a30b38ee75ae1e8d3ca5ab1fe5d0081d`.
- [`music/bomb-explosion.wav`](music/bomb-explosion.wav) contains exactly the same
  PCM for host preview; the firmware embeds only the raw PCM in read-only Flash.
- [`music/bomb-explosion-source.json`](music/bomb-explosion-source.json) records
  format, hashes, gain, sample counts, and the FFmpeg 8.0.1 version used.
  Regenerate offline with `python3 tools/prepare_bomb_explosion.py` from the repository
  root using local FFmpeg; normal firmware builds require no FFmpeg or MP3 decoder.
- `main/bomb_sound.c` streams 512 samples per write with a 1 KiB buffer on an
  independent worker, once per wrong-wire or timeout explosion, at output volume 75%.
  The result screen lasts 2.4 seconds. Actual onset, loudness, tail, and repeated
  concurrent animation/playback require listening on the card.

## Battery profiles

[`battery/wanli-500mah-profile2-xil40x-20261020.txt`](battery/wanli-500mah-profile2-xil40x-20261020.txt)
contains the supplied CW2017 profile for the Wanli New Energy 500 mAh cell
(profile2, XIL40X, 2026-10-20). It is an unchanged copy of the project-supplied
file, retained at the maintainer's request for this firmware update. Its
original filename is recorded in the paired Chinese document. No separate
license was included in the supplied file.

The format is 80 lines of two hexadecimal digits, one byte per line in register
order. The BSP embeds those bytes in `components/bsp/src/bsp_battery.c`; the
host gate checks the source file and initializer for an exact match. Text-file
SHA-256: `7ea96eba4634f4495a37c25ffacd54c1deaa0693b162c892d671a2ba68460919`.
Decoded 80-byte SHA-256:
`fa7365671093ef449c5bdcfb4adb279e3d275f888109f50b6b2f21e9b96220d5`.
This is a fuel-gauge profile, not a charging configuration or a generic profile
for every 500 mAh cell; see the [hardware guide](../docs/hardware-design/AI_HARDWARE_DEVELOPMENT_GUIDE.md#9-cw2017-fuel-gauge).
