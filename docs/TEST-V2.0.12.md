# RUPTURE V2.0.12 | GROWL validation and FM1 listening checklist

## Requested correction

The original ROLI reference has a repeated low note around D1 (36.7 Hz)
with prominent harmonics two through four. The user's later recording
shows V2.0.11 on a web FM1 emulator at approximately A2 (110 Hz) and
F#3 (185 Hz), with all four PLAY values visibly set to 100. The user
reports that the sound is not aggressive enough and the controls offer
too little timbral variation.

V2.0.12 replaces V2.0.11's gentle saturation with asymmetric saturation
plus a bounded mirrored wavefold. The two-state throat filter now has
a wider BRIGHT range. BODY and the four SHAPE controls have stronger
independent spectral effects. The seven original bandlimited carrier
wavetable levels remain; no copyrighted recording is embedded.

## Host validation, not hardware DMA verification

- Jieli pi32v2 application build: 540,728 bytes; limit 581,564 bytes.
- App headroom: 40,836 bytes.
- OTA loader: unchanged, SHA-256
  `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`.
- Protected bootheader: unchanged, SHA-256
  `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`.
- GROWL runtime state: two 32-bit words, unchanged from V2.0.11.
- GROWL control coefficient cache: 20 bytes on the render stack, updated
  per audio block, not per PCM sample.
- All GROWL audio processor operations: bounded integer multiply,
  shift, addition and branch; no sample-loop division, float or malloc.
- GROWL sweep: six separate two-second D1 timbres in the included WAV;
  all 1,536 extreme setting/pitch combinations bounded.
- Every control independently changes sustained PCM from its 50 setting
  to its 100 setting: BRIGHT 23.1%, EDGE 88.5%, BODY 34.5%,
  LEVEL 46.0%, SHAPE EDGE 37.2%, WEIGHT 25.3%, BITE 63.5%
  and PUNCH 11.1% RMS relative to the unchanged reference signal.
- D1 sustained six-scene headroom: peak 23,081 / 32,767;
  full-scale sample count 0.
- GROWL + two subs + very low 4-band Channel EQ + HALL for 132,300
  PCM samples: peak 26,256 / 32,767; full-scale sample count 0.
- V2.0.11 reference wavetable harmonic ratios unchanged by source edits.
- Original CXS, both suboscillator types, effects, PRESETS, project
  persistence, FM1 cross-chip flash acceptance and OTA loader unchanged.

Host timings are not hardware DMA measurements. The user had previously
reported DMA LATE increases with EQ, SUBs and reverb. Therefore V2.0.12
is **test firmware**, not a hardware-verified release. A physical FM1
test is essential. Keep the last stable firmware and recovery method.

## Listening demonstration

`GROWL-EXPRESSIVE-DEMO.wav` plays a steady D1 in six consecutive
2-second configurations:

1. Dark, clean.
2. Fully bright, clean.
3. Fully bright, EDGE=100.
4. EDGE=100, BODY=100.
5. EDGE and BITE/SHAPE formant=100.
6. All GROWL SHAPE controls at 100.

The demo is synthesized locally, not extracted from the source video.
It is a **dry oscillator preview**; a real FM1 will add filter, ADSR,
patch level, and optionally suboscillators and FX.

## Real FM1 acceptance test

1. Back up presets/projects; keep a known-good flash/recovery package.
2. Install V2.0.12 on the FM1 only when recovery is available.
3. Open SYSTEM INFO and note the **DMA LATE** count.
4. Disable all FX and SUBs. Test GROWL at D1, A2 and F#3 and sweep
   BRIGHT 0–100, EDGE 0–100 and BODY 0–100, then test each SHAPE
   parameter separately. Confirm clearly distinct timbres.
5. Hold a note for 30 seconds with EDGE/BITE high. Confirm no
   crackle unrelated to the intended distortion and no increasing DMA LATE.
6. Enable two SUBs, lower the Channel EQ frequency values, enable
   HALL reverb and hold a note for another 30 seconds. Record DMA LATE
   before and after.
7. Confirm CXS and both SUB SAW/SQUARE are still identical in operation
   to prior versions; verify patch save/load and PRESETS.

The test passes on physical FM1 only if **DMA LATE does not increase**
through these scenarios and notes are free of unintended discontinuities.
Hardware results are not asserted here.
