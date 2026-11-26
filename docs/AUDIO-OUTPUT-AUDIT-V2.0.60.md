# RUPTURE V2.0.60: jack-output investigation

Status: software candidate, not measured on a physical FM1. A clean build and host tests cannot establish silence at an analogue jack or prove power-off suppression.

## Findings in the supplied V2.0.59 source

1. `audio_target.c:native_pause()` disabled PC6 immediately, then stopped ALNK/I2S, without ramping the outgoing PCM to zero. Every project/preference write could therefore interrupt a nonzero sample and change the output-enable state. Whether PC6 also gates a particular board revision's headphone amplifier needs a circuit measurement; its software label is not a schematic.
2. `native_start()` cleared and restarted DMA, forced PC6 low and ran the cold analogue startup sequence again. SAVE/LOAD therefore introduced hardware transitions that are unnecessary for storage. Warm restarts also waited through the cold start gates.
3. `native_store_read()` borrowed the flash controller and disabled interrupts around bit-banged SPI reads. Catalog and sequence-list reads could reach it while the audio DMA was still rendering. A blocked ISR can miss a DMA deadline and replay stale samples. These are credible software mechanisms for clicks/crackle, not an analogue measurement of the user's noise.
4. Manager/rollback paths can nest pause/start calls. Restarting a codec twice compounds transitions. A file operation may also have another catalog read immediately after a commit.
5. Power-off is different: the inspected application has no power-switch notification or qualified rail-collapse detector. PB1 is a battery divider, sampled roughly every 1.5 seconds for a coarse icon. PB6 reads MASTER, roughly every 20 ms. Neither constitutes demonstrated early power-fail notification. The PC6 enable alone is not a verified headphone shorting/mute circuit. Its binary edge can itself click. No undocumented GPIO/ADC/PMU register was repurposed as a speculative fix.

## Implemented changes

- Foreground requests a 256-frame linear output fade (about 5.8 ms at the native 44.118 kHz rate). It follows the existing preset/output processing and attenuates the actual final stereo PCM, including effects tails.
- Two entirely zero blocks must pass the real DMA preflight AND the post-copy deadline test. A bit mask requires both distinct DMA halves; seeing the same half twice is insufficient.
- Only then is synthesis suspended. ALNK keeps clocking the two zero buffers throughout NOR erase/program/read, even with interrupts masked. Normal storage never disables PC6, stops I2S or reinitializes the codec.
- Warm resume retains the qualified analogue/screen gates, resets runtime state as before, and fades audio in using the existing 8192-sample ramp. The DMA buffers remain zero until the ISR owns them. Cold boot still performs the original gated codec startup.
- All five raw store callbacks enforce the handoff, including catalog reads and bounded legacy wipe. Tiny reads in one foreground operation share a pause and resume once on the next permitted main-loop pass. Explicit mute, MIDI-only mode, boot, OTA and faults cannot cause an automatic unmute.
- Nested pauses and duplicate starts are idempotent. A 50 ms timeout catches absent/rejected DMA progress. This exceptional path disables the hardware output, blocks storage I/O and latches an audio fault until explicit HOME+PLAY rearm. OTA staging callbacks also refuse access if the handoff failed or audio is not suspended.
- Oscillator, CXS/GROWL, SUB, native FX, storage codec/journal, flash layout, HAL, trusted OTA loader and protected header retain their prior sources/bytes. Output gating intentionally changes PCM during storage. One conditional per frame is added to the final publish loop; real DSP/DMA overhead needs hardware measurement.

## Power-off conclusion

The changes remove storage-related abrupt PCM stops and repeated codec/enable switching. They do **not** implement or prove suppression of an analogue transient when the power switch removes the supply. If the DAC/amp bias collapses after the processor stops, firmware cannot change that event without an independently powered/early-triggered hardware mute path. An actual board measurement must establish whether such a path already exists and whether there is sufficient advance warning. If absent, a qualified hardware mute/anti-pop modification is required; component values and GPIO connections cannot responsibly be chosen from this source tree alone.

Do not interpret STABLE in the filename as physical qualification of this revision. It names the existing single-core engine variant. Both variants remain hardware-unverified; CORE2 also retains its experimental status.

## Physical acceptance procedure (outstanding)

Use low interface/monitor gain initially. Preserve a known working firmware for comparison.

1. Record the stereo jack on an interface at fixed gain. Establish the noise floor with all notes released and effects tails finished. Save projects, presets, preferences and sequences repeatedly; exercise SAVE failures, catalog navigation and project LOAD/COPY/RENAME/DELETE. A brief intentional fade/mute is expected during physical flash access; crackle, a DC step, oscillation, or a hardware-enable pop is not.
2. Repeat with a held tone and a heavy FX tail where the UI permits the operation, then with TRS and USB MIDI. Check that there are no stuck notes, no endless mute and no additional DMA LATES/GUARD faults. A storage pause still clears audio/note state as the prior implementation did; this is not seamless live flash streaming.
3. Compare simultaneous USB capture and jack capture. If USB remains digitally clean while the jack has a spike during flash, investigate analogue supply/ground coupling rather than changing the DSP blindly. Repeat on battery without USB and on USB power to separate those paths.
4. Run at least 100 SAVE/LOAD cycles and a prolonged playing/MIDI/USB session separately on each variant. Check every saved project after reboot. Physical I2S continuity and PC6 stability through storage should be observed on a scope/logic analyser.
5. For power-off, record the jack with digital silence already established, then repeat while a tone plays. Measure output DC, DAC/amp rails, PC6 and processor power concurrently. Determine the first rail/switch edge and whether the processor has time to mute. This is required before promising a power-off fix. Keep the receiving interface/monitors muted when physically switching off while this remains unresolved.

No physical test above was performed in this environment. No assertion of zero crackle or pop-free shutdown is made.
