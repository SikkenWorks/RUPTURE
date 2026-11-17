# RUPTURE V2.0.14 — GROWL LATCH / RETURN

## V2.0.14 (2026-11-17)
- EXPRESSION=0 no longer removes the GROWL bass. Gradual body and presence
  floors keep the voice audible across negative expression values.
- LATCH retains the gesture. RETURN moves it back to 50 after 125ms.
- Both assignments appear in GROWL PERFORMANCE, persisted as IDs 94/95.
- No new per-sample DSP, extra audio buffer, flash sector or OTA loader.
- Firmware cross-compiled, host regressions pass; FM1 hardware unverified.

# RUPTURE V2.0.13 — GROWL expression performance

## V2.0.13 (2026-11-16)

- Two independent device-global PRESETS encoder mappings: original CXS
  assignment preserved; GROWL defaults to **EXPRESSION**.
- GROWL PERFORMANCE category added to the existing settings picker, after
  a new CXS/GROWL mode selector. Both assignments are visible in the chooser.
- The 0–100 Expression gesture co-modulates formant positions, brightness,
  body, nonlinear drive, clipping asymmetry and harmonic folds in one motion.
  Midpoint 50 leaves every GROWL coefficient untouched.
- Performance movement is temporary, not a patch edit or flash operation.
  Smoothing and coefficient changes occur once per DMA buffer.
- Backward-compatible 8-byte NOR preferences record now saves BOTH CXS
  and GROWL assignments. CRC, interrupted writes, project bank changes and
  global-settings journal compaction covered by new tests.
- Firmware cross-compiled; physical FM1 DMA LATE and sound validation remain
  unverified.

# RUPTURE V2.0.12 — More aggressive, more expressive GROWL

## V2.0.12 (2026-11-16)

- Compared the original 19-second Seaboard reference against the new
  five-second V2.0.11 recording. The new demonstration sounds too restrained
  according to the user; its knobs are visibly set to extreme positions.
  The newly played notes are also higher in pitch than the recurring D1 in
  the original clip.
- Reworked GROWL's two-stage vocal filter and introduced a strong, bounded
  asymmetric wavefold and drive circuit to add genuine rasp, not just volume.
- Greatly expanded timbral contrast across BRIGHT, EDGE, BODY and the four
  SHAPE controls, preserving the existing settings/preset structure.
- Moved GROWL's parameter conversion out of the 44.1-kHz PCM callback's
  per-sample inner loop; cached 20-byte parameters once per audio block.
- No extra oscillator, delay ring, reverb state, sample playback, float,
  dynamic memory or division inside GROWL's PCM processor.
- Original CXS, SUB SAW/SQUARE, EQ, FX, MIDI, PRESETS and OTA retained.
- Added separate expressive control sweep and combined dual-SUB/low-EQ/HALL
  regression tests. Real FM1 CPU/DMA LATE testing still required.

# RUPTURE V2.0.11 — New reference-inspired GROWL main oscillator

## V2.0.11 (2026-11-16)

- Analyzed the user's 19.27-second Seaboard Block video. The repeated deep
  note is approximately D1 (36.7 Hz), and its second through fourth
  harmonics dominate the fundamental. The source includes other sounds,
  compression and performance articulation, so this is a spectral guide.
- Replaced only the MAIN SAW oscillator mode with **GROWL**, a deterministic
  seven-level, 256-point bandlimited harmonic waveform generated offline.
  No recording data or sample playback is stored in firmware.
- Added two lightweight fixed-point one-pole states to animate the low-mid
  'throat' when BRIGHT changes, plus bounded drive/weight/bite/punch shaping.
  No realtime trigonometry, division, allocation, new voices or PCM rings.
- Retained the same CXS/GROWL 64-sample live transition, all eight existing
  parameter controls, and the saved main-oscillator project bit, so legacy
  SAW patches select GROWL automatically after update.
- Original CXS, SUB SAW/SQUARE, 8-slot FX, three reverbs, low-CPU Channel
  EQ, project settings, PRESETS, LED wave and the trusted OTA remain intact.
- Passed six-second sustained GROWL and all 256 extreme knob combinations
  with undefined-behavior checks. Another 132,300-sample test drives GROWL,
  both SUB oscillators, low-frequency Channel EQ and HALL simultaneously:
  no full-scale clipping. All previous software regression tests pass.
- Sound resemblance and worst-case DMA LATE must still be verified on FM1.

# RUPTURE V2.0.10 — Channel EQ sustained-note DMA LATE hotfix

## V2.0.10 (2026-11-15)

- Physical FM1 report: Channel EQ at low/minimum values crackles and
  increments DMA LATE, even with just one band enabled.
- Found that the silent eight-insert FX bypass was rejected by edited EQ
  gain fields, including the fourth HIGH AIR field. Non-neutral EQ alone
  could therefore run the full ordered rack / legacy render path at 44.1kHz.
- Added a dedicated guarded EQ-only path, with the four original RBJ biquads,
  exact source coefficients, no extra audio allocation, and a bounded
  per-sample flush of residual reverb/delay buffers after FX disable.
- Fixed inconsistent peak safety: EQ settings no longer choose between a
  guarded soft-knee route and an unguarded hard-clipping route.
- Eight low-frequency/Q/gain scenarios verify 6,400,000 baseline and saved
  control samples with equal PCM; reverb -> EQ transition stays PCM-exact.
- Host comparison: EQ-only ~50 ns -> ~25 ns/sample in typical single-band
  cases, with native pi32v2 machine code and full firmware build audited.
- Retained CXS, SUB, reverb engines, 8 FX inserts, PRESETS, LED wave, project
  and global storage and trusted OTA from V2.0.9.
- DMA LATE zero under real hardware combinations is not yet verified.

## V2.0.9 (2026-11-15)

- Physical FM1 report: DMA LATE events rose rapidly on sustained notes in
  V2.0.8 with sub-oscillators and reverb. Treat V2.0.8 as regressed.
- Inspected actual Jieli pi32v2 target disassembly, not just host CPU math.
  V2.0.8's conditional 32-bit CXS blend made `rupture_q15_sample` grow
  from 478 to 568 instruction bytes, adding branches in each exchange.
- Restored V2.0.7 CXS exchange machine code and the original complete
  Spring/Room/Hall reverb algorithms. Removed unsafe ROOM unrolling and
  bypass-fade arithmetic changes from V2.0.8.
- Kept only a demonstrably smaller reverb mixer for guaranteed clipped
  ordered-insert PCM (66 -> 18 target instruction bytes), with six exact
  mixed-FX Spring/Hall/Room golden PCM comparisons.
- Optimized both active sub-oscillators' pitch increment with native paired-
  register 64-bit multiplication in place of the V1.79 manually split and
  heavily guarded 32-bit path. The isolated FM1 implementation shrinks
  from 202 to 106 instruction bytes; no table, buffer or quality changes.
- 2,154,880 bit-identical SUB increment comparisons including octaves,
  detune, drift and integer overflow. Existing CXS, FX and OTA regressions
  remain required before this build is distributed.
- Actual DMA LATE reduction and final headroom must be checked on the FM1.

## V2.0.8 (2026-11-15)

- Removed the normal-case 64-bit weighted exchange division from the hot
  CXS cycle path. Preserved its legacy unsigned-excitation behavior exactly,
  including all internal oscillator states.
- Sub-oscillator CXS comparison: 352,000 sample-exact samples and 90,870
  original versus optimized exchange cycles, all states identical.
- Ordered reverb wet/dry interpolation and per-insert fade now use bounded
  signed 32-bit products and exact signed division, with no 64-bit fallback
  in their normal audio path.
- FM1 compiler fully unrolls ROOM's four FDN feedback-line loops; all lines,
  damping and stereo reflections remain present.
- Mixed eight-slot FX regression for Spring, Hall and Room: six independent/
  legacy configurations, each with 60,000 PCM frames including live bypass,
  slot moves and parameter changes, exactly matching V2.0.7.
- No audio quality change, no added RAM/delay buffers, no removed features.
  OTA loader, bootheader, global PRESETS and boot LED wave are unchanged.
- No physical FM1 DMA margin test has been run. Reduction of real DMA LATE
  events remains a hardware test objective, not a verified claim.

## V2.0.7 (2026-11-15)

- Fixed the definitive PRESETS failure: old handler existed only after the
  unconditional RUPTURE main-loop `continue`, so it was unreachable.
- PRESETS hardware encoder 6 now reaches its globally assigned parameter
  editor on every nonzero detent in the live 25 ms RUPTURE input service.
- Previously implemented name-and-value popup now appears when actually
  rotating, including off-screen parameters such as Scale Guide ROOT.
- SEQ + PRESETS pattern rotation is also restored in the RUPTURE handler.
- PRESETS wakes the screensaver; the active SETTING assignment chooser keeps
  its own input and never inadvertently changes a parameter or sequence.
- Native build and tests added for both dispatcher behavior and placement
  before the early continue. Audio PCM, storage codec, trusted loader,
  firmware bootheader, and boot LED wave unchanged.


## V2.0.6 (2026-11-15)

- PRESETS knob: actual parameter value and name appear in a non-overlapping
  foreground overlay for 1.55 seconds on every recognized detent, including
  assignments to parameters on another page (such as Scale Guide ROOT).
- Physical encoder 6 (PB7 matrix row) and assignment-to-editor path audited.
  The global NOR-backed mapping still survives project switching and reboot.
- Bootup: 27 keys followed by 14 function LEDs light successively, with a
  traveling brightness wave that settles into normal page LED behavior.
  Physical key presses and protected states cancel the visual immediately.
- LED effect is entirely outside PCM and timer-scan interrupts. Existing PC6
  analog-gate power-rail timing guard remains active during the LED wave.
- Host native build and regression tests run; on-device behavior and DMA
  timing still require a real FM1 check.


## V2.0.5 (2026-11-15)

- SEQUENCER: move HOLD and SLIDE metadata entirely above the four encoder
  graphics, for both empty and populated steps (including MIDI octave -1).
- PRESETS encoder: nine ordered categories, 94 stable assignable controls,
  including Scale Guide ROOT; BPM remains the default after an upgrade.
- PRESETS assignment persists independently of project and preset files in
  its own CRC-checked global NOR record. Project rewrites, catalog actions,
  global preference compaction and torn writes have new storage regressions.
- On leaving the selection modal, display WAITING/SAVING/SAVED with a
  progress bar. Do not touch flash during playback or active voice tails.
- Audio: replace the deterministic initial CXS eight-segment exchange in
  the note-on ISR with its byte-identical precomputed ROM state. This avoids
  five multi-division oscillator inits per attack with both SUBs enabled.
  A 262,144-sample comparison of the original and new algorithms passes.
- Full FM1 build and host regressions; actual physical DMA timing still
  requires a device test. OTA loader and protected bootheader are unchanged.

## V2.0.4 (2026-11-15)

- BEND: 44.1 kHz Q16 pitch smoothing prevents encoder-induced stepping on
  both upward bends and release, regardless of GLIDE mode.
- GLIDE: six saved curves LINEAR/EXP/LOG/SOFT/FAST/FIRM, packed into the
  reserved bits of the existing PJT35 per-preset GLIDE settings.
- FX browser: ambience (REVERB, DELAY), modulation (CHORUS, WAH WAH),
  drive (MOSFET, DISTORTION, OCTAVE FUZZ, SANDPAPER), dynamics/tone
  (COMPRESSOR, TILT EQ). Stored type IDs and old effect chains unchanged.
- DISTORTION: bass-tightened high-gain asymmetric hard-clipping metal voicing,
  tone lowpass and original wet/dry. Not a duplicate of MOSFET overdrive.
- ROOM/HALL: refined damping and Schroeder allpass / denser early reflections
  in existing 4096-word tanks; SPRING and reverb buffers unchanged.
- SUB engine: cache identical pitch/blend calculations when parameters stay
  unchanged, use 3 boundary tests instead of 7 for CXS selection and take an
  exact signed divide-by-4 fast path for the subs' fixed EDGE=50 smoothing.
  SUB waveform samples, phases and mixing remain bit-identical.
- Software QA and native build completed; actual FM1 DMA LATE behavior still
  requires on-device measurement.

## V2.0.3 (2026-11-15)

- SUB OSC 1 turns toward the viewer once, then holds original frame 12
  throughout sustain. A gentle 1-pixel whole-panel jitter replaces the
  looping facial/eye frames.
- Releasing the last held note resets the image and shake immediately;
  the next note starts the original turn again.
- SUB OSC 2 animation and original sprite sources remain unchanged.
- No change to audio/DSP, autosave/GLIDE, loading screen, firmware loader,
  recovery, flash compatibility or project formats.
- Verified in host tests. Physical FM1 test pending.

## V2.0.2 (2026-11-15)

- Increased SIGNAL BREACH loading cadence from 50 ms to 35 ms across the first 22 frames, then holds the final RUPTURE V2 image for 1000 ms (previously 780 ms).
- Installed the user-supplied 16-frame native 1-bit 224×30 SUB OSC 1 fear-face animation and the independent 16-frame SUB OSC 2 close-up.
- While a real note is held: one-time turn through frames 2–12, then a continuous eye-tremor loop through frames 13–16. On release: immediate return to still frame 1; no animation when silent or when another screen is active.
- Kept all supplied sprite pixels, existing SUB controls and knob positioning, 1-bit panel blitter, audio engine, preset SAVE/AUTO SAVE and trusted OTA unchanged.
- Added source-pixel parity and note-gate/loop/release regression tests. Hardware validation is still pending.

---

# RUPTURE V2.0.1 — smoother SIGNAL BREACH loading animation

## V2.0.1 (2026-11-15)

- Smoothed the boot sequence from 12 to 23 full-screen 1-bit frames, adding eleven ordered-dither transition frames between pixel-exact original art.
- Regularized display timing to 50 ms per animation frame, followed by a 780 ms logo hold; kept the nominal total loading-animation delay at 1880 ms.
- Preserved the existing 7200-byte framebuffer and lossless RLE storage; no new runtime interpolation, DSP work, or audio buffers.
- Reverified that all four per-preset GLIDE parameters are captured through the same manual SAVE and guarded AUTO SAVE snapshot and codec. No change to their existing project format or flash-write safeguards.
- Built a new FWSC using the byte-identical trusted OTA loader and protected bootheader. Physical FM1 verification remains pending.

---

# RUPTURE V2.0 — SIGNAL BREACH loading animation

## V2.0 (2026-11-15)

- Replaced only the loading screen with the 12 exact 240×240 1-bit Signal Breach sprites supplied for RUPTURE V2.0.
- Lossless compressed 86,400 raw frame bytes to 46,340 ROM bytes; frames decode into the already allocated UI framebuffer, with no additional audio/DSP processing or framebuffer.
- Frame timing follows the asset's 1,880 ms sequence, including the final 800 ms hold. The startup animation is shorter than the previous 2,019 ms splash.
- Preserved all synth and GLIDE features, SAVE/AUTO SAVE, SCALE GUIDE, screensaver and other animated pages, flash support and trusted OTA. Automated parity and boot artwork tests added. Physical FM1 validation remains pending.

---

# RUPTURE V1.99 — note-gated GLIDE animation + GUIDE labels

## V1.99 (2026-11-15)

- GLIDE tunnel walking and found-footage shake advance only while a synth note is held. The idle frame is stationary and the camera resets after note release. Hardware, MIDI, sequencer and arpeggiator note sources use the existing synth voice gates.
- SCALE GUIDE K3 now shows OFF, GUIDE, STRICT or LOCK instead of raw 0–3 values. Encoder order and existing project mode values remain intact.
- Preserved V1.98 four-parameter GLIDE SAVE/AUTO SAVE and all unaffected UI/audio paths. Added focused host regression tests; no extra DSP or flash writes.


## V1.98 (2026-11-15)

- Stage all four GLIDE settings (TIME MS, MODE, CURVE, ENV RETRIG) directly in the currently edited preset's existing project bank on each actual change.
- Preserve pending changes during fast preset switching, including older projects until they can be safely saved. The shared NOTE MEMORY glide-time control also updates the same preset bank.
- Continue using the single existing CRC-verified project write path for both explicit SAVE and guarded AUTO SAVE; no new flash partitions, project records, or flash write on knob turn.
- Prevent no-op GLIDE knob turns at a limit from unnecessarily restarting the autosave timer.
- Retain the V1.97 GLIDE found-footage tunnel, all other screens, all FX, synth/audio processing, and protected OTA loader unchanged. Hardware validation is still pending.

---

# RUPTURE V1.97 — GLIDE railway tunnel POV

## V1.97 (2026-11-15)

- Added the twelve approved **224×30, 1-bit** railway-tunnel frames to **GLIDE only**, progressing toward the distant light.
- Applied subtle found-footage camera shake (up to 2 pixels) every ~85 ms; animation advances one keyframe after three shake ticks. The picture shifts but the GLIDE controls do not.
- Reused the packed bitmap blitter; 10,080 extra bytes of static flash ROM, **zero extra audio PCM buffers or per-sample DSP work**.
- Left all four GLIDE parameters, the audio engine, FX, project saving, LEDs, MIDI, all other animated pages and safety/OTA paths intact. Firmware build, host regression suite and loader integrity audit are included; FM1 physical playback has not yet been tested.

---

# RUPTURE V1.96 — STRICT scale mode

## V1.96 (2026-11-15)

- Added **STRICT** after GUIDE in scale mode selection. Order: OFF, GUIDE, STRICT, LOCK. Presses outside the chosen scale are silent, do not light performance LEDs, and cannot be recorded as live sequencer notes.
- Blocked out-of-scale USB/TRS MIDI note-ons in STRICT, retaining note-off handling. GUIDE still allows all notes, and LOCK still quantizes out-of-scale notes.
- Preserved old saved project mode values: LOCK remains encoded as 2; STRICT encodes as 3. No project format expansion.
- Built the 27-key filter once per DMA buffer instead of adding DSP per audio sample. Oscillator, FX, EQ and GLIDE algorithms remain unchanged.
- Added audio PCM, MIDI, LED, mode-order and persistence regressions. Kept trusted FM1 OTA, flash IDs and bootheader.
- Software validation only. Physical FM1 performance and live-stage stability still require hardware tests.

## V1.95 (2026-11-15)

- Cache the exact WAH attack and resonance control conversions separately for every slot; refresh only when its own relevant knob changes. This adds a small fixed amount of scalar state, never another PCM delay line.
- Replace both WAH response `*15000/24000` calculations with the mathematically identical `*5>>3`; no rounding changes.
- Keep the old hidden-modulation coefficient cache warm even when independent FX are active: an attempted shortcut altered the first sample when hidden modulation was activated and was therefore rejected.
- Add a PCM oracle fixed to V1.94 for all ten effects, both 1x and 8x, each slot with separate parameter values and realtime bypass/reordering; all 20 cases must match bit-for-bit. Additional oracles cover legacy FX settings, hidden-modulation transitions and live WAH knob edits.
- All effects, individual FX control banks, GLIDE, MIDI, project saving, EQ, USB audio, first-run flow, OTA and hardware bring-up remain present.
- No extra delay, chorus or reverb history buffers; no new per-sample allocations or floating-point DSP.
- Physical FM1 CPU/DMA margin and live reliability are not yet independently verified. Host benchmarks are diagnostic, not a hardware certification.

---

# RUPTURE V1.93 — Repeatable FX Slots

## V1.93 (2026-11-15)

- GLIDE panel now shows simply **PORTAMENTO**; removed the **MONO** qualifier and **PITCH ONLY / NO EXTRA VOICE** line.
- Permit duplicate FX types in any of the eight slots. Insertion still enables the slot. Each slot independently stores its type, ON/OFF bit and short bypass fade; reordering changes the signal order. PJT35 archive size is unchanged and duplicates round-trip through save/load.
- For CHORUS, DELAY and REVERB, read their original fixed-size tank once per sample and reuse its return for later occurrences in the chain. In particular, do **not** advance the same ring twice in one PCM frame, which would halve its delay timing. No additional PCM buffers or voices were added.
- Likewise, repeated WAH, COMP, TILT, FUZZ and SAND do not advance their common envelope/filter state repeatedly within one PCM frame. DISTORTION repeats its clip stage, but updates the shared tone-smoothing memory at most once per frame. This protects envelope speeds and avoids gratuitous DSP work; shared per-type settings remain deliberate.
- All repeated types currently use shared per-type parameters. The three time-based effects also share a processor history and are therefore layered insert applications, **not independent effect copies** with distinct tanks. Fully independent duplicate reverb/delay histories would require additional SRAM and CPU and are outside this conservative hardware budget.
- Although the PJT35 record length is unchanged, older V1.92 builds reject repeated effect IDs. Save a backup and remove duplicates before downgrading.
- Previous unique-type chains reproduce exactly the same PCM hashes as V1.92, including Distortion/Wah and their real order. Verified repeated types, individual bypass and unchanged tank cursor advance with host tests. This does not establish actual FM1 CPU margin, reliability on stage or equal worst-case cost when new inserts are active.
- Preserved trusted OTA, two flash IDs, POPFIX and unchanged project codec format. Hardware validation is mandatory before live use.

---

# RUPTURE V1.92 — Distortion and Wah Wah

## V1.92 (2026-11-15)

- Expanded the FX library from eight to **ten algorithms**, with **eight ordered insert slots** unchanged. Added DISTORTION and WAH WAH; no change to supported simultaneous slot count.
- DISTORTION: K1 DRIVE, K2 BIAS (asymmetry), K3 TONE and K4 MIX. Hard-clipping character with a smoothed one-pole tone stage and bounded output; separate from MOSFET and Octave Fuzz.
- WAH WAH: K1 SENS, K2 SPEED, K3 RESONANCE and K4 MIX. Stable envelope-following auto-wah with one-pole bandpass sections. No additional delay buffer or filter state array.
- Both effects are processed only at their real slot positions; FX-list SEL still opens parameters, K4 toggles ON/OFF and ALGORITHM reorders the actual signal path. Inserting a new effect automatically enables it.
- Preserved the fast combined COMP/TILT/FUZZ/SAND DSP branch even when Wah or Distortion parameters are nonzero later in the chain. Avoids accidentally processing Wah twice and avoids a sizable unnecessary worst-case audio load. Both new MIX controls have smooth sample-rate transitions.
- Reused the existing four persisted Q15 parameter fields for each effect, so the PJT35 and GLIDE record sizes are unchanged. Removed-effect fades are cleared outside the audio interrupt, avoiding stale fade states and keeping the idle DSP shortcut effective.
- Updated HELP and the read-only 49-screen quick tour. Added full-range host arithmetic/routing tests and a real project save/load test for both effects.
- **Hardware qualification pending:** maximum eight-insert CPU use may exceed V1.89, especially with the new algorithms. FM1 DMA margin and stage reliability still require physical measurements. **Downgrade warning:** V1.91 will refuse projects with the new effect IDs 8 and 9, even though both versions use PJT35. Back up projects before changing firmware.

---

# RUPTURE V1.91 — FX Reliability and Real-Time Optimizations

## V1.91 (2026-11-15)

- FX slot **SEL now opens parameters** for a filled slot; **turn K4 clockwise for ON, counterclockwise for OFF**. Empty-slot SEL still opens the effect picker. K1 opens the effect library, SELECT scrolls slots and ALGORITHM changes actual signal order.
- Added the exact-PCM empty/bypassed-chain dry fastpath and bounded residual tail flushing. Processing cost drops in the common zero-FX case without changing the direct output.
- Replaced per-sample variable integer division in the MOSFET output stage with a fixed 257-point ROM transfer function; host stress tests verify its symmetry, monotonicity and bounded arithmetic.
- Reused the preexisting 4-processor combined DSP path where COMP/TILT/FUZZ/SAND appear in their historical order and audio state permits, while retaining arbitrary signal-chain ordering.
- FX reordering, insertion and bypass now publish the nine-byte chain together with tone-effect enable switches in one short interrupt-guarded transaction, avoiding a half-updated signal route in the audio callback.
- No added oscillators, sample voices, audio allocations, ISR waits, flash writes or UI calculations in the sample loop.
- **Safety qualification:** Although the full eight-effect DSP path is faster on the host than V1.90, it is still more expensive in the worst case than the former V1.89 four-effect engine. FM1 hardware cycle/DMA margin and live performance remain **unverified**. This version is intended for controlled tests before show use.
- Preserved PJT35 project and GLIDE compatibility, protected 16 KiB bootheader, trusted OTA, both supported JEDEC flash IDs and V1.89 speaker startup sequence.

---

# RUPTURE V1.90 — GLIDE and Eight-Slot FX Chain

## V1.90 (2026-11-15)

- Added a dedicated GLIDE synthesizer page after SUB OSC 2 with 0–2000 ms time, OFF/LEGATO/ALWAYS modes, LINEAR/EXP interpolation and envelope retrigger ON/OFF.
- Added SEQ-held K4 to mark individual stopped-sequencer steps as SLIDE, using the patch's glide duration.
- Reworked the FX menu into eight individually scrollable, initially empty insert slots. Add/replace/remove from the FX library; each new FX starts enabled. SEL toggles a filled slot, EDIT opens its existing parameter page, and ALGORITHM moves the selected effect and its ON/OFF status one slot at a time.
- Implemented a genuine ordered PCM/DSP graph. REVERB, DELAY, CHORUS, MOSFET, COMP, TILT, FUZZ and SAND consume the same fixed native processor instances, so duplicate effect types are disallowed. Effect order now changes the audio signal, not just the list display.
- Extended the project archive to **PJT35** with an additional 106 bytes for the FX chain, per-preset Glide settings and sequencer SLIDE flags. Legacy PJT34 projects retain their original audio processing order until the user edits the chain.
- Updated HELP, native quick tour and UI controls; retained V1.89 POPFIX, full ENV/BEND art, dual JEDEC flash-chip recognition, Web MIDI connection fix and the trusted OTA loader and protected bootheader byte-identically.
- Cross-compile and host codec/FX tests pass. Full hardware audio latency, highest FX CPU load and SAVE/reboot tests must be verified on a physical FM1 before regular use.

---

# RUPTURE V1.89 — Speaker Pop Fix

## V1.89 (2026-11-13)

- **Imported the speaker startup POPFIX** from the user-supplied V1.88.2 source into the complete V1.88.3 firmware. Only `bringup.c`, `hal/fm1_audio.h` and `native_output_startup.h` change startup behavior; a separate ABOUT version label identifies V1.89.
- Starts the silent audio DMA and I2S clock before asserting codec lines PC0/PC2/PC1. Preserves the original line order, with a 2 ms step and 5 ms settling interval. Warm restarts with already-enabled codec lines avoid an unnecessary second edge.
- Holds a watchdog-fed **300 ms quiet interval between codec start and LED-bank activation**, avoiding coincident codec-bias and LED-current transitions. The existing PC6 analog gate still requires 400 ms LED stability, DMA/USB settling, and subsequent muted-to-audible fade; recovery and keyboard-only modes retain their respective audio restrictions.
- **Preserved all V1.88.3 ENV frames**: the smoke settles into the exposed hole and returns in exact reverse on note release. BEND/LFO graphics and the synthesis/FX/MIDI/storage engines are unchanged.
- Retained the native trusted OTA loader, the protected bootheader, NUL-safe read-only Web MIDI connection verification and Puya/UC25HQ80 flash compatibility. The loader and bootheader are verified byte-identical to V1.88.3.
- Native and host-side regression checks verify the intended startup order and OTA packaging. Whether the speaker pop is audibly eliminated requires testing on real FM1 hardware; this remains a software release candidate.

---

# RUPTURE V1.88.3 — Complete ENV Animation

## V1.88.3 (2026-11-13)

- **Restored the full ENV eruption sequence:** all 20 original-resolution 224×30 monochrome frames now progress from the intact brick wall through an eruption into a fully exposed hole. The final eight frames animate the plumes settling and clearing instead of freezing in place.
- **Held note:** the finished hole remains visible. **Note release:** the exact animation rewinds toward the intact wall from the displayed frame, without a jump or a second plume. Short notes and retriggering during release are handled continuously.
- Regenerated the native ENV sprite and hold-frame ROM tables directly from the 1-bit PNGs; verified per-pixel parity. LFO, BEND, audio processing, effects, MIDI, presets and flash storage remain unchanged.
- Retained the **trusted OTA loader and protected 16 KiB bootheader byte-for-byte** from V1.88.2. The existing Web MIDI read-only identity probe and NUL-safe connection handling are unchanged; both supported Puya and UC25HQ80 JEDEC IDs are still accepted.
- Native compilation and host tests passed. Hardware installation, frame timing and actual OTA on a physical FM1 still require validation.

---

# RUPTURE V1.88.2 — BEND Crop Fix

## V1.88.2 (2026-11-11)

- **Fixed all 40 BEND frames** across LINEAR, SOFT, HARD, S-CURVE and EXP. The previous 156×90-ish poster cells were stretched independently along X/Y to fit a 224×30 panel, creating the flattened appearance.
- Each original poster cell now supplies a **true 1:1 pixel crop, 30 pixels high**, positioned by a per-frame camera Y coordinate. The crop (156 pixels wide) is centered in the existing 224×30 black panel with side margins, avoiding all image resampling or geometric distortion. Tall scenery outside the panel is deliberately cropped; it is not squashed.
- Frame sequence, 4-of-5 shake behavior, static close-up camera (no shake), curve presets, original source poster, control logic, voices, audio DSP, FX, flash storage and OTA/bootloader behavior retained.
- The native ROM sprite bank stays **exactly 33,600 bytes**, with **no additional per-frame computations** in the audio engine.
- Native FM1 software builds and tests are not hardware validation.

---

# RUPTURE V1.88.1 — Approved Poster Frame Correction

## V1.88.1 (2026-11-11)

- **Corrects V1.88's visual mismatch:** the earlier V1.88 firmware used newly drawn sprites, not the 40 illustrated frames from the approved reference poster. This release directly crops and converts **the actual 5 × 8 cells in the user-provided 1536×1024 JPEG**, retaining the source compositions.
- All five preset animations now use that poster: LINEAR (Everest), SOFT (inscribed monolith), HARD (pirate flagship emerging from a meadow), S-CURVE (ship close-up from bottom to mast), EXP (Kraken).
- The reference poster is a composite, not the original individual source PNGs: original cells are cropped and non-uniformly scaled to the native 224×30 1-bit RUPTURE viewport. This conversion necessarily loses resolution; both untouched poster and intermediate crops are provided for traceability.
- **No runtime animation math**: all five 8-frame sets are prepacked at build time to the exact existing 33,600-byte read-only ROM array. UI frame selection and the existing 4-of-5 shake behavior are unchanged; close-up remains stable.
- **Audio engine untouched**: no additional ISR, DSP, MIDI, DAC, FX or animation cycles inside audio processing. Existing native LCD deadline gating is retained.
- **OTA/Connection Fix preserved**: Puya `0x856014`, UC25HQ80 `0xB36014`, Web MIDI generic port probing and NUL-safe FM1 identity handling. Original protected 16 KiB bootheader, trusted OTA loader and flash/storage layout unchanged.
- **Version bumped to V1.88.1** so the corrected firmware and download artifacts cannot be mistaken for the original V1.88 release.

---

# RUPTURE V1.88 — Five Curve-Linked BEND Animations

## V1.88 (2026-11-11)

- **K3 selects one unique 8-frame 224×30 1-bit animation per curve**: LINEAR = unchanged V1.87 Mount Everest; SOFT = monolith smashing through stone; HARD = a three-masted pirate flagship bursting up from a meadow; S-CURVE = a steady close-up camera travelling up the flagship, from cannon-pierced hull to Jolly Roger; EXP = a colossal Kraken tearing open the ground.
- The 40 pre-rendered 1-bit frames are generated **offline** and packed into constant 33,600-byte flash ROM data. No additional audio ISR work, realtime procedural art, animation framebuffer, voices or DSP effects. The original 5 pre-FX K3 response curves remain byte-identical.
- **UI-only shake** on all sequences except S-CURVE. Up to 4 tiny ±1/2-pixel offsets cycle at 85ms while actively bending. The existing audio-first LCD refresh budget remains authoritative and can defer frames if audio is busy.
- V1.87 Everest sprites are kept bit-for-bit, including their original height and 1-bit monochrome appearance.
- Existing **OTA/Connection Fix** (NUL-safe identity parsing, generic Windows MIDI ports), both supported flashchip JEDEC IDs (Puya `0x856014`, UC25HQ80 `0xB36014`), trusted OTA loader, 16KiB protected boothead and flash layout remain untouched.
- Audio DSP, all 3 reverbs, presets, sequencer, MIDI queue, PC6 speaker gate and save format remain unchanged. Hardware verification required before unrestricted release.

---

# RUPTURE V1.87 — Everest BEND Mountain Sprites

## V1.87 (2026-11-11)

- Replaces the BEND page mountain animation with the new **Mount-Everest-style eruption sprite sheet** approved by the user. The mountain now bursts upward more realistically and aggressively as bend intensity rises.
- Integration is **offline only**: the eight 224×30 1-bit frames are preconverted from the supplied PNG sheet and compiled into `rupture_bend_sprites.h` ROM data. There is **no procedural drawing, no particle system, no extra LCD buffer and no work added to the audio ISR**.
- **K3 curve presets remain exactly as in V1.86.1**: LINEAR, SOFT, HARD, S-CURVE and EXP. K1 live bend behavior and K2 range selection remain unchanged.
- **OTA fix preserved:** the V1.86/V1.86.1 Web MIDI connection fix, dual flash-chip support and trusted OTA/storage code are unchanged.
- Changed assets only: `vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_sprites.h` and `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/*`.

---

# RUPTURE V1.86.1 — BEND K3 Curve Presets

## V1.86.1 (2026-11-11)

- **BEND K3** now selects five distinct pitch-response curves: LINEAR, SOFT (ease-out), HARD (ease-in), S-CURVE (smoothstep) and EXP (cubic).
- K1 still pushes the active string-style bend and quickly releases when turning stops. K2 still selects 1/4, 1/2, FULL, 1 1/2 or DOUBLE (2 semitones maximum). All curves share the same precise endpoints.
- Native BEND UI shows three physical knobs and their current values, a tiny actual K3 curve preview with K1 playhead, and the *curved* live pitch, meter and existing mountain sprite animation.
- Curve is computed in the control/UI path, **before** effects, not in the audio ISR. At K3 LINEAR the existing V1.86 pitch response remains unchanged. BEND performance settings are not saved to project flash.
- **OTA connection hotfix preserved:** NUL-terminated FM1 identity and extra JEDEC bytes are parsed correctly; unknown Windows MIDI port names still undergo read-only identification. Both existing flash chip IDs remain supported.
- Existing trusted OTA loader, 16 KiB boot/header, storage format, project/sequence handling, CXS/SAW and all effects unchanged. Device validation remains pending; keep a recovery path available.

---

# RUPTURE V1.86 — BEND Play Parameter

## V1.86 Web MIDI connection fix (installer/website only, 2026-11-11)

- Fixes a real FM1 detection regression introduced by V1.85.2 flash-chip telemetry. The 27-byte identity payload is NUL-terminated; JEDEC bytes 24..26 must **not** be concatenated to `FM-1_968`.
- Both Puya `0x856014` and UC25HQ80 `0xB36014` identity packets now parse correctly, while invalid device identities remain rejected.
- If Windows exposes generic or unrecognized MIDI port names, the installer safely probes at most 16 input/output combinations using the **read-only ID query**. OTA CHECK and WRITE rules are unchanged.
- Adds visible MIDI input/output diagnostics and a manual port pairing fallback when no FM1 identity is returned.
- **No native FM1 firmware, audio, storage, OTA loader or flash image bytes were changed.** Existing `RUPTURE-V1.86.fwsc` SHA-256 remains `239edee0056b32896f6f293cab22a05e366b94928ed76d2ebd4487047a45ccd8`.

- **FX short press** continues to open the existing FX screen. **Hold FX for 700 ms** to open the new BEND play-parameter screen.
- **K1 momentary bend:** turning K1 raises the playing pitch; releasing the encoder causes the bend to return rapidly to zero. K1 is transient performance control, not an FX wet level or a stored patch edit.
- **K2 range:** select `1/4`, `1/2`, `FULL`, `1 1/2` or `DOUBLE` (maximum 2 semitones). The active target, live semitone offset, 0–100% meter and `IDLE` / `BENDING` / `RETURN` status appear on the native 240×240 UI.
- **8 original 224×30, 1-bit mountain PNG frames**, compiled into packed ROM sprites: rise in response to K1, reverse automatically as the bend returns. No live mountain drawing or additional LCD buffers in the audio interrupt; existing CRT/VCR optics work on BEND.
- Pitch scaling is applied to the oscillator rates **before the existing FX processors**, preserving the sonic processing order, reverb algorithms and MIDI handling.
- BEND is session/performance state: no changes to stored projects, update-loader bytes, NOR layout, preset format, LEDs, PC6 startup logic or legacy page behavior.
- Native JieLi FM1 compilation and release packaging completed. Physical FM1 responsiveness, audible bend feel, DMA headroom, flash upgrade and post-USB power-on plop remain unverified without real-hardware checks.

# RUPTURE V1.85.2 — Dual FM1 Flash ID Compatibility

- Add `hal/fm1_flash_id.h` with exact allowlist `0x856014` (Puya P25Q80) and `0xB36014` (UCUN UC25HQ80). Both are 1 MiB SPI NOR; flash map, erase/program commands and project serialization stay unchanged.
- Read and cache JEDEC once before audio startup. Recognized IDs permit normal project and OTA staging writes; readable unknown IDs are refused. A failed/unavailable read (0) preserves V1.85.1's existing write behavior.
- Expose the cached ID in bytes 24–26 of the 27-byte MIDI update identity reply, retaining the exact `FM-1_968` text and the original SysEx protocol.
- Preserve the audited vendor/Felucca-derived app-only OTA loader and bootheader byte-identically: **there is no separate JEDEC allowlist in that loader**, so there is no verified loader-side patch to apply.
- No changes to synthesis, realtime DMA, sequencer, MIDI scheduling, UI, speaker-pop mitigation, flash partition boundaries, project data format, or first-run wizard. Existing V1.85.1 functionality is preserved.
- Host allowlist/unknown ID/identity tests, native FM1 build, MIDI regressions and sequencer audio regressions pass; project persistence and OTA on a physical UC25HQ80 FM1 remain unverified.

# RUPTURE V1.85.1 — Sequencer LENGTH=4 playback hotfix

- Corrected the audio-engine sequencer length validation: LENGTH=4 displayed a four-step loop on the UI but the DSP incorrectly defaulted to sixteen steps.
- The real audio playhead wraps inside the selected length when LENGTH is reduced during playback; only selected steps can trigger on subsequent note boundaries.
- Steps beyond LENGTH are **not deleted**; raising LENGTH to 16 restores all sixteen recorded notes.
- Only `audio_core.c` changed in FM1 firmware sources. Sound, reverbs, all other sequencer behavior, storage, Welcome, MIDI, LED/PC6 and OTA remain byte-for-byte unchanged at source level.
- Real DSP host tests cover live 16→4→16, fresh four-step loops and the unaffected 8–128 lengths. Unrelated 16-step/FX PCM is byte-identical to V1.85. Hardware verification pending.

# RUPTURE V1.85 — Realtime MIDI + DMA diagnostics

- Bound exceptional USB MIDI bursts to 32 queued packets per 128-frame audio half. Preserve FIFO and existing overflow panic; remaining events are serviced next DMA half. This is a deliberate latency/headroom tradeoff **only under extreme incoming burst traffic**, never a note-drop policy.
- Fast-path unchanged 27-key outbound panel MIDI; retry pending unsent Note OFF/ON after USB queue backpressure.
- Skip optional scope copying when USB MIDI backlog remains; actual PCM and effects never skipped.
- Add second read-only GLO → AUDIO DIAGNOSTICS page (SEL to switch) showing pre-render control peak, nested TIMER5 peak, latest DMA slack, maximum MIDI batch/queue, deferred MIDI batches, guard count and post-publish LATE count. The LATE, GUARD and POST LATE counters remain cumulative across audio restarts, allowing LATE=GUARD+POST LATE to remain meaningful. Original diagnostic display retained.
- Keep native CXS/SAW, both SUBs, all three reverbs, Welcome/format, native PC6 LED-pop mitigation, project codec, OTA source, trusted OTA loader and protected bootheader unchanged.
- Native JieLi build and host randomized 120,000-state panel-MIDI packet parity tests pass. Physical FM1 DMA LATE/GUARD and pop-free speaker operation remain to be verified.

# RUPTURE V1.84.2 — Speaker Pop: LED/PC6 Sequencing Hotfix

- Removed the simultaneous full-current twelve-button LED transition during AUDIO STOP/SAFE/UPDATE status. Protected states now keep the background function LEDs on their existing fast low-current glow; physical press feedback remains immediate. Normal page, SEQ, PLAY and REC LEDs remain unchanged.
- PC6's existing digital-silence/USB-stability startup now additionally requires **400 ms of actual stable function LED masks** before the single analogue power-enable transition. The LED soft-start is refreshed during the Welcome wizard, and the bank is initialized before the audio start.
- **Not changed:** synth PCM, SUBs/DRIFT, ROOM/HALL/SPRING and all FX, audio fault-stop guard, input/LED scan ISR, sequencer, storage, first-boot wizard logic, USB MIDI/OTA, protected 16 KiB header, trusted OTA loader. No PWM is applied to PC6.
- Cross-build and host startup arbitration tests passed. The source-based mitigation is **not proof that a hardware-only PC6 amplifier-bias pop has been eliminated**; a low-volume physical comparison, including GLO-held PC6-muted diagnostic, is necessary.

# RUPTURE V1.84.1 — Button LED Startup Pop Mitigation

- **Only the function-button LED policy changes.** In V1.84, the first-run Welcome state commanded all twelve function LEDs at full current in one update. V1.84.1 uses low-current glow for Welcome, introduces the dim states incrementally during the first ~260 ms, and then hands control to the original V1.84 LED rules.
- **Unchanged:** every non-startup button and note LED behavior, all synthesizer/DSP/reverbs, LCD, hardware matrix-scan ISR, USB/MIDI, boot/welcome logic, storage, protected flash header and OTA loader.
- **Physical limitation:** the sudden LED-current step was a plausible coupling source, but a startup pop can also result from the separate PC6 analog amplifier enable. The latter is deliberately untouched. The speaker-pop fix is not proven on FM1 hardware yet; do not infer physical silence from host tests.

# RUPTURE V1.84 — Hardware Integration

- **Contextual function LEDs:** active hardware page is bright; inactive function keys retain the existing fast dim glow. Physical presses always override the page indicator. Sequencer PLAY, armed/live REC and SEQ count-in each have unambiguous LED ownership. Normal octave, note, GUIDE and GRID feedback is preserved.
- **Encoder consistency:** a page change resets K1–K4 acceleration history, so a fast turn on the previous page cannot double the first movement on the new page. Selecting another FX panel also resets the history; discrete controls and existing accelerated continuous controls otherwise behave as before.
- **Realtime isolation:** changes only in UI/control-loop routing and button LED planes. No changes to the synthesizer DSP, three reverb engines, IRQ timing, HAL scan implementation, project schema, Welcome completion logic or native OTA writer.
- **Validation limits:** cross-compiled native firmware and host tests, including function LED priority and count-in logic. Physical LED perceived brightness, responsiveness and DMA/GUARD counts still need verification on actual FM1 hardware.

# RUPTURE V1.83 — Realtime Mono Headroom

- **Mono mixer:** replaced per-sample variable Q10 5–8-voice headroom stepping and variable division with the mathematically identical fixed `/4` operation for the always-monophonic RUPTURE build. Unmodified upstream poly behavior remains under the non-RUPTURE build path.
- **Master volume:** its Q16 target is now calculated once per audio render block, retaining the exact per-sample smoothing for changing gain, MIDI notes and preset ramps.
- **Sound preservation:** CXS, SAW, both SUBs with DRIFT, all three unique ROOM/HALL/SPRING reverbs, all other FX, envelope, sequence and MIDI paths are unchanged. Host stress renders match V1.82 sample-for-sample for rapid notes and multi-FX scenarios.
- **Safety:** project storage, the existing V1.82 Welcome completion marker, exact protected 16 KiB bootheader and trusted OTA loader are preserved; no new flash writes or partitions. The installed firmware's physical DMA deadline and true worst-case cycle headroom still need measured hardware validation; zero-underrun operation is **not yet certified**.

# RUPTURE V1.82 — First-Boot Welcome Restore

- Replaced the legacy-only `welcome_done` boot decision with a CRC-checked V1.82-specific completion marker in the existing device settings journal. Legacy V1.81 completion is now treated as requiring the V1.82 wizard once; normal subsequent boots skip it.
- Complete Welcome, backup confirmation, safe optional format choice, CRT preference and optional 43-screen Quick Tour are restored on first V1.82 startup. Valid project storage is not erased by the wizard, and format remains an explicit user action.
- New settings records preserve existing volume, autosave, screensaver, autoload-project and CRT controls; legacy records remain readable. No new flash partition or storage payload format.
- Limitation: an V1.82 → V15 → *same V1.82* round-trip cannot always be distinguished from a restart if V15 preserves the journal. Reinstalling exactly the same firmware cannot be guaranteed to force the wizard automatically.
- Host regression covers legacy marker migration, restart, two project records, torn writes and previous journal compaction. FM1 first-boot hardware behavior is not yet verified. Original OTA code, trusted loader, protected header, synth and reverb processing remain unchanged.

# RUPTURE V1.81 — Rapid-Note Headroom & LCD DMA Coalescing

- **Fast-note mono DSP:** compute pitch increments and organic/LIVING control state only for RUPTURE's single active mono voice, rather than eight unused ETCH polyphonic slots. MIDI note-on, note-off, legato, step gates and all sound processors remain unchanged.
- **Timed mono gates:** do not rewrite the held-note stack on each audio sample when no sequence note expired. On a genuine gate expiration compact the stack exactly as before, at the same sample.
- **LCD contention:** rapid key presses previously marked every frame as an urgent SPI LCD update, bypassing UI pressure throttling. Coalesce only visual updates during detected note bursts to at most ~13 fps (~10 fps under audio pressure). User interface actions retain priority; no change to audio/MIDI event handling, modulation, or sample rate. Animation frames are skipped only as redundant intermediate displays, not removed from assets.
- **Reverbs:** SPRING, ROOM and HALL DSP files and audio algorithms untouched. Project storage, bootheader and OTA loader untouched.
- **Host verification:** byte-identical PCM output in a 512,000-frame rapid MIDI/step-gate/sustain/FX scenario, host note-burst UI budget tests, native FM1 crossbuild. Physical DMA LATE/GUARD and DSP cycle budget still need confirmation; cannot guarantee zero hardware underruns from host tests.

# RUPTURE V1.80 — ROOM / SPRING Rebuild & Fixed-Point DSP

- **ROOM:** replaced the shallow single-ring reflections with four independently delayed, damped lines in an energy-distributing Householder FDN. SIZE alters line lengths; DECAY controls loop feedback. A fully wet ROOM return no longer relies on low-level taps from an unrelated reverb design.
- **SPRING:** replaced sign-alternating feedback with two differently sized cross-coupled delay loops and cascaded short allpass dispersers, including loop-dependent re-dispersion. Damping and restrained spectral shaping target a more usable spring-like tail rather than the previous brittle metallic sound.
- **HALL:** preserved the existing algorithm and its native PCM behavior. The three engines share the same established 4,096-sample tank and 64-sample diffuser; just one engine processes each frame. K1 TYPE, K2 DECAY, K3 SIZE and K4 MIX remain unchanged. Project format / default legacy-HALL migration unchanged.
- **CPU work:** replaced redundant 64-bit Q15 calculations in phaser, tremolo, auto-wah, tape and decimator with *exact* bounded 32-bit products. Removed per-sample SIZE/DECAY-to-gain conversions from ROOM/SPRING: coefficients are cached at the parameter-change boundary. No new oscillator, FX instance, reverb PCM buffer or UI ISR.
- **Validation:** native FM1 crossbuild, 2-million-frame topology switching and bounds test, SIZE/DECAY response tests, high-gain SUB+FX stress for all three reverb engines, Hall sample-for-sample golden comparison, and native package/hash checks. Original OTA source, protected bootheader and trusted loader preserved.
- **Caveat:** ROOM uses a more capable delay network and can be slightly more expensive. Hardware DSP MAX, live DMA LATE/GUARD behavior and subjective realism still require actual FM1 measurements and listening. Never claim guaranteed zero underruns without physical testing.
- See `docs/TEST-V1.80.md` for results and limitations. Includes original synthesized stereo WAV comparisons in `demos/`.

# RUPTURE V1.79 — CPU Headroom / SUB DRIFT / DMA Safety

- Optimized both suboscillator Q16 pitch increments using a **sample-exact** saturating 32-bit partial-product fast path. Continuous per-sample drift, phase, detune, octave, note timing and waveform remain identical to V1.78 (700,000+ tests and identical long-run PCM hash).
- Optimized eleven additional global FX dry/wet interpolations and both channels of the three independent reverbs using mathematically exact Q15 32-bit operations. An out-of-range guard retains original 64-bit precision when necessary. Full all-effects PCM output matches the V1.78 engine byte-for-byte in stress scenarios, including reverb switches.
- Coalesced **knob-only LCD redraws** under recent measured audio wall-time pressure (50/75/100ms limits, plus nonessential art capped at 20fps). The knob parameters continue publishing at normal input speed; urgent navigation, metronome, notes, screen state and OTA are not delayed. This targets SUB + DRIFT edits under heavy CPU/DMA pressure without reducing audio DSP quality.
- No DSP buffer/FX topology/project codec/sequence/OTA changes. Hardware DMA LATE and GUARD must be measured on the actual FM1; no zero-underrun guarantee before hardware soak testing.
- See `docs/TEST-V1.79.md` for exact PCM parity, native tests, integrity and remaining physical validation.

# RUPTURE V1.78 — Three Independent Reverb Engines

- Replaced the previous single shared reverb algorithm with three **different real DSP topologies**: SPRING (dispersive allpass + inharmonic sign-alternating feedback), HALL (Schroeder diffusion + filtered multi-tap late-feedback tank), ROOM (sparse specular early reflections + short damped feedback comb).
- K1 selects the algorithm without overwriting the user's SIZE or DECAY; K2 = DECAY, K3 = SIZE, K4 = true 0–100% dry/wet. At full wet there is no direct oscillator feed. Only one algorithm runs per sample.
- The three engines use the existing 4096-sample reverb ring, 64-sample diffuser and filter states; no new SRAM, effect instances, per-voice work or external dependencies. Actual FM1 DSP/DMA performance remains subject to physical measurement.
- The selected engine is now saved for project-global and per-preset FX by using two previously-unused bits of the existing 1-byte routing field, retaining the PJT34 record length and CRC. Legacy routing values 0/1 decode as HALL and remain loadable. Older RUPTURE releases may reject **newly saved** projects with SPRING or ROOM, so make backups before downgrading.
- The 16-KiB protected bootheader, trusted OTA loader, OTA native code, device identity and all other synthesizer and sequencer modules are preserved.
- See `docs/TEST-V1.78.md` for host DSP, storage, throughput and firmware verification. Not physically verified on FM1.

# RUPTURE V1.77 — Navigation, Warm SAW & Usable Reverb

- SELECT on a normal screen with no assigned SELECT behavior now opens the identical scrolling screen list used on PLAY. Dedicated SELECT functions for FX, SEQ, project/sequence slots, help, settings and safe dialogs remain unchanged; no interception during OTA/recovery/welcome.
- SAW oscillator timbre: the original one-pole low-state and bandlimited SAW are retained. Rebalanced BRIGHT response puts a warmer filtered bass core in the lower/middle travel, while EDGE and SOUND SHAPE are less prone to premature hard clipping. No extra oscillator, delay line, wavetable or filter state. The original CXS oscillator code is untouched.
- Reverb: SPRING, HALL and ROOM tonal presets use more distinct SIZE/DECAY values and expanded existing tap spacing. A much more audible five-tap wet return, with stable 0% dry / 100% wet endpoints, fixes the perceived volume-drop-only behavior. The existing single 4096-sample tank, five reads and stereo return path remain in place; no per-voice reverb or larger buffers. These are character presets within the same algorithm, not separate physical spring and hall simulations.
- Builds and host regression/measurement results are in `docs/TEST-RESULTS-V1.77.md`. No project storage-layout change, firmware identity change, bootheader change or trusted OTA loader change. Hardware listening/CPU/DMA verification is still required.

# RUPTURE V1.76 — Recording Quantization, First Downbeat & Steady LEDs

- **Recording quantization:** note-ons snap to the nearest step rather than the delayed main-loop cursor. RUPTURE uses the hardware note-contact timestamp and current audio sample-clock phase, including tempo/swing and the wrap between first/last steps. Sustain tracks quantized note-on origins and does not count a future step before it starts.
- **Count-in downbeat capture:** the ISR-to-UI transition now reads the live audio step even before the UI publishes transport ON; it no longer substitutes the unrelated edit cursor. A key pressed within the last 50 ms before step 1 can be captured on the first step once it remains held. The first-step guard prevents the timestamp from wrapping a valid downbeat hit back to the previous sequence's last step.
- **Count-in transport latch:** after the UI acknowledges the start, the ISR's temporary force-PLAY flag is released, so later PLAY/STOP controls remain effective.
- **Intermediate LED brightness:** the existing 11-column LED multiplex scan uses 1-in-2 cycles (~455 Hz), or 1-in-3 in DIM LO (~303 Hz), rather than 1-in-8 (~114 Hz) / 1-in-12 (~76 Hz). No extra timer interrupt, LCD draw, audio callback, PWM timer or new buffer is added. Felucca HAL and boot are unchanged; RUPTURE sets compile-time overrides.
- Preserves V1.75 two-bar count-in, REC/PLAY behavior, independent click, sustain, pattern shift, step clear, ENV graphics, storage codec, protected 16-KiB boot header and exact trusted OTA loader. Native/host verification only; physical LED perception, key feel, timing and device recovery still require tests.

# RUPTURE V1.75 — Armed REC & Sample-Accurate Two-Bar Count-In

- Pressing REC on a stopped sequencer now ARMS live recording without starting playback. Press PLAY after REC to hear two bars of quarter-note clicks. The grid and physical SEQ key flash with the sample-clocked ticks; playback and recording begin together on step 1 after exactly eight beats.
- When the sequencer is already playing, REC still toggles live recording immediately, with no count-in. REC or PLAY during an active count-in cancels it; the synth can be played during countdown without capturing notes.
- The original independent PCM metronome mixes outside the monophonic synth. PRESETS still controls BPM; ALGORITHM changes metronome volume while armed/recording. Transport changes do not allocate voices, write flash or change the project format.
- Updated 43-page Quick Tour sequencer lesson, 13 HELP cards, cheat sheet and README. Protected bootheader and trusted OTA loader from V1.71 are retained; hardware validation remains necessary.

# RUPTURE V1.74 — Live Sustain, SEQ Shift & Welcome Overlay

- **Live sustain recording:** detect key holds and releases against the sequencer audio playhead; record their length inside the existing note data without creating new triggers or discarding HOLD/lock lanes. Every take is session-local until the user saves.
- **SEQ long press:** removed the shortcut that opened the sequence library. **SAVE** on SEQUENCER still opens the eight sequence slots.
- **SEQ + PRESETS:** while physically holding SEQ, turning PRESETS shifts the entire current sequence left or right one step per detent, wrapping around. It preserves note durations, local HOLD events, explicit HOLD origin references, parameter locks, and other pattern settings. The dormant pattern buffer is filled outside the audio interrupt; only the final pattern pointer is swapped atomically. PRESETS still controls BPM when SEQ is not held.
- **Welcome-overlay:** found that previously completed onboarding is intentionally skipped using persisted `welcome_done`. Returning users now see a short, *read-only*, non-destructive WELCOME screen at normal boot (the complete setup wizard remains only for first-time users). No flash format or forced setup.
- Quick Tour SEQUENCER lesson, HELP cards, README, cheat sheet and GitHub Pages documentation updated. METRONOME control stays ALGORITHM during REC; OTA and bootheader retained.
- Host tests / native build results are documented separately. Hardware behaviour remains unverified.

# RUPTURE V1.73 — PRESETS BPM & ALGORITHM Live-REC Click Volume

- During active live sequencer recording, **PRESETS keeps its existing global BPM role**, with no special REC exception. Changes affect sequencer tempo and the beat-synchronised metronome together.
- **ALGORITHM controls metronome volume (0–100) only while live recording**. Encoder events are consumed before normal sequencer HOLD and other page controls; outside live recording the original ALGORITHM mappings are unchanged.
- Updated the native SEQUENCER footer, Quick Tour SEQUENCER lesson (including space for its instruction), onboard HELP with a dedicated LIVE REC CLICK card, README, cheat sheet and website embedded documentation.
- No changes to the separate metronome PCM mixer, monophonic synth DSP, existing ENV PNGs/animation, sequencer step-clear logic, project/sequence storage, OTA logic, protected bootheader or OTA loader. Physical FM1 testing remains required.

# RUPTURE V1.72 — ENV Settling, Live REC Metronome & Full Step Clear

- ENV: introduced eight reworked, pixel-exact 224×30 monochrome PNG smoke/debris settling frames derived from the original wall. The smoke/falling pieces disappear after the eighth frame. The wall maintains its subtle, UI-only 1-pixel tremble throughout Note HOLD, and the original Note OFF closing frames remain unchanged. PLAY/LFO/SOUND SHAPE sprites and all existing opening/closing frames are untouched.
- Live REC: a quarter-note metronome follows the actual native sequencer's BPM/step triggers. It is rendered as a separate short audio click **after** the mono synth's voice, effects and idle gate. It never allocates, steals, silences or retriggers synth voices. It runs only in live recording, including during sequencer rests. PRESETS controls its independent 0–100 session volume while recording. Ordinary PLAY has no click.
- Sequencer: pressing SEL while stopped clears the whole selected step, all triggers, local HOLDs and six parameter locks, and safely detaches the directed HOLD links from other steps that reference the cleared triggers. Unlike V1.71, it does not delete only the first note. Step clear is blocked during transport/REC. No project format migration or storage-region changes.
- Added bounded host tests for metronome independence/tempo subdivision, full-step clearing, original asset parity, settling, sustained vibration and release behavior. Native FM1 builds and exact firmware OTA-container checks are required before any hardware claim; physical FM1 installation remains unverified.
- OTA: no modifications to native OTA handler, bootloader/header, update protocol, firmware identity (`FM-1_968`), flash layout, recovery or persistent data codec.

# RUPTURE V1.71 — Optional On-Device Quick Tutorial

- After the original first-run CRT/storage setup, added an opt-in **VIEW QUICK TUTORIAL?** YES/NO question. NO immediately completes setup; YES enters 43 read-only real-screen previews. No setup data is written until the existing first-run finish path, and no flash format can be triggered in the tutorial.
- Each lesson shows the existing native 240×240 1-bit screen, the actual route into that page, and descriptions of functional K1–K4 encoders. All eight FX receive separate chapters. Non-editable screens explain their purpose without inventing knob functions.
- Compact bottom overlay: **SEL** next, **SELECT** previous/next, **ALGORITHM** full-screen text toggle, **HOME** skip/exit. No parameter, patch, sequencer, storage, MIDI or OTA actions are executed by tutorial navigation.
- Added **GLO → QUICK TOUR** so existing installations can replay the walkthrough; replay explicitly avoids altering the welcome setting or writing the global settings journal.
- The four animated scene renderers, knob focus/easing and DSP remain unchanged. The protected bootheader, trusted OTA loader, V15 return path and OTA behavior remain subject to native regression testing. Physical FM1 and V15 tests are still required.

# RUPTURE V1.70 — ENV Smoke Settling & LFO Falling Dirt

- **ENV:** after the original wall-break opening animation, eight new original 224×30 monochrome hold frames make the airborne dust drift down and disperse. The last frame is pinned, motionless, for as long as a note remains held. Note OFF still uses the original ENV closing frames 13–19; short taps also close safely.
- **LFO:** after the original sinkhole opens, eight new original 224×30 monochrome frames show small rocks continually falling into the dark hole. They loop throughout Note HOLD. Note OFF retains the original reverse sequence down to the intact road.
- Added 16 pure 1-bit PNG source sprites, loop previews and a reproducible generator. The source graphics are packed into read-only native firmware ROM; no particle calculation, extra framebuffer, work in the audio ISR or new DSP overhead.
- The 20 original sprites of each existing panel animation (PLAY, ENV, LFO, SOUND SHAPE) remain byte-identical. The original HOLD animation of PLAY and SOUND SHAPE is also unchanged. The LFO shake continues during falling dirt; ENV shake ceases once dust has fully settled.
- Added actual native renderer and scheduler regression tests for settling, endless loops, immediate release, short taps and the on-screen sprite bytes. OTA/recovery identity and trusted loader are unchanged; a physical FM1 and V15 recovery test remain required.

# RUPTURE V1.69 — Exact PCM DSP & Smooth Knob Animations

- Precomputed compressor attack, release, threshold and ratio coefficients are refreshed on parameter changes rather than calculated for every PCM sample. The compressor transfer function, envelope rate, feedback and gain remain identical to V1.68. Existing idle bytes in the native FX state are reused; FX state footprint is unchanged.
- Replaced the four-knob 45×45 per-pixel geometry scan with exactly the same 332 foreground pixels per rotary outline. The entire 240×240 native framebuffer is pixel-identical to V1.68 across 42 non-versioned screen fixtures, including FX, EQ and knob focus.
- Knob animation targets **40 FPS** (25 ms main-loop service) when DMA deadlines are healthy, independently of the historical DSP MAX. Time-based needle easing stays responsive when some LCD frames are late. Presses and parameter text remain immediate.
- PLAY, ENV, LFO, SOUND SHAPE loops and shake retain their original progression. Only actual repeated DMA deadline misses trigger the existing audio-first backoff (50/100 ms), never just a high historical DSP MAX.
- Tests: 800,000 stereo PCM frames with live compressor automation match V1.68 sample hashes, and 12 additional FX scenarios and 42 native framebuffers match exactly. Physical FM1 CPU load, sustained LCD FPS, audio deadlines and cross-firmware/V15 recovery still require hands-on testing.
- The trusted OTA loader, protected 16 KB bootheader and firmware identity `FM-1_968` are preserved.

# RUPTURE V1.68 — Sequencer Step 1 & REC Playhead Fix

- **Step 1 note removal:** the RUPTURE sequencer key-toggle can now delete a trigger at zero-based step 0 even when older directed HOLD links reference it. Exactly those dependent links are detached, while unrelated triggers, independent legacy HOLD lanes and parameter locks remain unchanged. The same behavior applies to all 128 visible steps.
- **Explicit stopped-step delete:** press **SEL** in the stopped SEQUENCER to remove the displayed note on the selected step. This is also available on STEP 1; no delete is performed by SEL during live recording. Tapping the same note key retains its normal toggle behavior when the transport is stopped.
- **REC playhead animation:** the visible sequencer cursor now requests an LCD repaint when the audio engine advances a step; a deferred frame remains pending until painted. The high-load LCD safety cap is retained. Sequencer LEDs, audio clocks, MIDI, DSP and flash persistence code are unchanged.
- **Clear recording state:** the SEQUENCER header shows REC instead of PLAY while recording. The native SYSTEM INFO version now reads V1.68.
- OTA and recovery packaging retain the byte-identical 16 KiB protected bootheader and trusted OTA loader. Physical FM1 verification is still required.

# RUPTURE V1.67 — Consistent Knob Focus

- Fixed the actual issue: K1–K4 focus is now recorded for **every editable rotary page**, including all eight FX choices. The internal FX-page forwarding can no longer hide the focus indication.
- Added the same side-of-knob focus bars to the SEQUENCER, which uses a separate renderer, and to the fifth ALGORITHM/BLEND knob on SUB 1 and SUB 2. Channel EQ shows a corresponding row indicator because its parameters are displayed as values rather than round knobs.
- The active indicator lasts approximately 950 ms after the last turn and clears on page changes, FX selection changes, and EQ tab changes. It never draws through labels or values.
- No changes to synthesis, DSP, sequencer timing, project format, OTA code, trusted OTA loader, or protected bootheader. Physical FM1 validation remains required.

---

# RUPTURE V1.66 — SEL, Animation Timing & Sound Shape Fixes

- Replaced the old **SCL** label with **SEL** on native RUPTURE screens and help pages, and in the current user documentation. Hardware button mapping is unchanged.
- Fixed the V1.64 animation slowdown: changed sprite/shake frames remain pending until an LCD paint actually happens. Scheduled frame updates also catch up to wall-clock time after a slow main-loop iteration. Real repeated DMA misses still impose a protective render budget.
- PLAY has an actual subtle 1-pixel water-loop shake. ENV, LFO and SOUND SHAPE preserve their held-note animations and closing sequences. Original 1-bit PNG bytes, frame order and octave-based PLAY speed are preserved.
- **CXS WEIGHT** now increases the low/high tonal contrast instead of mostly adding gain. **CXS PUNCH** emphasizes upper-band/transient content for the full duration of a held note instead of ending after eight oscillator cycles. The original first-eight-cycle attack accent remains at reduced intensity.
- With WEIGHT/PUNCH at zero, CXS output is sample-identical to V1.65; existing saved presets with **nonzero** WEIGHT or PUNCH will intentionally sound different. SAW shaping stays unchanged. No project/storage format change or new audio buffer.
- Native build, targeted real CXS/SAW audio regressions, sprite renderer checks, OTA staging/regression, package CRC, pinned bootheader/loader and WebInstaller parity were retested. Physical FM1 audio and V15 restore tests remain outstanding.

# RUPTURE V1.65 — Software Release Candidate / Validation Gate

- Rebuilt and audited the V1.64 feature set without changing audio/DSP, storage schema, USB/OTA or rescue logic.
- Corrected the SYSTEM INFO version and fixed the outdated firmware filename in README.
- Added a structured, reproducible release-gate report with a physical FM1 acceptance matrix.
- Verified application package boundaries, pinned bootheader/loader, installer parity and regression results.
- **Release status: software RC only.** Real FM1 stress and actual V15/cross-firmware recovery remain mandatory before a production release.

# RUPTURE V1.64 — Hardware Status & Audio-First LCD Scheduler

- Consolidated two competing screen-rendering paths into one and replaced redundant 50 ms idle full-screen redraws with a calibrated main-loop display budget: 350 ms idle refresh, 50 ms animation refresh under normal conditions, 100 ms with pressure/late DMA indicators, and 180 ms under severe stress. No new DSP/ISR work, buffers, synthesis routines or notes were changed.
- Physical inputs, notes, page transitions and explicit warnings remain immediately paintable. Only optional intermediate cosmetic frames are dropped when audio pressure increases.
- SYSTEM INFO now identifies the battery result as a **coarse voltage band** (LOW, MID, HIGH, FULL or UNKNOWN), not as a precise fuel-gauge percentage. USB link status and existing DSP/DMA diagnostics remain read-only.
- Existing FM1 key LEDs, active-note and sequencer LED mappings, 1-bit RUPTURE animations, PROJECT & PRESET safety, screen filter default OFF, and first-run wizard are retained.
- Protected 16 KB bootheader, trusted OTA loader, firmware identity and native cross-firmware / V15 recovery implementation preserved. OTA simulations do not replace a physical cross-firmware and V15 restoration test.

# RUPTURE V1.63 — Project & Preset Safety Polish

- Added an explicit unsaved-work indicator in the UI header and next to the active project's name in the project library. This is separate from the autosave retry flag, so a failed autosave never falsely clears the warning.
- The existing two-press project OPEN confirmation now warns about **unsaved edits**. STORAGE > PROJECTS also warns before switching away or reloading the current project. HOME or moving the cursor cancels confirmation.
- Uniform flash-operation feedback: SAVING/LOADING, SAVED SUCCESSFULLY, LOADED SUCCESSFULLY, SAVE FAILED, LOAD FAILED. Success is only announced when the existing storage operation reports OK.
- Added a RAM-only, single-gesture knob UNDO using HOME+SCL. Safe single-byte controls on PLAY, SHAPE, SUB 1/2 and TILT/FUZZ/SANDPAPER only. Complex FX, sequencer changes and project loads intentionally excluded.
- Updated SYSTEM INFO to report the actual V1.63 firmware version. Existing animations, CRT default OFF, boot identity, native audio DSP, storage journal format, protected OTA bootheader, trusted loader and cross-firmware/V15 restore source left unchanged.
- OTA/V15 hardware cross-install and audio stress tests remain required on physical FM1 units.

# RUPTURE V1.62 — Native Control & System Info

- Added a dedicated read-only SYSTEM INFO screen under GLO: firmware and identity, USB link, battery, storage, project-slot count and measured DSP/DMA peaks.
- Polished continuous knob behavior: one step per slow detent; controlled two-step acceleration only on sustained rapid turns. Discrete choices never accelerate.
- Added a subtle, temporary focus marker to the last changed control knob.
- Kept CXS/SAW audio DSP, all existing waveform animations, storage journal schema and OTA/recovery routines unchanged.
- Physical FM1 verification remains required, especially audio DMA margin and cross-firmware/V15 transition.

# RUPTURE V1.60 — First-run format confirmation

- Following a deliberate three-second SAVE hold, the wizard now displays a dedicated **FORMATTING... / PLEASE DO NOT POWER OFF** screen while the existing storage operation runs.
- The result remains on screen until SEL is pressed (with a short anti-skip interval): **FORMAT SUCCESSFUL / STORAGE READY** and a visible check mark only after successful initialization; **STORAGE ALREADY READY** if valid RUPTURE storage is present and no erase occurs; **FORMAT FAILED** on an unsuccessful operation.
- Existing NO paths, CRT choice, project storage, animation sprites, synthesis, and OTA-to-other-firmware/V15 recovery code are preserved.
- This is a UI/control-flow correction. No additional work or allocations in the audio callback. Hardware validation remains necessary.

# RUPTURE V1.59 — First-Run Welcome and Clean Release

- First launch shows a 240×240 monochrome welcome overlay with backup confirmation, a continue-without-backup branch (including `:(`), optional storage formatting, CRT ON/OFF selection and a visible CONTINUE TO RUPTURE action.
- Flash format is never automatic. The wizard requires an explicit YES followed by a three-second physical SAVE hold; HOME/SEL cancels. Already-valid RUPTURE project storage is preserved rather than reformatted.
- Setup-completed status is stored in the existing CRC-protected global settings journal, without a new flash partition or per-project format. The marker is copied through A/B project saves and journal compaction. If the user continues with no compatible, initialized storage, no journal write is possible and the wizard may reappear on reboot.
- CRT preference stays stored globally; previously chosen CRT preferences are respected until changed. The audio ISR, synthesis/DSP code, protected bootheader, trusted OTA loader and cross-firmware/V15 restore logic are unchanged.
- README is now a permanent user guide with no release-history text. CHANGELOG.md contains release history. The distribution is cleaned of stale firmware binaries, old build outputs and redundant logs.
- Native build, welcome-flow and storage-journal tests passed. Physical FM1 formatting, startup and V15 restoration remain to be tested on hardware.

# RUPTURE V1.58 — SHAKE ANIMATION (LFO, ENV, SOUND SHAPE)

- Adds a subtle, 1-2 pixel shake to existing LFO sinkhole, ENV wall rupture and SOUND SHAPE volcano 1-bit sprites.
- ENV and LFO now continue shaking while their note gate is held, and reverse/close normally at Note OFF. Volcano's 3-frame lava loop shakes with its sprite changes.
- A byte-packed UI-only shifter keeps the blit inexpensive, with no extra screen framebuffer, no sprite rerendering and no change to audio processing.
- The original sprites, CXS PRESSURE fix, persistent settings, identity `FM-1_968`, cross-firmware OTA logic and stock V15 restore logic remain intact.
- Physical FM1 audio/display and cross-firmware recovery still need hardware verification.

# V1.58 (source update)

- Added subtle PLAY-style shake/jitter to the **SOUND SHAPE** volcano animation.
- Added matching shake/jitter to the **ENV** wall-breach animation.
- Added matching shake/jitter to the **LFO** sinkhole animation.
- Uses the existing pixel-for-pixel PNG sprite art with a tiny UI-only whole-panel offset; **no audio-engine, DSP, interrupt or OTA logic changes**.
- OTA / V15 recovery path intentionally untouched.

# V1.57 — SOUND SHAPE Volcano Animation

- Added 20 offline-converted native 224×30 1-bit PNG volcano sprites to SOUND SHAPE. The static label is replaced; oscillator controls and labels are retained.
- Note ON progressively fractures the volcano; holding keys loops 3 lava eruption frames. On the last Note OFF the lava recedes and the crater closes.
- Animation data lives in firmware ROM (16,800 bytes), sharing the existing UI canvas and animation state. No audio ISR, CXS, SAW, FX, oscillator or envelope DSP changes.
- Firmware identity FM-1_968 and previously implemented OTA/stock V15 compatibility protections are preserved. Physical hardware verification is still required.

# V1.56 — Dedicated ENV & LFO Rupture Animations

- LFO uses the approved top-down street/sinkhole artwork; holding keys opens the hole and releasing the last key **reverses the exact opening-frame sequence** back to the intact road. ENV uses the approved independent wall-rupture sequence with its own opening and closing frames.
- Added **40 offline-prepared 224×30 1-bit PNGs** to the source project and their 33,600 bytes of packed, read-only bitmaps to native firmware. The same byte-aligned 840-byte sprite blitter and existing 240×240 1-bit framebuffer are reused. Animation state/timers are managed exclusively by the non-realtime UI loop, never the audio interrupt. No added voices, DSP effects, audio buffers or DSP calculations.
- Preserved V1.55 PRESSURE fix, V1.54 PLAY DAM V3 animation and shaking, Utrecht screensaver, CRT default OFF, existing project format and settings. OTA C source, trusted loader, protected 16 KiB boothead, firmware identity `FM-1_968` and cross-firmware safety behavior must remain unchanged.
- The WebInstaller retains the independent JavaScript selection/right-click guard, repaired firmware validation, 100% transfer/reboot statuses, post-boot identity retry, and FM1 Recovery Windows link. Native pixel parity, animation timing, OTA package tests and physical FM1 status are documented in `docs/TEST-V1.56.md`.

# V1.55 — CXS PRESSURE Range Fix + OTA Preservation

- Fixed CXS PRESSURE appearing to change the sound only between 0 and 1. The existing cycle-exchange energy model changes segment durations only slightly after fixed-point rounding. PRESSURE now also progressively shapes the amplitude profile of the existing eight CXS waveform segments, providing real tonal variation throughout the 0–100 range.
- All new amplitude work happens only at oscillator-cycle boundaries, never for every audio sample. No new voice, synth block, extra RAM buffer, audio interrupt, FX stage or parameter-storage field. The change intentionally affects CXS timbre, including any enabled CXS suboscillator using the shared core; SAW core is untouched.
- Preserved DAM V3, 20 supplied sprites, shaking visual, Utrecht screensaver, CRT default OFF, settings and project formats.
- Safety-critical: `ota.c`, `usb.c`, protected 16 KiB boot area and the trusted staged OTA-loader must remain unchanged. Native OTA regression and package comparison are required; the actual physical FM1 update and recoverability are not proven by software tests.
- Retains the corrected installer status, manual reconnect/verification, text selection and rightclick blocking, and README/CHANGELOG + FM1 RECOVERY WINDOWS links. These fixes are now also carried forward in the site generation template.

# V1.54 — DAM V3: Water Across the Road

- Replaced the original 12 PLAY waveform sprites with **20 exact 1-bit 224×30 PNGs** from the user-approved DAM V3 set, including water and foam flowing visibly over the dashed road. The PNGs are copied as immutable ROM pixels, not procedurally redrawn.
- First note triggers frames 1–10, sustained notes cycle only frames 11–13; last note-off begins the appropriate closing frame from 14–20. Octave -2..+5 sets loop rate from 250 down to 40 ms. Very short notes never trigger a full flood.
- The existing UI canvas copies 840 bytes per frame, exactly as V1.53. Static ROM asset size grows from 10080 to 16800 bytes. No new RAM framebuffer, no audio callback or DSP changes, and no extra audio samples are calculated. Longer animation sequences may result in more UI redraw events per note.
- Retains V1.53 CRT default OFF, screensaver, storage format, FM-1_968 identity, trusted OTA loader, and V1.53 WebInstaller success/reconnect improvements and FM1 Recovery Windows nav link.
- Native JieLi crossbuild and 20/20 pixel-parity host test pass; actual FM1 CPU/DMA timing and LCD impact remain unverified.

# RUPTURE V1.53 — CRT/VCR OFF by Default

- Changed the CRT / VCR LOOK factory/default setting to **OFF** on first boot and in the legacy settings fallback. The SETTINGS switch still enables the exact same optional V1.52 two-axis CRT effect and persists its value in the existing CRC-checked 8-byte global settings journal.
- Existing explicitly stored CRT ON/OFF choices are respected; they are not silently overwritten by a firmware upgrade. For any unit with CRT previously saved ON, change it once under SETTINGS to select OFF.
- With CRT disabled, the original binary 1-bit RGB565 LCD output is used; no per-pixel CRT transformation is run. No audio DSP, waveforms, stored projects, OTA loader, protected boot header or firmware identity changed.
- Kept the improved V1.52 web installer success, reconnect and manual verification flow; updated its embedded verified .fwsc to V1.53.
- Native, storage and OTA tests are host-only; physical FM1 UI and OTA verification remain outstanding.

# RUPTURE V1.52 — CRT Optics / Scanline Fix

- Fixed the V1.51 CRT effect: white 1-bit text is no longer damaged by a checkerboard row-cutting filter.
- Visible 2D barrel/fisheye distortion with gentle bilinear phosphor sampling, rounded black corners and subtle monochrome dimmed scanlines in RGB565 output.
- The original 1-bit UI buffer remains pristine; the effect is applied only when writing existing LCD DMA stripes. No new full framebuffers, audio DSP or storage-format changes. QR report stays unfiltered.
- SETTINGS → CRT / VCR LOOK remains saved and default ON. OFF restores the original unmodified 1-bit UI.
- Same OTA loader, protected bootheader and identity FM-1_968. Native build and targeted host validation pass; actual FM1 performance and appearance need physical testing.

# RUPTURE V1.51 — Monochrome CRT / VCR Display Look

- Subtle horizontal barrel warp and alternating monochrome scanline pixels added after the existing native 1-bit renderer, covering PLAY, SETTINGS, editors, screensaver, boot logo, and storage progress overlays. White remains white, black remains black; there are no colour or grayscale pixels. The scan-safe REPORT A BUG QR screen is intentionally unaffected.
- SETTINGS → CRT / VCR LOOK is a direct SEL ON/OFF switch, enabled by default on existing devices. OFF bypasses the filter entirely and is pixel-identical with V1.50 rendering.
- Reuses the already-allocated 240×240 1-bit display canvas; only 30 bytes of temporary row space and a small read-only 1200-byte barrel table. No new full-screen framebuffer or audio-DSP code. Filtering runs exclusively in the UI task, not the audio interrupt. CPU/display overhead requires measurement on physical FM1 hardware.
- The CRT flag is saved in the existing eight-byte CRC-protected device-global NOR journal using a new non-conflicting magic range for OFF. Boot volume, autosave, screensaver timeout and startup project remain backward-compatible. Compaction and project bank moves retain CRT state.
- Keeps the V1.50 Domtoren screensaver and V1.49 DAM sprites, all audio effects, MIDI, project storage, OTA recovery path and existing FM-1_968 identity.

# V1.50 — Extended Utrecht Screensaver Rupture

- Extends the procedural Utrecht screensaver so the crack no longer stops at the foreground ground split. Once the ground has ruptured, the fracture now climbs upward through the streets, disappears briefly behind the front houses, and continues visibly through the Dom Tower.
- Adds a late-stage structural shift in the Dom Tower: one cracked half sinks a few pixels downward while the saver is near its peak frame, then returns automatically during the existing reverse loop.
- Keeps the established saver timing model intact: the 64-frame saver still plays forward, holds, reverses back to frame zero, and loops using the existing state machine. No extra full-screen animation frame table, heap allocation, DSP work, or audio-thread rendering was introduced.
- Implementation remains purely 1-bit native UI drawing inside `ru_crack_saver()`, using the existing Utrecht city art plus procedural crack carving and a small moving tower-half mask. Physical FM1 validation is still required.

# V1.49 — DAM Sprite Waveform Bar

- Replaces PLAY waveform bars (both CXS and SAW) with **the actual 12 user-supplied 224×30 monochrome PNG sprites**. Native ROM stores their exact 1-bit pixels; no redraw, scaling, or procedural fluid simulation.
- The crack opens on the first gated note; leak animation cycles only source frames 06–08; stops on the last gate release and closes via source frames 09–12. A very short press skips safely to the matching closing frame.
- Leak frame interval reacts to displayed octave -2..+5: 270/220/165/115/75/55/45/35ms. Existing audio ISR, presets, sequencer, effects, MIDI/USB, storage format, recovery boothead, and OTA loader preserved.
- +10,080 static bytes (flash) for 12 original bitmaps, no new framebuffers or audio allocations. The renderer copies 840 bytes into the already-existing 1-bit UI canvas; unchanged LCD bands remain cached.
- Added exact pixel comparison and timing-state tests. Native crossbuild passes; physical FM1 display and audio under load require validation.

# RUPTURE V1.48 — Global SAVE Routing & SAW Harmonic Guard

- **SAVE anywhere in normal editing:** FILTER, FX, EQ2, ENV, LFO, ARP, GLO, SETTINGS, STORAGE, HELP, SOUND SHAPE, SUB and all other ordinary UI pages now open the same 12-slot PROJECTS browser. There is no implicit flash write when entering the browser. A project slot still needs the existing name/confirmation flow. SEQ and SEQ SLOTS retain independent sequence-saving; in the NAME editor SAVE still commits the typed name. OTA/SAFE/LOADING and disabled legacy screens remain protected from UI redirection.
- **SAW high-pitched alias/whistle reduction:** The SAW wavetable itself is bandlimited, but the nonlinear SAW EDGE/BITE/PUNCH/BRIGHT shapers can create new harmonics above the Nyquist limit, heard as an independent high whistle at extreme settings. A smooth, parameter-dependent harmonic guard now reads the **existing** 64/32/16-harmonic bandlimited SAW tables before the nonlinear stage. Neutral SAW is PCM-identical with V1.47. At maximal shaping, the guard uses the 16-harmonic bank without any new oscillator, buffer, table, per-sample divide or floating-point DSP. The SUB SQUARE and SUB SAW, CXS and all effects retain their prior signal routes. The only intentional tonal trade-off occurs at strong drive, where very high harmonics are reduced to suppress aliasing.
- Spectral host test at 151.27 Hz, fully cranked controls: aliased/non-harmonic power in 7–12 kHz falls from about -43.49 dB to -64.90 dB relative to total (improvement ~21.4 dB). Normal SAW path is sample-exact and RMS at maximum parameters remains approximately unchanged (29,720 vs 29,708 Q15 units). This demonstrates software alias reduction, not physical FM1 noise or guaranteed elimination of every possible high-frequency tone.
- Native FM1 crossbuild, all-page routing tests, 700k SAW test samples (UBSan), existing oscillator/sound-shaper tests, site and firmware integrity checks. The protected bootheader and OTA loader remain unchanged; project format unaffected. Physical listening and worst-case DMA headroom still require validation.

# RUPTURE V1.47 — SAW SOUND SHAPING Controls Fix

- Corrected the V1.46 partial SAW mapping: SOUND SHAPE PUNCH used the legacy CXS eight-cycle transient and stopped affecting sustained SAW notes. In V1.47 all four SOUND SHAPING controls meaningfully change SAW audio **throughout a sustained note**, not just at initial note-on.
- SAW now applies WEIGHT (low-end reinforcement), EDGE (existing bounded colour), BITE (odd-order harmonic saturation) and PUNCH (high-frequency saw-edge accent) with the **existing** SAW lowpass state, 32-bit integer arithmetic and no new oscillator voices, tables or buffers. With WEIGHT/BITE/PUNCH at zero and EDGE at 50, the neutral SAW PCM remains bit-identical to V1.46. In steady SAW the expensive, 64-bit CXS post-shaper is no longer run.
- CXS shaper and CXS PCM path are unchanged; ALGORITHM selects the main oscillator on PLAY/SOUND SHAPE, and still controls BLEND only on SUB 1/2. Existing SOUND SHAPE parameter persistence, 240x240 UI and stock V15 OTA path remain intact.
- Direct JieLi crossbuild succeeds (241648 bytes, +68 bytes versus V1.46). The new sustained-tone test confirms audible PCM changes from **all four** SOUND SHAPE knobs after 60,000 samples, PCM equality when neutral, bounded extreme settings and undefined-behavior sanitizer safety. Physical FM1 hardware output and DMA slack have not yet been measured.

# RUPTURE V1.46 — Native Oscillator Header & Active SAW Knobs

- Moved the CXS / SAWTOOTH indicator from above the PLAY waveform into the existing header between `PLAY` or `SOUND SHAPE` and `OCT X`, respecting 240×240 1-bit text widths. The waveform panel now occupies y=50..79, leaving empty pixels y=80..82 before the upper knobs begin at y=83. No extra overlay or new UI components.
- All four PLAY knobs have a real SAW signal path: **BRIGHT** (mix raw bandlimited saw with a lightweight one-pole warmth tap), **EDGE** (bounded soft saturation), **BODY** (controlled low-frequency reinforcement) and **LEVEL** (bounded output gain). When CXS is selected the previous PRESSURE / BALANCE / MEMORY / DENSITY labels and DSP are unchanged. The existing four macro values are reused and remain saved in their prior project records; there is **no extra preset storage**, though these values are shared across both oscillator modes.
- SOUND SHAPE's **EDGE** knob now also affects SAW; WEIGHT, BITE and PUNCH already use the shared post-carrier shaper. The extra fixed-point saw DSP runs only in SAW or during the short mode crossfade, without new voice slots, wavetables, heap, or FX changes. One small 32-bit per-voice filter state is added; it is reset on new note.
- Native renderer screenshots and pixel-spacing regression check verify CXS/SAW PLAY and SOUND SHAPE layouts. Host DSP test verifies all SAW knobs produce different PCM samples and handles extreme settings without overflowing. Project/sequence storage and verified stock V15 OTA route retained. Factual FM1 DMA and listening performance still requires physical verification.

# RUPTURE V1.45 — CXS / SAW Main Oscillator, SQUARE / SAW Subs & Reversing Screensaver

- ALGORITHM chooses the active main oscillator (CXS or SAWTOOTH) on both PLAY and SOUND SHAPE. On the two SUB screens it **still** controls only the original SUB BLEND, independently for SUB 1 and SUB 2. Other pages keep their existing assignments.
- New bandlimited SQUARE and SAW choices for either independent SUB oscillator, preserving OFF/CXS/CLASSIC and the existing anti-click type/blend fades. No new synth voices, extra output buffers or realtime floating-point computations; 7 mip levels per waveform are generated offline and stored in program flash. Frequency-band transitions smoothly crossfade below harmonic Nyquist thresholds to avoid new pitch-glide clicks.
- Main SAW replaces the CXS carrier when selected; crossfades during live ALGORITHM changes, uses the same SOUND SHAPE, filter, ADSR and FX as before. The original CXS output path is unchanged when selected. The steady SAW path skips CXS event processing.
- Save/load main selection using the previously unused `spectral_wave` bit 4, alongside both SUB TYPEs in their original 33-byte PJT34 DSP records. Prior project formats and legacy defaults remain intact; bit 5 stays invalid. New projects default to CXS and both subs OFF.
- Screensaver now plays forward, holds the final frame for 2.2 seconds, reverses to frame zero, then loops. It reuses all original 64 native animation frames without storing new images or running inside the audio callback.
- Firmware/OTA protected bootloader and stock V15 compatibility fix of V1.44 preserved. Physical FM1 DMA/flash verification remains necessary after installation.

# RUPTURE V1.44 — Official Stock V15 OTA Compatibility (ERROR 3)

- Reproduced `OTA PREP FAILED / ERROR 3` with the **original M-VAVE stock V15** `FM-1_015` firmware package, SHA-256 `db1642b2b6fa5c2cccb11ffd13878068bb28601678d3644049f99dc40e7edb8a` (699,956 bytes). The official package has **eight UFW entries**, while the old OTA validator erroneously required **exactly two**. The firmware refused it during CHECK before flash was erased.
- Recognize the exact eight-entry stock V15 metadata structure (`flash.bin`, `info.log`, `USR`, `isd_config.ini`, `ota.bin`, `script.ver`, `blimit.bin`, `tail.bin`) as a separate, strictly validated package family. Keep the original two-entry rules for all app-only firmware; do not stage auxiliary metadata. Bounds, ordering, list/header CRC, full flash CRC and the pinned stock image and loader SHA-256 remain mandatory.
- Fixed a second refusal that followed the first one: the RUPTURE boot area differs from official V15 in only **four known bytes** (`uboot.boot` directory CRCs at `0x20..0x23`). Accept only this exact, verified difference when restoring the exact pinned official V15; any other bootheader mismatch remains error 5. Official V15 uses its own verified full-flash loader, not the custom app-only writer.
- Native C host replay of the complete original V15 SysEx file now reaches the OTA handoff/commit with a RUPTURE V1.43 boot image. Negative tests confirm no staging writes on corrupted header or flash, unknown OTA residual, or unknown bootheader. Two-entry app-only firmware regression retained.
- No synthesis, audio DSP, UI, storage, firmware identity, bootheader, included OTA-loader binary or project format changes. Physical FM1 tests of the return to stock and interrupted transitions are still necessary; users already running a blocking older firmware need a functioning OTA or ROM-rescue path to install this fix.

# RUPTURE V1.43 — Cross-Firmware OTA compatibility and stock-restore safety

- Accept and safely reclaim a bounded, structurally recognized legacy `LOADER.BIN` left in the OTA staging region by a previously installed compatible firmware. Previously the exact SHA allowlist only recognized ETCH's own staged loader or official V15, so some FM1 units failed OTA PREP with a valid loader from another firmware already present.
- Never run that foreign residual loader. Validate the new incoming package normally, stage the pinned trusted loader for app-only updates, protect reserved areas, and retain failure-safe cleanup.
- Reject any official V15 *full-flash* handoff when the actual installed 16-KiB boothead is different from the requested stock package (`-5`). This prevents a blind bootloader rewrite. Normal app-only packages remain independent of the source boothead.
- Show the numeric OTA failure code on the FM1 UPDATE screen, to distinguish staging refusal `-15`, boothead mismatch `-5`, stock validation `-20` and other errors instead of only `OTA PREP FAILED`.
- Add native C regression tests for blank stage, recognizable residual stage, unknown residual data, damaged staging headers and protected boothead comparison.
- **Limitation:** official stock V15 package is not bundled or replayed in these tests. Physical compatibility and older installed V1.42 instances still require a known-good OTA or ROM rescue path; software tests are not physical proof.

# RUPTURE V1.42 — OCTAVE FUZZ Audio Stability

- Fixed a reproducible audible-scale PCM discontinuity during rapid DRIVE/OCTAVE/TONE adjustments, without changing the existing controls or project data.
- DRIVE, OCTAVE and TONE control values are converted to fixed-point targets outside the ISR and smoothed sample by sample, preventing instantaneous distortion and tone jumps.
- OCTAVE FUZZ full-wave rectification now has a bounded, first-derivative-continuous transition at the zero crossing, reducing high-frequency alias-generating edges while retaining its aggressive octave-up character.
- The loudness-matching target updates at the existing control rate, but makeup gain now ramps every sample instead of stepping once every 32 samples. A flash-resident reciprocal table replaces the variable-integer divide in the audio callback.
- Kept existing four-band EQ, all other FX, project data PJT34, settings, audio deadline protection and OTA loader unchanged. Physical FM1 playback/DMA stress verification is required.

## V1.41 — Channel EQ 2.0 gain and realtime stability (2026-11-01)

- Corrected Channel EQ 2.0 gain staging. The previous fixed `-sum(positive band boosts)` trim could reduce total volume by up to 36 dB even when the musical intent was to boost bass. This global attenuation is removed. A bounded, signal-dependent peak guard engages only close to digital clipping.
- Fixed EQ BYPASS with saved non-neutral controls: an OFF EQ is now genuinely dry and cannot activate wet output compression merely because stored EQ gains are nonzero.
- Reduced active-EQ audio-interrupt work by updating coefficients at controlled intervals, caching unity/active-band status and skipping coefficient ramps once settled. Nominal frequency responses and Q controls remain unchanged.
- Corrected neutral-EQ publication so 0 dB is always exact passthrough, including freshly initialised EQ states.
- Added repeatable tests for bass boosts combined with other bands, cut/boost/frequency/Q sweeps, dry parity, clipping protection and UBSan.
- Retained existing PJT34 project format, Q controls, source filter coefficients, protection of boot/OTA sectors and DMA watchdog. No firmware update or test run can establish physical FM1 stability until measured on hardware.

## V1.40 — FX CPU budget and default EQ bypass (2026-11-01)

- Fixed the shared post-mix FX scheduler: disabled chorus, delay and reverb no longer process full wet DSP at MIX 0 when DRIVE, OCT FUZZ or another effect is on. Their current smoothed MIX remains authoritative, preserving audible fade-outs and original algorithms.
- Optimized the RUPTURE eight-effect dispatch to bypass dormant legacy ETCH modulation engines while retaining the full path when a legacy project uses them.
- Removed a compressor toggle click: disengaging compression now releases its gain smoothly instead of jumping instantly to unity.
- New projects start with Channel EQ 2.0 selected but fully bypassed (OFF), with neutral gain parameters. Existing saved project EQ state and bypass mask remain unchanged when loaded.
- No changed bootloader, OTA loader, storage structure, filter algorithm or DMA safety cutoff. Physical FM1 timing and stability are not yet validated for this revision.

# RUPTURE V1.39 — Dry Audio Stability Fix

- Fixed Channel EQ 2.0 at 0 dB incorrectly preventing the global FX dry fast-path. It previously forced non-trivial EQ and effect processing even with all filters/FX off.
- Flat EQ2 coefficients and inactive bands now bypass their heavy realtime calculations while preserving active-EQ processing, stored settings and parameter transition behavior.
- Fast neutral pitch-bend and filter-OFF paths reduce unnecessary operations for RUPTURE's monophonic CXS engine.
- Native FM1 build and targeted host regression tests pass. A physical FM1 DMA soak test remains required; this is not a claim of independently measured elimination of crackling.
- The DMA buffer ownership protection, audio-stop guard, protected boot header, trusted OTA loader and project storage format are unchanged.

# RUPTURE V1.38 — Full filter optimization & monophonic DMA protection

- Prepared and cached filter coefficients for LOWPASS, BANDPASS, HIGHPASS and 24 dB/oct LADDER. Stable controls no longer recompute the same coefficients for every sample. The filter output and internal state remain sample-exact against previous kernels.
- Corrected the DMA overload response for RUPTURE's actual one-voice architecture: ineffective 8→2 voice shedding is replaced with immediate nonessential UI/scope throttling. Persistent genuine missed deadlines still trigger the audio safety stop.
- Kept DSP pole counts, cutoff/resonance mapping, fixed-point gain, project files (PJT34), effects, EQ2, panel controls, USB, flash layout and trusted OTA loader unchanged.
- Native crossbuild and 2.5 million new sample/state comparisons pass. This does not replace physical FM1 deadline, audio and power-on stress tests.

# RUPTURE V1.37 — Ladder realtime stability and audio fault diagnostics

- Replaced five signed 64-bit multiplies per sample/voice in the resonant four-pole ladder hot path with overflow-bounded 32-bit Q15 arithmetic. The resulting output and all four internal states are exact matches to V1.36 for valid audio input, including maximum resonance.
- Added Q15 input bounding in the ladder branch to prevent upstream values outside the documented audio range from destabilizing the feedback loop. Ordinary signals are unaffected.
- Kept audio DMA deadline preflight, voice-budget relief, audio-stop protection, boot/recovery behavior, project formats and stored sounds intact. The audio watchdog is **not disabled**.
- A deadline-induced audio shutdown now shows **AUDIO STOP / DMA TIMING FAULT** rather than incorrectly presenting it as a boot SAFE MODE. Genuine Safe Mode remains available with its original meaning.
- Completed native JieLi crossbuild and 972,000 sample-exact filter comparisons, including 300,000 automated parameter-sweep samples. Physical FM1 measurements remain necessary before declaring the reported crackle and shutdown fixed on hardware.
- Preserved the original protected 16 KiB flash header, OTA-loader bytes, firmware identity `FM-1_968` and existing PJT34 storage format. All features from V1.36 retained.

# RUPTURE V1.36 — Storage recovery and Auto Load Project

- WIPE ALL uses a dedicated confirmed and address-restricted native NOR erase/readback path over known user flash 0x97000..0xDFFFF. Standard project writes remain confined to the existing RUPTURE A/B banks. Boot, application and OTA sectors remain protected.
- Fixed deletion of the last saved project in the dedicated project bank pair; an unsaved NEW project can be created afterwards. Shared-waveform-owner deletion while surviving projects depend on it remains safely blocked and now explains why.
- Rejected project edits no longer permanently poison the storage state. Writable state is restored only when a committed valid project catalog is readable.
- Added GLO → SETTINGS → AUTO LOAD PROJECT. It defaults to NEW; SELECT browses only existing projects plus NEW; SEL chooses the power-on target. Missing/deleted targets fall back to NEW. The choice is CRC-protected in the existing 8-byte device-global flash journal and survives regular save and bank compaction.
- Preserves Channel EQ 2.0, PJT34 compatibility, project and sequence data format, audio DSP, protected 16-KiB bootheader and original trusted OTA loader.
- Native FM1 crossbuild and directed host NOR storage tests passed. Flash behavior and cold-boot AUTO LOAD still need physical FM1 verification.

- **V1.35:** EDIT cycles EQ GAIN / FREQUENCY / Q, SEL no longer cycles.
# RUPTURE V1.34 — Channel EQ 2.0 and parametric peak width

- New fixed-point post-mix four-biquad EQ: LOW 80 Hz shelf, BODY 250 Hz bell, BITE 1.5 kHz bell and AIR 5 kHz shelf, all with ±9 dB gain in 0.5 dB steps.
- EDIT opens GAIN / FREQUENCY / PEAK WIDTH (Q); repeated EDIT presses cycle the pages, never SEL. K1–K4 adjust the four gains or center frequencies; K2/K3 adjust BODY/BITE Q from 0.3 to 8.0. Lower Q is wider; higher Q is narrower. EQ2 response plot uses the actual coefficients.
- Gain is bit-identical at 0 dB; coefficient smoothing and post-boost headroom management minimize clicks and clipping. Biquads are implemented in fixed point without per-sample transcendental calculations.
- New projects use EQ2. Older projects load LEGACY by default, with optional ALGORITHM switch to EQ2. PJT34 stores EQ mode, four frequency selectors and both Q values without enlarging the known flash erase layout. Previous PJT33 records remain decodable.
- Original OTA loader, protected flash header, other effects, sequencer and MIDI remain unchanged. Native build and host regressions pass; physical FM1 CPU, sound and flash tests still required.

# RUPTURE V1.33 — Sequence save/load and UI alignment

- Fixed the first-save sequence bug: newly named empty slots no longer trigger a false overwrite confirmation before note data is committed to flash.
- Saving a sequence now persists its BPM and step length per sequence in existing patch metadata, with no project-format expansion. Loading restores the saved pattern, BPM, length, rate, gate and swing.
- Older sequences without the new metadata marker use stored project timing as a fallback. A save that failed in a previous release cannot be retroactively recovered if its pattern was never written.
- Legacy sequence name placeholders `ETCH24`–`ETCH31`, with or without a space, start with an empty name field; existing populated patterns and custom names are retained.
- The PROJECTS and SEQUENCES save/open browsers show the opened entry with `ACTIVE`, separately from the selection highlight.
- Both small pitch-octave and HOLD readouts use the same centered value column on the native 240 × 240 sequencer screen.
- Preserved the CXS audio DSP, storage project journal, first 16 KB bootheader and trusted OTA loader.

## V1.22 · FX stability, mixer & fifth SUB knob

- Removed WAVEFOLDER and TRANSIENT SHAPER from RUPTURE FX browsing and audio rendering. Remaining FX: eight. Old saved settings remain decode-compatible; legacy code is still present for ETCH compatibility.
- Octave Fuzz: dry/wet envelope-based loudness normalization after distortion, preserves full Drive and Octave intensity.
- Tilt EQ: stronger, independent K1 Tilt, K2 Frequency, K3 Mix, K4 Output.
- Sandpaper: independent and audible SOFT/WARMTH ranges, smoothed controls and reduced parameter-switch transients.
- Channel EQ K4: HIGH CUT replaced with HIGH AIR treble boost (0 neutral); historical K4 values adopt new meaning.
- SUB 1 and SUB 2: fifth virtual BLEND knob, physical ALGORITHM control, independent gain 0–100; saved per project inside existing 21-byte PJT33 extension with new tag; older PJT33 blobs load with default BLEND 75.
- Important downgrade warning: newly saved V1.22 PJT33 records cannot load on V1.21 or older. Keep backups; real FM1 stability unverified.

## V1.21 — SAVE project menu on every HOME page
- Fixed a native navigation omission: SAVE now opens the same named 12-slot PROJECTS browser from MAIN, SOUND SHAPE, SUB OSC 1 and SUB OSC 2. Previously SUB 1 and SUB 2 ignored the button.
- SEQ / SEQ SLOTS still route SAVE to the independent eight-sequence library. The FX+SAVE service chord remains reserved. Project writes still require explicit browser actions and confirmation; no automatic flash write is added to SAVE navigation.
- Added native shared SAVE routing and regression coverage for all four HOME pages, non-home behavior and the protected chord. No changes to CXS DSP, FX, audio DMA, flash schemas or trusted OTA loader.
- Updated the official `index.html`, README, CHANGELOG, cheat sheet and embedded installer together with firmware V1.21. The physical button test on FM1 remains outstanding.

## V1.20 — Unified information and installation website
- Replaced the separate installer entrypoint with a professional `index.html` containing auto-loaded README.md / CHANGELOG.md, a searchable cheat sheet and the original safe Web MIDI installer.
- Added embedded offline documentation snapshots and one-click download of the SHA-256-validated firmware. The page always uses the release file when served via GitHub Pages, so routine documentation edits update without rebuilding the firmware.
- Added deterministic website generation with a strict `.fwsc` / embedded installer / release-manifest SHA-256 parity check. The OTA loader and firmware image are unchanged by this website update.

## V1.20 — RUPTURE / SIKKENWORKS Animated Boot Intro
- Replaced 10-stage pixel-assembly boot logo with 24-frame pure 1-bit jagged crack / widening rupture intro; final exact wordmark and slogan: SIKKENWORKS / Born out of passion.
- Single consistent 240×240 art source prevents mismatched crack geometry from frame to frame; all intermediate frames are generated by the actual native C UI engine from a 7,200-byte read-only bitmap.
- Runs exclusively before audio init; no additional per-sample CPU work, video buffer, heap, data-flash changes or modified trusted loader. FM1 watchdog fed during the reveal and final hold.
- Existing 1-bit screens, 12×8 named storage, ten FX, CXS/sub mix and hardware-recovery priority retained. Host tests verify intermediate 1-bit PNGs and final bitmap parity. Physical FM1 boot timing not yet tested.

## V1.19 — Realtime DMA Telemetry and Priority for Audio
- Added GLO → AUDIO DIAG: WALL MAX, DSP MAX, RENDER, PUBLISH, MIN SLACK, LATE, GUARD and SCOPE SKIP. All are read-only, update at most once per second on screen, and are reset at audio init.
- The ALNK interrupt now times core synth+FX rendering separately from DMA/USB/visual scope publishing. Each counter is a fixed-size scalar; no audio heap, memory copy, printf, file I/O or flash write added to realtime work.
- Under high ISR wall-time pressure the visualization capture is deferred rather than contending with audio DMA and USB. This never drops an audio or MIDI frame. A higher SCOPE SKIP count is a visual-performance indicator, not an audio failure.
- Regression test for DMA preflight, interrupts, timer wrap, visual pressure thresholds, as well as all previous twelve-project and ten-FX stress tests.
- Protected first 16 KiB, OTA loader, PJT33 structure and the entire audible CXS and FX DSP algorithms remain unchanged. Real-hardware DMA deadline tests are still necessary.

## V1.18 — FX Full-Chain Peak Guard & DSP Hot-Path
- FX-bus soft knee now engages for the compressor, wavefolder, transient, tilt, octave fuzz, Sandpaper and EQ as well as time-based effects. Maximum wet PCM peak is 28,000/32,767, with a division-free knee starting at 22,000. Full neutral bypass remains bit-exact.
- Removed compiler-emulated signed 64-bit divides from the hot per-sample processing of the four newer FX, Sandpaper blending, and echo/reverb feedback. All narrowed products have explicit signed 32-bit range bounds. FX feedback remains bounded and unchanged in the A/B storage format.
- A hard DMA preflight remains enabled: software-only host tests cannot guarantee that an FM1 at maximum FX will never overrun. On-device worst-case TIMER4 wall-time and extended power/USB load soak remain REQUIRED. This release does not disable the protective audio-stop behavior or suppress diagnostic counters.
- Strong Wavefolder and Octave Fuzz drive each uses a small pre-nonlinearity one-pole filter (8 B total additional DSP state) to reduce high-frequency alias energy without 2x CPU oversampling. This is mitigation rather than alias-free synthesis.
- All existing 12-project PJT33 storage, FX settings, project SAVE/autosave, and boot/OTA sectors remain compatible. The limiter protects final PCM, not nonlinear aliasing already generated by wavefold/fuzz at Nyquist; a full hardware quality proof remains pending.

## V1.17 — FX Memory & Project Compatibility

- Introduced PJT33: a bounded **21-byte** project-snapshot extension for Transient Shaper, Tilt EQ, Octave Fuzz and Sandpaper, all sixteen K1–K4 settings, their four enabled flags, and the existing 22-bit FX bypass mask.
- Manual project SAVE and guarded 20-second autosave capture this FX state before the existing CRC-checked A/B journal commits; no flash operation is added to the audio interrupt path or another sector.
- LOAD and cold boot restore the correct project FX. Old PJT32 projects restore all four new FX to OFF with documented neutral parameter defaults and bypass reset, preventing cross-project leakage.
- Bad, missing or truncated PJT33 extension data is rejected. Host regression tests include roundtrip, corrupt masks/knobs, legacy format migration, project isolation, and all prior firmware tests.
- Existing 33-byte per-preset DSP record remains unchanged; the 12 named project / eight sequence catalog and protected first 16 KiB + trusted OTA loader remain unchanged.
- PJT33 projects **are not backward-readable in V1.16 or older firmware**. Back up projects before a downgrade. Hardware FM1 storage/OTA validation and audio CPU profiling remain pending.

## V1.16 — Sandpaper
- Added SANDPAPER as the tenth native RUPTURE FX selection with its own 1-bit screen and K1 SMOOTH, K2 SOFT, K3 WARMTH, K4 MIX.
- New stereo-linked post-CXS signal smoothing uses a 2-stage IIR low-pass, gentler peak knee, controlled bass-body shelf and smoothed dry/wet fade. Three tiny filter states, no delay buffer, no heap and no flash changes.
- Host DSP tests verify 95 Hz bass retention, 4.5 kHz harmonic reduction, knob response, bounded full-scale audio, exact dry bypass after fade. The established V1.15 FX tests remain passing.
- Sandpaper defaults OFF, settings session-only, physical FM1 listening and CPU load profiling still pending.
- Native pi32v2 firmware, 37 bit-exact 1-bit renderer exports and firmware WebInstaller.

## V1.15
- Added COMPRESSOR and WAVEFOLDER to RUPTURE FX (SEL toggle, SELECT scroll, four native knob parameters). They use the existing persisted native FX settings and no additional sample buffers.
- Added audible post-CXS TRANSIENT SHAPER (ATTACK/SUSTAIN/SENS/MIX), TILT EQ (TILT/FREQ/MIX/OUTPUT) and OCTAVE FUZZ (DRIVE/OCTAVE/TONE/MIX) in native fixed-point DSP.
- Each of the five FX is independently enabled/bypassed. All default off; old dry audio is bit-exact at defaults. New algorithms have bounded state, no heap and a short mix ramp on bypass.
- 36 true 1-bit native screen exports, project and existing OTA flash map unchanged. Native audio host tests added for all five effects.
- Transient/Tilt/Octave settings remain session-only pending storage format and backwards-compatibility validation; physical CPU/audio stress remains unverified.

## V1.14
- Introduced per-effect click-reduction ramps for chorus, delay and reverb on bypass toggles.
- Maintained exact dry CXS output, unchanged flash layout, recovery loader and user project format.
- Added a linker-section SRAM and FX recommendation audit. Hardware timing remains unverified.

## V1.13
- Device-global Master Volume persists independently of project/preset loads using a CRC-validated append-only NOR record.
- Verified project bank swap preserves volume; exhausted journal safely compacts with verified A/B commit.
- Fixed native 296-byte PJ52 catalog write to comply with the FM1 HAL 256-byte maximum and page boundaries.
- Corrected boot-time resetting of Master Volume to 100%.
- CXS bass, dual subs, existing project/sequence storage and trusted OTA-loader preserved.
- Host regressions and native JieLi build pass; physical device test still pending.

## V1.12
- Store and restore NOTE MEMORY (LAST/HIGH/LOW, LEGATO, GLIDE 0–400 ms in 5 ms increments, BEND 0–12) with each saved CXS preset/project snapshot.
- Master Volume 0–200% now survives project SAVE/LOAD and restart for the saved active CXS preset. The value is **project/preset-scoped**, not a device-wide separate global setting.
- Old snapshots without V1.12 marker restore safe defaults (100%, LAST, LEGATO ON, GLIDE 0, BEND 2).
- No new flash sector, project codec version, or change to 33-byte PJT32 records.
- Original OTA loader and 16 KiB protected header preserved; hardware validation still required.

## V1.11 — Dual Sub Preset Memory
- SUB 1 and SUB 2 now persist TYPE, OCTAVE, DETUNE, DRIFT per saved preset and project.
- Legacy presets without a sub marker load both subs OFF with independent neutral octave/detune defaults.
- Extra tag validation rejects orphaned/malformed sub records; existing PJT32 record size and A/B flash bank layout unchanged.
- SOUND SHAPE memory and original CXS-off output retained.
- Native pi32v2 build and host roundtrip tests added. Physical flash/reboot verification still open.

## V1.10 — Suboscillator label + audibility fix

- Corrected both suboscillator knob label rows to TYPE / OCTAVE / DETUNE / DRIFT (missing NAME EDITOR label row had shifted array indices).
- Replaced attenuating sub crossfade with additive sub summing that maintains 100% main CXS level while subs are enabled.
- Increased sub contribution limit to 50% each, with bounded peak guard above 28k Q15.
- SUB 2 defaults to -1 octave instead of -2 to improve audibility on typical audio playback; -2 remains selectable.
- Independent SUB 2 default detune +4 cents; CLASSIC upgraded from pure sine to a harmonically band-limited square-like oscillator; TYPE remains OFF by default to preserve earlier patches.
- Added focused regression tests for both UI labels, no-level-drop mix, per-sub harmonic energy, bounded peaks and both-OFF parity.
- No persistent storage/OTA changes; physical listening and CPU measurements remain outstanding.


## V1.9 — Dual independent suboscillators

- Added SUB OSC 1 and SUB OSC 2 as dedicated HOME knob pages after SOUND SHAPE.
- Both subs offer independent TYPE OFF/CXS/CLASSIC (sine), OCTAVE −2/−1/0/+1, DETUNE ±50 cents, and DRIFT 0–100.
- Native CXS sub generators use separate exchange states and random seeds; CLASSIC uses band-limited sine lookup. Both share the played monophonic note, glide, and MIDI pitch.
- Bounded pre-filter gain compensation prevents summing three full-scale generators; short fades protect TYPE changes. Both OFF preserves the former main CXS signal.
- Added host synthesis tests and 31 native C-renderer pixel-perfect screens. Native crossbuild and .fwsc validation pass.
- Sub parameters currently session-only; no project format changes. On-device tests remain open.


## V1.8 — Named libraries, SAVE context and keyboard step playhead

- SAVE now opens PROJECT LIBRARY from PLAY/SOUND SHAPE, or SEQUENCE LIBRARY from SEQ, without performing a silent immediate write.
- New 12-project and 8-sequence scrolling text lists, with LOAD / SAVE / NAME actions.
- Implemented a 12-character on-device name editor using SELECT, ALGORITHM, SEL and HOME.
- Project titles remain in their existing 12-slot catalog; sequence titles use reserved 24–31 patch name fields already serialized in PJT32.
- Native white-key sequencer position LEDs: sixteen steps per page, dim availability and full-brightness active step. The SEQ screen follows the actual playing step.
- Codec roundtrip name persistence and white-key mapping tests passed; FM1 native build, .fwsc verification and OTA parity checked.
- Physical hardware validation still required; older RUPTURE versions may ignore reserved sequence titles.


## V1.7 — SAVE, STEP VELOCITY AND IDLE AUTO SAVE

- Restored the physical SAVE key (button bit 9) in RUPTURE's native UI routing. Physical REC (bit 13) is now independent and edits only SEQ recording.
- SAVE directly commits the current active project through the bounded, CRC-verified two-bank project writer; on-screen footer confirms success or failure, with no silent inaction.
- K3 edits the velocity of every existing note on the currently selected sequence step (1–127), and controls the starting velocity for an empty step.
- SEQ opens with an immediately editable step, without requiring initial scrolling; keyboard note-on events can edit the selected step even during playback.
- Added delayed idle-only Auto Save after the initial explicit project SAVE or LOAD: >=20 seconds since the last edit, >=60 seconds between writes, only when sequence playback is stopped, no held keys, and the storage is owned, valid and ready. Failed writes are not auto-retried without a new change.
- USB lightning icon is now keyed to configured USB connectivity rather than momentary suspended state. USB power without enumeration remains unverified as a charge indication.
- Increased active native screen refresh responsiveness, preserving the native 1-bit renderer and the original OTA loader/bootheader.
- Built native FM1 image, verified .fwsc checksum, host regression tests; no physical FM1 validation performed.


## V1.7 — SAVE, STEP VELOCITY AND IDLE AUTO SAVE

- Restored the physical SAVE key (button bit 9) in RUPTURE's native UI routing. Physical REC (bit 13) is now independent and edits only SEQ recording.
- SAVE directly commits the current active project through the bounded, CRC-verified two-bank project writer; on-screen footer confirms success or failure, with no silent inaction.
- K3 edits the velocity of every existing note on the currently selected sequence step (1–127), and controls the starting velocity for an empty step.
- SEQ opens with an immediately editable step, without requiring initial scrolling; keyboard note-on events can edit the selected step even during playback.
- Added delayed idle-only Auto Save after the initial explicit project SAVE or LOAD: >=20 seconds since the last edit, >=60 seconds between writes, only when sequence playback is stopped, no held keys, and the storage is owned, valid and ready. Failed writes are not auto-retried without a new change.
- USB lightning icon is now keyed to configured USB connectivity rather than momentary suspended state. USB power without enumeration remains unverified as a charge indication.
- Increased active native screen refresh responsiveness, preserving the native 1-bit renderer and the original OTA loader/bootheader.
- Built native FM1 image, verified .fwsc checksum, host regression tests; no physical FM1 validation performed.


## V1.5 — TWELVE PROJECT CATALOG

- Upgraded the RUPTURE owned-flash P05 catalog from 4 to 12 slot addresses within the existing two 80 KiB banks; 8 sequence archives remain per project.
- Added new catalog version 13 (PJ52), while reading and migrating existing version 11/12 four-slot catalogs without changing the already committed bank until the new CRC-checked payload is committed.
- Added true 16-bit occupancy mask and 12-entry project names; native PROJECTS navigation and the 1-bit menu now expose slots 1–12.
- A failed write or exhausted bank capacity returns an error before commit; the last valid snapshot remains available. Twelve large fully populated projects cannot all be guaranteed to fit in 80 KiB per bank.
- Added native flash-emulator tests for all 12 records, legacy four-slot migration, corrupt/torn write fallback and slot deletion; run together with prior regression tests.
- Retained CXS, soundshaping, note memory, effect chains and bass register unchanged. Preserved the original OTA loader and protected 16 KiB flash header.
- WARNING FOR DOWNGRADE: V1.5 saves catalog version 13. Earlier RUPTURE versions do not read that new catalog. Make a project backup before installation and do not downgrade without exporting/restore preparation.
- Native build and container valid offline. Physical FM1 installation and power-cycle tests remain outstanding.

## V1.4 — SOUND SHAPE PRESET MEMORY

- WEIGHT, EDGE, BITE and PUNCH now persist per preset and per saved project in the existing PJT32 per-preset DSP bank.
- Reuses four bytes formerly assigned to the ETCH Organic oscillator, which RUPTURE CXS never executes. Tagging uses the reserved high bit of spectral_wave only for RUPTURE-owned patch records; no new flash areas, sector erase policies, snapshot sizes or boot components.
- Upgraded the RUPTURE project-capture path to PJT32, preventing accidental downgrade of saved shape data to PJT30.
- Legacy/older RUPTURE projects with no shape tag restore neutral 0/50/0/0 settings, avoiding invented values.
- The legacy organic pitch-drift data is never interpreted as CXS shape controls and is bypassed for RUPTURE playback.
- Extended project codec validation, rejecting shape markers on non-RUPTURE presets; added encode/decode, backwards-compatibility and corruption-guard host tests.
- Native pi32v2 app and firmware container rebuilt; byte-identical native rebuild tested. Same FM-1_968 identity, protected first 16 KiB and original OTA loader.
- Four hardware-backed projects and 8 sequence slots each remain; 12 project slots, TRS MIDI OUT and native loader-screen progress remain unimplemented.


## V1.3 — CXS LFO + SOUND SHAPE

- Routed the actual LFO directly into monophonic CXS audio: KEYED/FREE/TEMPO with PITCH vibrato and LEVEL tremolo targets. Previously the UI changed an unused wavetable modulation path.
- Short HOME on PLAY opens a second native four-knob page: WEIGHT, EDGE, BITE, PUNCH. HOME again returns to the main CXS controls; long HOME returns to PLAY.
- Added four bounded fixed-point CXS sound-shaping processors, all directly controlled by hardware K1–K4. Neutrals preserve the previous dry CXS waveform; EDGE 50 preserves original smoothing.
- Replaced the header font with a dedicated heavy 8x9 pixel RUPTURE wordmark drawn by the native 1-bit C renderer.
- 28 screen exports from exactly the same native firmware renderer, including SOUND SHAPE.
- Added PCM-difference, conserved-cycle and output-range host checks for the new LFO and shaping controls.
- Updated firmware identity metadata, WebInstaller payload/hash and flashable `.fwsc`. Protected bootheader and OTA loader remain byte-identical to the validated reference package.
- Flash storage layout remains unchanged: 4 native project slots with 8 sequence slots per project. Extra SOUND SHAPE controls are runtime-only pending safe storage migration.

## V1.2

- CXS PRESSURE/BALANCE/MEMORY/DENSITY now survive preset and project save/load without changing project-record size; legacy patches remain readable.
- Added native 31250-baud TRS MIDI IN over UART1, RX-DMA ring and channel voice/realtime parser.
- Interleaved USB and TRS messages use the existing bounded audio MIDI queue; overrun triggers panic instead of stuck notes.
- Added running-status, MIDI clock, sustain CC, pitchbend, system-common, and SysEx-discard tests.
- Fixed onboard HELP: 4 available project slots, not 12 already present.
- Kept the protected boot header and original trusted OTA loader intact.
- Not implemented: native twelve-project flash expansion, TRS MIDI OUT, separate loader LCD progress, proof of charging current, physical FM1 validation.


## V1.1

- Corrected the virtual potentiometer geometry: 0 at 7:30, 50 at 12:00, 100 at 4:30; smooth interpolation retained.
- Fixed RUPTURE LFO page to control existing C-wave modulation RATE, DEPTH, MODE and TARGET.
- Drive CUTOFF automatically enables the ladder filter when the selected filter was OFF, so the control has an audible effect.
- Channel EQ K4 clearly labeled LOW CUT; its DSP high-pass filter acts only on bass frequencies at the left of the curve. LOW/MID/HIGH gains remain independent.
- Battery icon adds a lightning symbol when the USB data interface is configured. This indicates USB connection, not verified charging current.
- Installer progress reports actual firmware bytes served during the independent trusted OTA-loader stage and remains visible across handoff; no fake progress is shown for unobserved steps.
- Unchanged trusted bootloader, protected flash header, CXS bass sound, project layout and MIDI protocol.
- Updated documentation, 1-bit screen captures and release hashes.


## V1.0

- Corrected ARP mode display so internal mode 0 renders OFF, not UP; the remaining mode names now match their native DSP behavior.
- Implemented the native step-sequencer UI: SELECT scrolls steps; the keyboard adds/removes per-step note triggers when stopped. REC enables live capture to the active playhead.
- K1 selects 4/8/16/32/64/128 steps, K2 adjusts the shared 30–240 BPM, K3 sets new-note velocity and K4 sets step gate.
- Updated native patch restore to recognize stored four-step sequences instead of silently reverting to sixteen.
- Channel EQ opens with EDIT, not REC. REC returns to its recording function on the SEQ screen.
- Boot logo renders immediately after LCD init, before subsequent lengthy startup and storage initialization; 1.5-second finished-logo hold retained.
- Removed the redundant fixed "K1 K2 K3 K4" screen caption, including in onboard HELP, without removing labeled parameter controls.
- Removed the spurious `README.me`. README contains product and full credits; version history is only in CHANGELOG.
- Rebuilt the FM1 application and exact-pixel 1-bit screen exports; retained protected bootheader and verified OTA loader.


## V0.9

- Corrected virtual K1–K4 dial rotation (positive knob movement now moves clockwise) using the same native C pixel renderer for the FM1 and PNG exports.
- FX: SELECT cycles REVERB, DELAY, CHORUS, MOSFET, SEL toggles the focused effect; K1–K4 edit its live DSP parameters. Enabling a zero-mix effect supplies a safe audible starting mix.
- Reverb SPRING / HALL / ROOM offer distinct functioning decay/size presets within the inherited DSP core; physically modeled spring tank not claimed.
- REC opens native black/white Channel EQ with a live response graph, 3 gain bands and HPF, all four parameters wired to native DSP.
- SETTINGS now shows only working K1 MASTER VOLUME, no unused K2–K4 controls.
- Battery icon uses Felucca ADC voltage thresholds 531/561/591 with smoothing and valid-reading checks; no fabricated percentage. BEETLE battery UI approach credited as adaptation.
- Updated HELP, README, CHEAT SHEET, installer, native PNG exports, tests and flash package. Preserved native CXS, OCT 0 baseline, six-note memory, project data sectors, boot header and trusted OTA loader.


## V0.8

- Added native **8 sequence slots per verified project** to RUPTURE, backed by the existing CRC-checked P05 A/B project snapshot. No new flash regions.
- Long-press SEQ opens a fully functional LOAD/SAVE screen; SELECT picks a sequence slot and ALGORITHM chooses action; SEL enters, double SEL confirms SAVE. K1 SLOT, K2 ACTION, K3 RATE, K4 SWING.
- Saved sequences retain notes, holds, velocity, lengths, parameter locks, rate, gate and swing. Loading stops transport before switching pattern and avoids changing live data on corrupt archive entries.
- Wired native PROJECTS screen to actual four existing project slots with explicit load/save confirmation. Unverified slots 5–12 are not exposed as writable.
- Added explicit save status and capacity/failure feedback; corrected status text to avoid implied success.
- Preserved CXS audio, OCT 0 pitch, 6-note memory, boot header and trusted OTA loader.
- Added host regression for all eight saved sequence patterns and corruption isolation; regenerated 26 exact native 1-bit screen PNGs.

## V0.7

- Made the former absolute OCT −2 register the RUPTURE baseline **OCT 0**.
- RUPTURE OCT 0 uses exactly the same oscillator pitch and key mapping as V0.6 OCT −2.
- OCT− and OCT+ LEDs now indicate the octave **relative to RUPTURE 0**; neither stays lit at baseline.
- Main-screen and other RUPTURE headers explicitly show `OCT 0`; shifting OCT+ once shows `OCT +1`, OCT− once shows `OCT -1`.
- Physical octave travel remains unchanged (absolute −4..+3, shown as RUPTURE −2..+5).
- Tested the display translation, LED status, press limits and unchanged bass-note MIDI mapping; regenerated exact native UI PNGs.
- No changes to CXS synthesis, note memory, filters, bootloader, OTA loader or existing patch storage.

## V0.6

- Introduced the working low bass register (then called OCT −2), corrected OCT buttons and LEDs, and improved encoder polling, knob animation and boot animation.

# RUPTURE V1.32 — Utrecht screensaver, settings & REC LED

- Removed the square R monogram from the GitHub Pages website header; SIKKENWORKS text remains.
- Replaced the meadow screensaver with original author-supplied monochrome Utrecht artwork (Domtoren and city buildings) and a staged street fracture that expands into a canyon. Uses only the existing 240×240 UI framebuffer; audio DSP and USB updater unchanged.
- SETTINGS → OTHER → SCREENSAVER now opens a dedicated options page: ENABLED toggle with SEL, START AFTER adjustable by K1 when selected with SELECT. Values: 15, 30 (default), 45, 60, 90, 120, 180, 300 seconds.
- A/B global preference record remains 8 bytes, with backward-compatible decoding of all prior autosave/saver settings and new persistent delay index. Device-global settings survive project switches and volume changes.
- REC function-button LED is OFF by default and illuminates only when live sequence recording is armed. It extinguishes when recording is toggled off or transport stops.
- No audio engine, storage region/layout, protected bootheader or trusted OTA loader changes.

# RUPTURE V1.31 — Original QR restoration

- REPORT A BUG now embeds the **exact 29x29 module bitmap from the author-uploaded QR image**, preserving all 841 modules. Reconstructed 29-module QR in V1.30 deviated on six modules and used reversed colors.
- The original supplied white-on-black QR is rendered without interpolation at 5x5 FM1 pixels per module, centered in its own screen.
- The original uploaded PNG is preserved unchanged in `assets/qr-code-original.png` with SHA-256 verification and source extraction script. No QR regeneration.
- All previous RUPTURE functions, persistent storage formats, DSP, bootheader, and OTA loader remain unchanged.

# V1.30 — Live STORAGE catalog, legacy sequences, pixel meadow and bug-report QR

- STORAGE → PROJECTS now explicitly refreshes the native catalog on entry. The list no longer depends on first visiting the normal PROJECTS page.
- Empty legacy ETCH factory-labelled 24–31 patch slots are ignored only when no pattern data exists. Saved sequences with real patterns or deliberate custom names remain visible; no stored data is changed by this display fix.
- MASTER VOLUME values and AUTO SAVE ON/OFF are vertically centred in their SETTINGS cards and use a consistent right-hand value area.
- 30-second idle screensaver becomes a deterministic meadow and widening pixel ravine, with cliff edges, strata, grass, branching fractures and falling fragments. Rendered using the existing 240×240 1-bit UI canvas, not the DSP. The final frame holds instead of snapping back.
- GLO now adds REPORT A BUG as a fifth card, opening a sharp embedded 29×29 QR symbol which scans to https://github.com/SikkenWorks/RUPTURE/issues and reads SCAN TO REPORT A BUG below it.
- Bootloader/OTA loader, protected first 16 KiB, DSP and flash storage schema preserved. Native build and tests have been run; physical checks on FM1 remain necessary for changed behavior.

# V1.29 — Global menu labels and direct SETTINGS controls

- GLO now displays **AUDIO DIAGNOSTICS** in full, without truncating the card label.
- SETTINGS > AUTO SAVE is a direct checkbox: SELECT chooses its row and SEL toggles ON/OFF immediately. No separate screen. The choice retains the existing persistent global journal behavior and defaults OFF.
- SETTINGS > MASTER VOLUME allows K1 to adjust the percentage directly when its row is highlighted; SEL still opens the dedicated volume page and there SELECT or K1 adjusts the same 0–200% control.
- SETTINGS > OTHER > SCREENSAVER remains a direct SEL toggle with no separate page, ON by default.
- UI-only navigation improvements; existing audio engine, data layout, OTA loader and first 16 KiB bootheader are unchanged.
- Host tests validate rendered labels and the input routing source contracts. Physical FM1 confirmation is still required for this build.

# V1.28 — Storage Manager, ABOUT, USB indicator and refined screensaver

- Fixed only the missing center stroke of the E in the top-left RUPTURE header, leaving the other cracked letterforms unchanged.
- USB-C charging/connection bolt now follows live USB configuration/suspend/disconnect state and disappears when the cable is unplugged.
- SETTINGS > STORAGE > PROJECTS: inspect 12 saved projects; OPEN, SEQUENCES, DUPLICATE to first empty slot, RENAME, DELETE. Eight per-project sequences can be opened, duplicated, renamed and deleted through the existing native pattern archive. Operations are CRC/commit-checked without replacing a stored project with unrelated live RAM.
- SETTINGS > STORAGE > WIPE ALL: defaults NO, requires YES via SELECT+SEL, erases and verifies known user-storage/legacy sectors 0x97000..0xDFFFF only. Preserves boot, application, OTA staging and trusted loader. Irreversible. Active project and shared wave-owner protections stay in effect for individual deletes.
- SETTINGS > OTHER > SCREENSAVER: enabled by default. Choice persists in existing 8-byte A/B global preference records with old records transparently interpreted as enabled. AUTO SAVE remains opt-in OFF by default.
- Screensaver now draws branching, opening asymmetrical fissures with pixel fragments. UI framebuffer only; no DSP, audio ISR or allocated audio buffers changed.
- GLO > ABOUT: complete author-supplied English introduction, correctly word-wrapped and scrollable using SELECT on the physical 240×240 panel.
- README updated exclusively as product documentation, control guide and upstream credits, with no version/change history; this CHANGELOG contains the historical entries.
- Native storage, UI and sequence manager regressions passed in host tests. Protected first 16 KiB and trusted OTA loader stay identical to prior release. Author to verify new behavior on real FM1 hardware.

# V1.27 — Screensaver, Compact Sequencer, Global Settings and UI Controls

- UI-only 30-second idle screensaver: black panel and progressive jagged cracking, using existing 1-bit framebuffer; wakes on activity. Transport, notes, OTA, recovery and unsafe storage prevent activation. No audio DSP modifications.
- SEQ length 4/8 only draws the actual active steps; unused grid boxes are hidden. Eight- and four-step layouts are centered.
- AUTO SAVE is now opt-in and defaults OFF. The device-global setting survives power cycles, project writes and A/B log compaction using the existing 8-byte NOR records (new magic for ON, legacy records read as OFF). No project-format changes.
- GLO uses outlined cards matching HELP; SETTINGS has MASTER VOLUME and AUTO SAVE, navigated by SELECT + SEL. MASTER uses SELECT/K1 after entry; AUTO SAVE uses SELECT for OFF/ON, SEL to apply.
- Removed the generic ALGORITHM-to-K1/K2/K3/K4 fallback; explicit mappings for SUB BLEND, sequence sustain, name editing and HELP scrolling stay intact.
- README rewritten as permanent product documentation, user controls, installation, licensing and full upstream attributions. Release history is kept only in CHANGELOG.md.
- Updated native UI screens and regressions. Trusted OTA loader, bootheader, firmware identity and synthesis/FX/audio ISR unchanged. Physical validation of the new release remains pending.

# V1.26 — Slot Save/Load Semantics, Battery Calibration & OTA Transfer Progress

- Project/sequence library: SEL or SAVE on an empty slot opens the name input; SAVE on an occupied slot overwrites (with confirmation when different from the current slot); SEL on an occupied slot prompts before loading. SAVE+SEL on occupied slot retains RENAME.
- Name editor removes the redundant SEL NO ACTION footer, leaving SAVE TO STORE.
- Battery 100% interior now fills all 24 pixels; the full ADC threshold changes from 591 to 578, with the old invalid-read and smoothing protections intact. Actual battery calibration still needs checking on user hardware.
- Browser installer removes blanket claims that RUPTURE is untested on physical FM1s and tracks unique transferred bytes for meaningful progress. Device OTA loader remains unchanged: actual on-device flash progress is not claimed.
- Full original synthesis/FX/MIDI/USB/storage and existing firmware identity FM-1_968 retained. No changes to flash record layout or protected boot area.

# V1.25 — SAVE Integrity, Storage Progress, USB Branding & Bipolar EQ

- Knob spacing corrected on all standard K1–K4 pages: upper knob centers y105, lower y181; title area, lower values and footer no longer overlap.
- USB runtime descriptors now report manufacturer **SIKKENWORKS** and product **RUPTURE** instead of ETCH. The pinned updater/loader handshake and `FM-1_968` package identity are unchanged. Old host OS MIDI-port caches may need USB reconnect.
- The NAME editor now uses **SAVE only** to write. **SEL does nothing** to the name; SELECT moves the cursor and ALGORITHM edits characters. On failed SAVE, the typed name remains available for retry.
- First explicit SAVE is permitted on a verified blank RUPTURE-owned project bank, while unknown/foreign, unapproved or unsafe flash regions remain protected. Project and sequence writes use the existing CRC-verified A/B journal and report failures accurately.
- Native 1-bit storage popup overlays report actual ERASE / WRITE / VERIFY / READ progress from completed NOR operations. Sequence RAM loads are naturally nearly instant. Overlay callbacks run only between flash commands, with the audio engine paused.
- CHANNEL EQ K4 is a signed treble shelf: **-100 to +100** with neutral zero. Lowpass high-band attenuation and positive AIR boosting both work. Existing positive-only K4 projects migrate using a new marker (bit 5) in the unchanged 21-byte PJT33 FX record. Older versions do not understand this marker.
- Device OLED/LCD progress *during the separate OTA loader's flash stage* is **not implemented**: the trusted self-contained loader owns the display during the transfer. The browser's real progress remains; no unverified modifications to recovery/boot code.
- Regression coverage includes the full native audio/FX suite, first-save on erased NOR, 12 projects, A/B torn writes, progress phases, USB descriptors, signed EQ, and source routing. Real FM1 SAVE/LOAD and OTA tests still required.

# V1.24 — Dual-SUB + FX Headroom, CLASSIC Sub Fix & UI Polish

- Corrected SUB mixing under effects: small fixed-point dynamic headroom allocation, continuous soft-peak safety, and sample-rate BLEND slew. The sub-disabled CXS path stays byte-for-byte dry-equivalent.
- CLASSIC now enters after the resonant CXS filter but before ADSR, velocity and the eight-effect chain, ensuring CLASSIC does not disappear when the filter closes. CXS subs retain their filter routing. This is a deliberate sound change for CLASSIC patches.
- Tested alternating CLASSIC/CXS sub channels and changing ALGORITHM blend while eight RUPTURE effects are fully active, including delay/reverb feedback, in 110250 frames; no full-scale PCM clipping and no systematic silence in the host test.
- Moved upper knob visuals lower so the 50–80px label panels are not touched; added a 1-bit cracked RUPTURE wordmark with native-rendered output.
- No changes to PJT33 flash schema, 12 projects, eight sequences, independent FX saving, USB/MIDI, OTA loader or initial bootheader.
- Hardware validation still needed: actual FM1 DMA deadline, high-load audio and perceived loudness matching; host tests are not guarantees.

# V1.23 — Named SAVE, Sequencer Sustain & Scrollable HELP

- Fixed PROJECTS and SEQUENCES save UX: SEL in NAME editor now commits the name and the appropriate stored data; the physical SAVE key also commits. When already inside PROJECTS or SEQUENCES with SAVE selected, physical SAVE performs the save action rather than reopening/resetting the list.
- Added visible confirmation/error status in both slot libraries: SAVED, SAVE FAILED, SEL AGAIN TO SAVE and LOADED. Occupied slots require an explicit second confirmation.
- On sequencer step with a note, ALGORITHM extends native note duration across empty following steps. It stops at the next attack, sequence boundary or 255-step native limit; turning left shortens the duration. The visual grid distinguishes held steps from triggered steps.
- Reworked HELP into 10 scrollable cards with chapter titles, page counters, and scroll indicators. SELECT changes cards, ALGORITHM scrolls three lines at a time.
- Increased the size of "Born out of passion." in the actual 240×240 monochrome boot ROM, regenerating all 24 true native frames.
- Preserved PJT33 project data, CXS DSP, eight live effects, dual SUB BLEND, OTA loader and initial bootheader. All new user-visible behaviour requires physical FM1 confirmation.

# CHANGELOG
# RUPTURE V1.94: per-slot FX parameters (software candidate)

- Each of eight FX slots stores independent K1–K4 values. Repeat types no longer share displayed/editable parameter values. Moving a slot moves its values, and saves/loads preserve them.
- Separate state for Distortion, Wah, Compressor, Tilt, Fuzz and Sandpaper. MOSFET is stateless. Chorus and Delay expose separate tap/modulation/mix controls but share one PCM history each. Reverb still shares one tank for all slots; only MIX is fully independent between Reverb slots.
- The old single-effect DSP retains sample-exact output when parameter values match. Zero additional large PCM ring buffers. Extra simultaneous active processors still require extra CPU and are not physically verified for FM1 live use.
- PJT35 receives a tagged 66-byte optional tail; previously saved PJT35 files remain readable. V1.93 rejects the new extended files.
- Preserved stock-derived protected bootheader and trusted OTA. Physical installation and stress testing pending.

