# RUPTURE V1.55 — PRESSURE and OTA audit (host verification)

## Change scope

- Native CXS `native_cxs_q15.c`: PRESSURE 0..100 now shapes the eight segment levels, additionally to existing cycle-exchange segment durations. The extra calculations execute on oscillator cycle wrap, **not on each sample**. There are no new oscillator voices, buffers or storage fields.
- The corresponding standalone `src/native_cxs_q15.c` host reference was kept in sync.
- No changes to `audio_core.c`, `usb.c`, `ota.c`, UI rendering or the DAM V3 assets. Firmware identity remains `FM-1_968`.
- **CPU caveat:** 8 small extra amplitude calculations occur per CXS cycle. This is not zero cost and real hardware DMA headroom was not measured. No new per-sample DSP stage was introduced.

## Positive tests

- Native JieLi pi32v2 crossbuild with `--storage --updater --usb-audio-48k`: PASS.
- Native firmware app: **260,908 bytes**, vs V1.54 **260,872 bytes** (**+36 bytes**), 320,656 bytes available app headroom under the linker limit.
- `tests/test_v155_cxs_pressure.c` (UBSan): PASS at ~130 Hz and ~519 Hz, verifies different sustained PCM responses at PRESSURE 10/25/50/75/100; live control sweeps and parameter extremes preserve the 32,768-unit oscillator period.
- PRESSURE 0: original V1.54 CXS PCM matches **after the first oscillator cycle**. The first cycle uses the built-in default PRESSURE=58 until the active patch's PRESSURE value is applied, as in the existing initialization flow.
- V1.45 main oscillator, SAW/suboscillator and screensaver test: PASS.
- V1.48 SAW harmonic guard test: PASS.
- DAM V3 animation timing: PASS; exact 20/20 PNG-to-native bitmaps: PASS.
- Native V1.43 app-only OTA packet replay using the new **V1.55 fwsc**: PASS. The updater accepts the valid app-only container and safely refuses corrupt/unknown loader and boot data.
- `.fwsc` verifier, CRC and product identity: PASS.
- First **0x4000 bytes of flash.bin** (protected bootheader) and exact incoming **ota.bin** (trusted staged loader) are byte-identical with V1.54. Flash content from **0x93000** to the end is also byte-identical. Source-level `ota.c` and `usb.c` are identical to V1.54.
- Website source template preserves repaired manual-confirm UI and the independent JS selection/right-click blocker. `index.html` and `WebInstaller.html` contain the **same byte-identical** embedded V1.55 firmware as `RUPTURE-V1.55.fwsc`; checksum, size, website release checks and Node syntax: PASS.

## Limitations / open checks

- The full historical test suite **did not pass**. It progressed through V1.26, then stopped at the V1.27 UI test (`count_pixels(166,110,14,14)>0`). Both that test and `rupture_ui.c` are unchanged compared with V1.54. This is not evidence of a new PRESSURE UI change, but full-suite coverage remains incomplete.
- Original official stock **V15** external firmware replay was **not rerun** because the exact official V15 firmware fixture is not bundled/available in this workspace. The native updater and protected artifacts remain unchanged, but this is not end-to-end proof of stock recovery.
- No physical FM1 was available for actual flashing, audio listening, DMA/IRQ time measurement, cross-firmware transition, reconnect, or recovery testing.
- New CXS sound can be louder at high PRESSURE; test/adjust master gain and listen for clipping on actual hardware.

**Flash precaution:** Backup all projects first, verify the physical recovery method, and never treat a passing OTA container CRC or host simulation alone as proof of safe hardware flashing.
