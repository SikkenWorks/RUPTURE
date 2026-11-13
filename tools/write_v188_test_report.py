from pathlib import Path
import hashlib,json,sys
R=Path(__file__).resolve().parents[1]
M=json.loads((R/'RELEASE-MANIFEST.json').read_text())
sha=lambda b:hashlib.sha256(b).hexdigest()
sys.path.insert(0,str(R/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
f=(R/'RUPTURE-V1.88.fwsc').read_bytes()
p=fw.parse_ufw(f);e={x['name']:x['data'] for x in p['entries']}
report=f'''# RUPTURE V1.88 — BEND Curve Animations: technical release validation

Date: 11 November 2026

## Delivery

- **RUPTURE V1.88** is a native FM1 application and installable `.fwsc` for product identity `FM-1_968`.
- Five unique 8-frame K3-linked **224 × 30 monochrome, 1-bit** sprite animations (40 total):
  - `LINEAR` — the **original V1.87 Mount Everest** (pixel-for-pixel identical).
  - `SOFT` — a giant angular **monolith** rupturing the earth.
  - `HARD` — an old three-masted **pirate flagship** bursting from a grassy meadow.
  - `S-CURVE` — **steady camera movement** from the flagship's cannon-studded hull, past the sails and crow's nest, up to its Jolly Roger.
  - `EXP` — colossal **Kraken** emerging from a catastrophic ground rupture.
- All except S-CURVE have a small prebudgeted UI-only ±1/2px 85ms shake while the bend is active. No shake on the close-up at any point.
- All frames are precomputed in Python **offline**, with an editable generator and individual native 1-bit PNGs plus 4×2 sprite sheets. At runtime only a ROM sprite lookup + the established packed-pixel LCD blitter are used.
- K1, K2, K3 curves, bend-release behavior and all audio signal routing are retained from V1.87. No added voices, effects, DSP load, buffers or audio interrupts.

## Native FM1 build

- **PASS**: JieLi/PI32V2 cross-compiler, storage + OTA + USB audio 48kHz target.
- **PASS**: Two separate deterministic builds produced the same native application SHA-256.
- Application length: **{M['app_bytes']:,} bytes**, leaving **{581564-M['app_bytes']:,} bytes** of the verified application region.
- Native application SHA-256: `{M['app_sha256']}`.
- `.fwsc` length: **{len(f):,} bytes**. SHA-256: `{sha(f)}`.
- App image entry: `0x02000120`, validated by bundled native build preflight.

## OTA / protected flash integrity

- **PASS**: `.fwsc` structure, entries, embedded checksums, product ID, same installation container size as V1.87.
- **PASS**: Prebuilt trusted OTA loader **byte-identical** to V1.87: SHA-256 `{sha(e['ota.bin'])}`.
- **PASS**: 16 KiB protected bootheader **byte-identical** to V1.87: SHA-256 `{sha(e['flash.bin'][:0x4000])}`.
- **PASS**: NUL-safe FM1 identity parser; can detect `FM-1_968` with Puya P25Q80 `0x856014` and UC25HQ80 `0xB36014`, without treating JEDEC bytes as part of device name.
- **PASS**: Simulated generic Windows MIDI port-name discovery sends read-only identity query; does **not** silently send UPGRADE/WRITE during discovery.
- **PASS**: WebInstaller and GitHub Pages index embed the identical V1.88 `.fwsc` bytes, matching SHA-256 and length.

## Audio, DSP and UI regression scope

- **PASS**: 16 critical CXS/SAW engine, audio mixer/DMA, native FX, MIDI, OTA, storage, flash-ID and ISR source files are byte-identical to V1.87.
- **PASS**: CXS + SAW K3 audio PCM tests: all five response curves; sustained notes do not retrigger and return to neutral when released.
- **PASS**: Previous LENGTH 16→4→16 sequencer DSP regression test.
- **PASS**: All 40 native sprite PNGs match the exact ROM bytes and 1-bit dimensions; total **33,600 bytes** of packed BEND sprite graphics.
- **PASS**: Eight LINEAR/Everest native pixels match the previously approved V1.87 art.
- **PASS**: 80 actual RUPTURE native 240×240 render previews generated: 5 curves × 8 frames × CRT ON/OFF.
- **PASS**: Renderer produces different shake positions for LINEAR, SOFT, HARD and EXP; S-CURVE close-up output is identical for all shake phases.
- **PASS**: The existing audio-priority LCD refresh gate is retained, limiting the extra UI painting to budgeted frames. The animated quake is capped to ~85ms intervals rather than a busy repaint loop.

## Limitations and physical validation

- **NOT TESTED**: Installation/boot on a physical FM1, real sound and LCD refresh timing, power-on speaker pop, recovery, storage cold reboot or OTA across all older firmware variants.
- UI-only shake can increase **LCD** updates while BEND is actively held, even though it adds **zero new processing to the audio engine**. The existing measured audio-first LCD budget is retained but device-specific zero DMA LATE/GUARD cannot be guaranteed without hardware testing.
- Back up projects and preserve a working FM1 recovery route before flashing. Initially test at a low speaker/output volume.
- Previous versions' cumulative test suite was not fully rerun; only the above targeted tests were exercised.
'''
(R/'RUPTURE-TEST-RESULTS-V1.88.md').write_text(report)
print('Wrote',R/'RUPTURE-TEST-RESULTS-V1.88.md')
