# RUPTURE V1.40 — FX headroom and audio-stop investigation

## Reported physical symptoms
The author reports persistent crackle and DMA DEADLINE audio stops on V1.39 when enabling multiple effects, especially OCT FUZZ and DRIVE combinations, and sometimes after toggling DRIVE off. The FM1 device was not attached during this investigation. Physical clearance has not yet been demonstrated.

## Reproducible code defect
The previous global FX fast bypass worked only if every effect was dry. On the first active effect, `native_fx.c` entered the shared wet chain. The chorus/delay/reverb conditions used `!bypassed || smoothed_mix`, which is **true even with MIX 0 whenever a bypass bit is clear**. Consequently, their complete delay-line, reverb and coefficient DSP ran unnecessarily on every sample while OCT FUZZ, DRIVE or another unrelated effect was enabled.

## V1.40 corrections
1. CHORUS DSP is now executed only while its slewed mix is nonzero. The existing slew finishes naturally when toggled off.
2. DELAY DSP is now executed only while its slewed mix is nonzero, retaining audible tails during the fade.
3. REVERB DSP is now executed only when its actual target or smoothed mix is nonzero.
4. The RUPTURE eight-effect pathway skips unrelated dormant ETCH modulation DSP when no saved legacy effect is active or fading. Legacy active processing is preserved.
5. COMPRESSOR return-to-unity after bypass is smoothed instead of being instant (avoids a click); fully dry exact PCM is recovered afterward.
6. A NEW project selects EQ2 in bypass/OFF state and initializes its gains neutrally. Previously saved projects retain their persisted EQ mode, bypass mask and coefficients.
7. Existing audio DMA deadline protection, target layouts, storage serializer, native 24 dB ladder, bootheader and OTA writer are untouched.

## Host validation
- Native JieLi pi32v2 build (`--storage --updater --usb-audio-48k`): PASS, app 229,888 B of 581,564 B (351,676 B flash headroom); this is **not DMA CPU budget**.
- V1.40 12-scenario stereo PCM comparison to original V1.39: 12 × 40,000 samples, bit-for-bit identical, covering dry, OCT FUZZ, REVERB, DELAY, CHORUS, COMPRESSOR, TILT, SANDPAPER and combinations.
- Undeclared/undefined arithmetic: UBSan run with complex effects, PASS.
- V1.39 EQ2 unity/active/bypass regression: PASS.
- V1.15 active FX, V1.18 all-FX headroom stress (peak 26,102 Q15, zero clipped samples, zero host dropouts), V1.22 FX loudness and V1.24 dual-sub + FX tests: PASS.
- Newly added compressor bypass-release test: PASS, no instant gain reset, dry PCM restored after slew.
- Firmware integrity: complete .fwsc CRC passed; WebInstaller and website embed **exactly the same package bytes**, checksummed with SHA256.
- First 0x4000 bytes of protected bootheader and `ota.bin` SHA-content are unchanged relative to V1.39.
- The broad historical suite `tests/run_tests.sh` is separate; its result is documented only if it completes.

## Critical physical acceptance criteria
On a recoverable FM1, back up projects before updating. First test the known V1.39-failing patch at FILTER OFF and EQ OFF, then separately activate DRIVE, OCT FUZZ, COMPRESSOR, TILT, CHORUS, DELAY, REVERB and SANDPAPER. Then combine effects and repeatedly bypass/unbypass each. Record GLO > AUDIO DIAGNOSTICS `LATE`, `GUARD`, `MIN SLACK` with USB cable in/out and sequencer playing. These must show no accumulated deadline faults and nonzero slack. Long audio soak and real timing measurements are required to consider physical crash resolution confirmed. Stop on repeated DMA faults: do not bypass the safety mechanism.

No claim of on-device validation or physical crash elimination is made in this build report.

## Completed historical regression run
The complete `sh tests/run_tests.sh` run finished with **exit code 0**. It includes storage, native UI, old-effect audio, EQ2/PJT34 and 2,500,000 cached-filter reference samples. The log is included in the complete source package as `docs/REGRESSION-V1.40.log`.
