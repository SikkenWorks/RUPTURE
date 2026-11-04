from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
p=root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c';s=p.read_text();assert '"V1.80","FM-1_968"' in s;s=s.replace('"V1.80","FM-1_968"','"V1.81","FM-1_968"');p.write_text(s)
p=root/'vendor/etch-v1.88-source/firmware/fm1-native/build.py';s=p.read_text();assert "'version':'RUPTURE V1.80'" in s;s=s.replace("'version':'RUPTURE V1.80'","'version':'RUPTURE V1.81'");p.write_text(s)
p=root/'README.md';s=p.read_text();insert='''## V1.81: Rapid-note realtime reliability

V1.81 targets rising **DMA LATE** and **GUARD** counts when playing rapid notes. The monophonic synth now avoids needless eight-voice rate and modulation housekeeping on note changes, and performance-gate countdowns no longer copy the full held-note stack on every PCM sample. The existing MIDI, panel-key, sustain, sequencer and ARP behavior is preserved. On a desktop host, long fast-note and sequence-gate PCM comparisons match V1.80 exactly; on-device cycle savings remain to be measured.

The LCD formerly treated every Note ON/OFF edge as an urgent full-screen transfer, even during dense playing. At rapid note rates V1.81 coalesces nonessential LCD frame updates to 75 ms, or 100 ms under recent audio pressure, with a final frame when activity stops. **No key, MIDI event, rendered audio sample or DSP parameter is delayed or dropped.** Explicit screen/menu controls retain priority. Visual feedback can become less frequent during very fast playing to protect the audio bus. The screensaver, UI assets and existing three reverb algorithms remain unchanged.

**Hardware validation is still required.** Restart your FM1, note the baseline DMA LATE, GUARD, DSP MAX and MIN SLACK, then repeat the fast-note stress that raised the counters in V1.80. The important result is that LATE and GUARD **do not increase**. Host PCM identity and cross-compilation cannot guarantee zero underruns on a real FM1. Keep a project backup and a verified Pico rescue option.

'''
pos=s.find('## Reverb engines and realtime safety (V1.80)');assert pos>=0;s=s[:pos]+insert+s[pos:];s=s.replace('RUPTURE-V1.80.fwsc','RUPTURE-V1.81.fwsc');p.write_text(s)
p=root/'CHANGELOG.md';s=p.read_text();s='''# RUPTURE V1.81 — Rapid-Note Headroom & LCD DMA Coalescing

- **Fast-note mono DSP:** compute pitch increments and organic/LIVING control state only for RUPTURE's single active mono voice, rather than eight unused ETCH polyphonic slots. MIDI note-on, note-off, legato, step gates and all sound processors remain unchanged.
- **Timed mono gates:** do not rewrite the held-note stack on each audio sample when no sequence note expired. On a genuine gate expiration compact the stack exactly as before, at the same sample.
- **LCD contention:** rapid key presses previously marked every frame as an urgent SPI LCD update, bypassing UI pressure throttling. Coalesce only visual updates during detected note bursts to at most ~13 fps (~10 fps under audio pressure). User interface actions retain priority; no change to audio/MIDI event handling, modulation, or sample rate. Animation frames are skipped only as redundant intermediate displays, not removed from assets.
- **Reverbs:** SPRING, ROOM and HALL DSP files and audio algorithms untouched. Project storage, bootheader and OTA loader untouched.
- **Host verification:** byte-identical PCM output in a 501,760-frame rapid MIDI/step-gate/sustain/FX scenario, host note-burst UI budget tests, native FM1 crossbuild. Physical DMA LATE/GUARD and DSP cycle budget still need confirmation; cannot guarantee zero hardware underruns from host tests.

'''+s;p.write_text(s)
p=root/'CHEAT-SHEET.html';s=p.read_text();needle='<section><h2>V1.80 REVERB ENGINES & HEADROOM</h2>';assert needle in s;s=s.replace(needle,'''<section><h2>V1.81 RAPID NOTES / AUDIO HEADROOM</h2><p>Rapid-note mono pitch and gate processing now avoids idle eight-voice work. Dense note playing coalesces only decorative LCD updates (75 ms; 100 ms at audio pressure), not notes, samples, MIDI or parameters. The three ROOM/HALL/SPRING reverb engines are identical to V1.80. For a hardware check, restart, open GLO → AUDIO DIAGNOSTICS, record DMA LATE/GUARD, play very fast notes with both SUBs and full FX; both counters must remain unchanged. Host tests do NOT certify zero FM1 hardware underruns.</p></section>
'''+needle,1);p.write_text(s)
p=root/'WEBSITE-DEPLOY.md';s=p.read_text().replace('V1.80','V1.81');p.write_text(s)
p=root/'WebInstaller.html';s=p.read_text().replace('V1.80','V1.81');p.write_text(s)
print('Updated RUPTURE V1.81 runtime version, metadata and documentation')
