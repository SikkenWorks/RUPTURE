# RUPTURE V1.48: SAVE routing & SAW high-frequency alias reduction

## Reported issues
1. Physical SAVE opened PROJECTS only from the four HOME pages, not other routine editing screens.
2. SAW exhibited a high whistle which becomes more apparent with BRIGHT/EDGE/BITE/PUNCH turned up.

## Confirmed source-level causes
- `rupture_save_route.h` returned `RU_SAVE_NO_MENU` for FILTER, FX, EQ2, GLO and other ordinary pages. In `bringup.c` a secondary excluded-page list also prevented navigation.
- The 256-point/7-band main SAW table was bandlimited **before** waveshaping. Strong DRIVE/BITE/PUNCH and hard-clipped peaks introduce nonlinear harmonics which can alias into non-harmonic high-frequency tones. A host analysis demonstrates this energy and reproduces the effect's dependence on strong shaping. This may explain the reported whine; physical device verification is still required.

## Changes
- `ru_save_destination()` returns PROJECTS for every ordinary UI ID, including GLO, EQ, FILTER, FX, ARP and all SETTINGS/STORAGE pages. SEQ and SEQ SLOTS remain separate sequence save menus. The NAME editor retains SAVE-to-commit. Transfer, SAFE MODE, loading and intentionally disabled legacy pages are excluded for safety.
- `ru_saw_guarded_read()` smoothly blends existing 64/32/16-harmonic SAW tables as nonlinear shape strength increases; coefficient computed once per audio render block. No additional oscillator, wavetable, RAM buffer, variable division inside the regular audio sample path, runtime floating-point, heap or ISR flash operation.
- CXS, SUB SAW/SQUARE, EQ, FX and all native project records left structurally unchanged. The trusted bootheader and OTA loader stay byte-identical to ETCH V1.88 reference.

## Reproducible host measurements, strongly driven SAW
At fundamental frequency ~151.267 Hz, 44.1 kHz, 90,000 sustained samples with Hann-windowed FFT and all SAW shaping controls 100, non-harmonic spectral energy relative to full waveform power:

| Band | V1.47 | V1.48 | Difference |
|---|---:|---:|---:|
| 2–7 kHz | −45.55 dB | −62.63 dB | −17.08 dB |
| 7–12 kHz | −43.49 dB | −64.90 dB | −21.41 dB |
| 12–20 kHz | −38.39 dB | −55.44 dB | −17.05 dB |

The RMS changed from 29,720.5 to 29,707.5 Q15 units in the strongest saturation test. Neutral SAW was proven sample-for-sample identical with V1.47 over 700,000 samples. The harmonic guard intentionally reduces upper harmonic content only at high shaping: this is a sonic tradeoff, not a claim of zero change.

## Tests executed
- JieLi native FM1 crossbuild: PASS, app **242336 B / 581564 B**, headroom 339228 B.
- 49 native screen ID SAVE destination audit: PASS (39 project destinations, 2 sequence destinations, 8 special-name/OTA/SAFE/disabled exceptions).
- Prior V1.21 physical SAVE router host test: PASS with updated correct destinations.
- V1.48 700,000 neutral parity and dynamic waveshaper source samples, 7-band boundedness with UBSan: PASS.
- V1.45 oscillators/SQUARE/SAW sub/screensaver, V1.46 SAW knobs, V1.47 sustained SOUND SHAPING: PASS.
- V1.41 bass/EQ, V1.42 OCT FUZZ, V1.38 filters: PASS.
- Historical full regression suite: started and passed audio/MIDI/storage through V1.7 but exceeded execution time; not reported as fully passed.
- Firmware packaging: validated CRC, product identity and protected boot/OTA comparison; WebInstaller/website hash tests executed by release script.

## Hardware validation outstanding
The exact human-perceived whine, FM1 audio amplifier behaviour, knob event latency, all combinations of SAVE input and DMA slack require physical checks. Back up projects and retain a tested recovery route before installation. RUPTURE's DMA safety system is not disabled.

## Firmware integrity
- Version: 1.48
- FWSC bytes: 610086
- FWSC SHA-256: `ceb711f8cc1b72f0147c35a15060abf99eae5111d07f7d0eadf79e4d3e9df8fa`
- App SHA-256: `5c71605c7d8b54d06af48e0630f4825480d5862889b8f2a0abc37f148a08569f`
