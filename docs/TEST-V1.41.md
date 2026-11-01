# RUPTURE V1.41 — Channel EQ 2.0 audit

## Root causes

1. `rupture_eq2_publish` used `ru_eq2_auto_trim[sum(positive half-dB)]`. Four +9 dB bands gave a global -36 dB trim, contradicting the EQ graph and user intent. Even boosting only LOW attenuated unrelated frequencies.
2. `rupture_eq2_process` slewed 20 coefficients on every sample even after convergence. V1.41 reduces that work using cached status and an eight-sample update cadence during transitions.
3. `etch_fx_process_stereo_masked` considered saved EQ knobs during EQ bypass when deciding whether to invoke the wet output knee. OFF could be quieter than the same dry signal.
4. Newly initialized neutral EQ state needs unity detection at publication, not after several audio samples.

## Changes

- Global headroom trim is unity. The existing RBJ fixed-point biquad coefficients, shelf/bell frequency maps and user Q controls are unchanged. The output guard is nonlinear only near digital clipping; quiet/normal signal remains unattenuated.
- Active EQ coefficient slew is performed at approximately the same time scale but with fewer per-sample operations; once coefficients settle, no further coefficient slewing is performed. Neutral bands preserve sample history without full biquad MACs.
- Saved non-neutral EQ values cannot defeat a user-selected EQ bypass.
- No change to persistent PJT34 format, bootheader, OTA loader, global DMA guard or FX algorithms.

## Tests performed

- JieLi pi32v2 native build: PASS. App 229,936 bytes in a 581,564-byte application region (this is FLASH capacity, NOT audio CPU headroom).
- Original V1.34 static EQ response and engine tests: PASS.
- Original V1.39 dry audio/bypass regression: PASS.
- V1.41 80 Hz +9 dB boost: RMS amplitude ratio ~3.00 versus dry at normal level; full 4-band boost did not globally mute.
- V1.41 repeated frequency/Q/gain changes with all 4 bands: PASS.
- V1.41 stored non-neutral EQ controls with EQ bypass: exact stereo dry output in host simulation.
- UBSan integer/undefined operation check: PASS.
- V1.40 twelve stereo FX scenarios: PASS with original expected PCM.
- V1.24 multi-sub FX test and V1.38 mixed filter test: PASS.
- Broad historical regression up through V1.23: PASS before hitting the execution time limit. NOT claimed as a single completed full suite.
- WebInstaller, package CRC/SHA and bootloader protection: verified in release packaging.

## Limitations

- Actual DMA interrupts, available DSP cycles, real-device sound and USB jitter are not measurable in this host environment.
- Soft limiting above the peak threshold necessarily changes high-amplitude waveforms to prevent clipping. Below this threshold EQ boosts/cuts are not globally compensated.
- Back up projects before installing. Hardware listening test should try boosts, cuts, high Q, four-band combinations and bypass under normal output levels, then inspect `GLO → AUDIO DIAGNOSTICS` for LATE/GUARD/MIN SLACK.
