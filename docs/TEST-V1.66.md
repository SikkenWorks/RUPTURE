# RUPTURE V1.66 — SEL / Animation / SOUND SHAPE Verification

**Status:** software-tested native candidate; **not physically FM1-verified**. No physical device was accessible for this release.

## Fixes and observations

1. **Physical button labeling:** `SCL` replaced with `SEL` in the native 1-bit UI, help chapter labels and current user docs. There is no button scan remapping.
2. **Visual frame pacing:** the animation frame state advances independently of LCD paint; changes are latched until they are drawn. Time-based catch-up prevents cumulative lag from slower service calls. Severe measured DMA misses still cause the original protective throttling.
3. **PLAY animation:** 20 packed 1-bit frame source sprites remain untouched; note-on creates fissure, 10–12 loop while held with subtle one-pixel shake, note-off closes. The 3 other panels preserve held-note states, shake/loop, release closing.
4. **CXS WEIGHT:** low-shelf vs. high content transformation using the existing oscillator body-lowpass state. Increasing WEIGHT changes the sustained frequency balance, not only volume.
5. **CXS PUNCH:** bounded high-band accent and retained reduced attack transient. Does not go silent after the first 8 oscillator cycles. At 0, the PCM is bit-identical to V1.65.
6. **SAW route:** unchanged; original SAW shaping and alias guard preserved. Existing patches with nonzero WEIGHT/PUNCH intentionally have a different CXS tone; project format unchanged.

## Reproduced test outcomes

- Native FM1 application cross-compiled and memory-audited using the JieLi pi32v2 toolchain (`--storage --updater --usb-audio-48k`): success, application 318,956 bytes / 581,564-byte limit; 262,608 bytes of headroom.
- `test_v166_cxs_weight_punch.c`: pass at bass and mid pitches; sustained waveform, spectral balance, neutral PCM and fast parameter sweep verified.
- `test_v166_animation.c`: pass for PLAY/ENV/LFO/SOUND SHAPE hold/release/shake and retained delayed LCD frame, plus packed idle bitmap parity.
- `test_v147_saw_sound_shape.c`: pass for all SAW SOUND SHAPE knobs.
- `test_v155_cxs_pressure.c`: pass for sustained CXS pressure, all ranges.
- `test_v160_welcome.c`: pass for explicit format, cancel and no-backup flow.
- `test_v163_edit_safety.c`: pass for unsaved indication and parameter undo.
- `test_v164_lcd_budget.c`, `test_v164_system_info.c`: pass for input prioritization, idle throttling, and diagnostic status.
- `test_v165_sprite_mapping.c`: pass for 180 animation render combinations.
- `test_v165_png_parity.py`: pass for 60/60 original ENV/LFO/volcano 1-bit images.
- `test_v143_ota_reclaim.c` replaying actual V1.66 package: pass for accepted update, rejected corrupt/unknown staging, protected boot guard.
- FWSC verifier: product `FM-1_968`, application CRC valid. Protected flash 0x0000–0x3fff and OTA loader byte-identical to V1.65.
- Website/package embed and SHA-256 consistency: verified in site tests.

The complete historical suite was not rerun. Some other long-standing host storage tests require extra host dependencies and were not used as a pass claim.

## Physical acceptance checks still required

- Listen and measure CXS WEIGHT/PUNCH with sustained notes on the FM1, including high-resonance filters and full FX chains. Check clipping, aliasing, sudden parameter changes and performance.
- Check all four note-held animated panels for sustained loop/shake and smooth Note OFF closing while playing notes, sequencer and ARP, in normal and stressed modes.
- Save/reload projects with nonzero SOUND SHAPE values; confirm storage integrity, dirty marker and undo.
- Verify stable audio DMA during full UI activity, USB MIDI and 44.1/48 kHz USB-audio stress.
- Perform a safe, backed-up OTA cross-firmware transition and actual official V15 return on physical hardware, with a tested recovery path. The simulation and binary parity are **not** a proof of hardware recovery.
