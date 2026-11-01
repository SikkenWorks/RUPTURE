# RUPTURE V1.24 validation

- Native JieLi pi32v2 build: PASS, 196580-byte app, 384984 bytes app partition headroom.
- Full RUPTURE host regression suite: PASS.
- V1.24 dual-sub + all eight FX host stress (110250 PCM frames): peak 13886 / 32767, full-scale clips 0, active frames 109941; runtime input has CXS and CLASSIC subs, FX on max and changing sub blend.
- Native renderer: upper K1/K2 paint outside label y=50..80, header wordmark has negative-space crack; UI test PASS.
- CXS dry OFF parity and V1.9 / V1.10 sub-oscillator regressions: PASS.
- PJT33/project storage and all FX regressions: PASS in host emulation.
- Boot header and OTA verification: performed by release build script versus proven V1.20 package.
- Actual physical FM1 boot, frequency response, DMA deadline, loudness matching and glitch-free 30-minute stress: **NOT VERIFIED**. Software tests do not justify claiming absolute glitch-free operation.
