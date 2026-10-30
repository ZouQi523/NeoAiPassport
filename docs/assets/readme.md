<p align="right">
  <a href="readme.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# NeoAiPassport project notes

[Repository overview](../../README.md) · [Official development documentation](../README.md)

These notes describe this project's requirements and integration decisions.
They are separate from upstream hardware and engineering documentation.

## Current application: Countdown Bomb

The offline countdown-and-defuse experiment is developed on `feature/countdown-bomb`; see [application notes](countdown-bomb.md).

## Microphone: stopped by the user

On 2026-10-30 the user stopped this application. The requirements below are
historical context, not an active implementation plan. Current work is to
understand and validate the official baseline and build/flashing workflow.

### Previously accepted requirements

- Use a wired connection between the AI Passport and the Mac.
- Capture speech with the card's microphone and use Doubao Input Method for
  transcription and text entry. A new speech-recognition service is not requested.
- One button starts capture while held and stops on release, matching Doubao's
  hold-to-talk operation.
- Another button starts continuous capture on a click and stops on the next
  click, matching Doubao's continuous-speaking operation.
- Show the current microphone capture state with an icon on the device screen.
- Keep the third button's function undecided.

### Unresolved at cancellation

- Whether a Mac background companion and virtual audio device are acceptable.
- Doubao's actual shortcuts, installed version, and compatibility with relayed
  audio and generated keyboard events.
- Physical button assignments, the long-press threshold, and interactions between
  held capture and continuous capture.
- Disconnect/error behavior, third-button behavior, and preservation of existing
  device functions or stored data.

The USB Serial/JTAG port was detected on the Mac. No serial audio transfer,
system microphone input, keyboard triggering, or Doubao transcription has been
validated on this device. USB presence does not establish USB Audio/HID support.

## Layout migration: 2026-10-30

At the user's request, the custom `projects/` organization was removed in favor
of the official root-level firmware layout. The unused `shared/` scaffold was
also removed. Original workspace files were saved in an ignored local migration
backup, and the existing Git history and `origin` were retained.

The baseline migration was prepared on `feature/official-baseline` (renamed from `feature/microphone`
when the application was stopped). This branch was created from the existing
NeoAiPassport history; it is not a branch descended from upstream `main`.
The initial migration was prepared as an uncommitted source snapshot, without
committing, pushing, publishing, or writing the device. Later application checkpoints
and device tests require their own authorization and are recorded separately. The strict upstream
fork workflow's clean-`main` ancestry has not been established; changing Git
history is a separate decision.

### Source identity and license

- User-selected entry: [FoloToy/ai-passport on Gitee](https://gitee.com/FoloToy/ai-passport).
- Cross-checked source: [FoloToy/ai-passport on GitHub](https://github.com/FoloToy/ai-passport).
- On 2026-10-30, a Gitee Git query confirmed the same `main` commit at both entries.
- Revision: [`006c2279852e352a8433b2c21e5e09be2ca36f77`](https://github.com/FoloToy/ai-passport/tree/006c2279852e352a8433b2c21e5e09be2ca36f77).
- Source commit date: 2026-10-21 UTC.
- Archive: [fixed-revision source archive](https://codeload.github.com/FoloToy/ai-passport/tar.gz/006c2279852e352a8433b2c21e5e09be2ca36f77).
- Archive SHA-256: `ede47b920e231e48bc8e9c158db4284da104b2b571216a530736cc4cec0ab068`.
- License: [MIT](../../LICENSE); the original FoloToy copyright is retained.
- Required firmware environment: ESP-IDF 5.5.3 for ESP32-C3, with dependencies
  pinned by `dependencies.lock`.

The root README pair and these notes are project-specific. Local adjustments to
the imported files are recorded here: project instructions in the AGENTS pair,
additional ignore rules, and an additional project copyright in LICENSE.
Two host-tool portability adjustments preserve the same checks on macOS:
`tools/install-actionlint.sh` compares SHA-256 output without GNU-only long
options, and `tools/validate.sh` uses Apple's `-dead_strip` linker option on Darwin
while retaining GNU `--gc-sections` elsewhere.
The device application and BSP remain the upstream hardware-test baseline.

## Reference implementations

- [Mac Voice Remote](https://github.com/ihonghong/ai-passport-macos-voice-remote):
  macOS companion, virtual microphone, and physical shortcut controls over BLE.
- [Input Bridge](https://github.com/MindMobius/ai-passport-input-bridge):
  USB/BLE audio and input-method shortcut bridging, primarily for Windows.
- [Folo AI Passport Voice](https://github.com/zhaohuaxiaoy/folo-ai-passport-voice):
  USB/BLE streaming and a separate cloud transcription service.

These repositories are research references only. Their application code has not
been imported, installed, or tested in NeoAiPassport.

## Official baseline environment and validation

The following records the completed official baseline validation. The current
application is linked above; Microphone remains stopped. Development tools remain under ignored `.toolchains/`;
installation and activation do not edit global Git, shell, or package-manager configuration.

- SDK: ESP-IDF v5.5.3, annotated tag object `b31fcc7a314a44ad992b58f589f7d1d8a4fadff6`,
  resolving to commit `2c211b236707889e8400c4dc5644dd5c4ee071e0`, checked against the official GitHub tag.
- The SDK root uses Espressif's official China Git service; submodules use the SDK's pinned commits.
- Tools and the Python environment are installed under `.toolchains/espressif/`, using Python 3.12.
- The five official skills are enabled through ignored, repository-local relative links in `.agents/skills/`.
- The full official gate `./tools/validate.sh` passed: repository checks, host tests, firmware compilation, merging, and archive validation. Device validation has not run.

### Activate the local isolated environment

Run from the repository root; this affects only the current terminal:

```sh
export IDF_TOOLS_PATH="$PWD/.toolchains/espressif"
export IDF_PYTHON_ENV_PATH="$IDF_TOOLS_PATH/python_env/idf5.5_py3.12_env"
. .toolchains/esp-idf-v5.5.3/export.sh
idf.py --version
./tools/validate.sh
```

This uses the project-local tools and Python environment. It does not flash the
device or edit shell startup files.

### Verified firmware from this run

- Type: official hardware-test baseline, with the device application and BSP unchanged.
- Build: PASS; Host tests: PASS; Device tests: NOT RUN.
- Unverified: on-device boot, display, three buttons, audio, battery, wireless, and low-power behavior.
- Firmware version: `ddeae51-dirty`, identifying the then-uncommitted baseline workspace, not an upstream release.
- Merged image: `build/firmware/47a6ce35f63f4e0cad8a9247e19c586394737935074ce0eaad06397146d77fee/FoloToy-AI-Passport-full.bin`, 1,593,152 bytes, intended flash offset `0x0`.
- Full-image SHA-256: `47a6ce35f63f4e0cad8a9247e19c586394737935074ce0eaad06397146d77fee`.
- Matching ELF SHA-256: `66494830d26d971df1a32e66732e8f2eec51269ea8e41d8dc16ac8fa04baf94b`.
- Debug archive: `build/firmware/47a6ce35f63f4e0cad8a9247e19c586394737935074ce0eaad06397146d77fee/`; its `manifest.json`, ELF, MAP, and images were independently reverified.
- `dependencies.lock`, `sdkconfig.defaults`, `partitions.csv`, `main/`, and `components/` match the pinned upstream snapshot.
- Read-only enumeration found the device's serial port during the build. No port was opened, no firmware was flashed, and no device storage was erased or accessed.

Artifacts remain in ignored local build storage; nothing was committed, pushed,
or published. Flashing this baseline replaces the current application and the
merged image may reset NVS settings; device validation requires separate approval.
