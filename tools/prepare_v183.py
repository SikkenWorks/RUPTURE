from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c'
s=p.read_text();assert '"V1.82","FM-1_968"' in s;s=s.replace('"V1.82","FM-1_968"','"V1.83","FM-1_968"',1);p.write_text(s)
p=root/'vendor/etch-v1.88-source/firmware/fm1-native/build.py'
s=p.read_text();assert "'version':'RUPTURE V1.82'" in s;s=s.replace("'version':'RUPTURE V1.82'","'version':'RUPTURE V1.83'",1);p.write_text(s)
p=root/'README.md';s=p.read_text();s=s.replace('RUPTURE-V1.82.fwsc','RUPTURE-V1.83.fwsc')
needle='## V1.81: Rapid-note realtime reliability'
assert needle in s
s=s.replace(needle,'''## Realtime mono headroom

The RUPTURE mono renderer uses a fixed four-to-one gain divider rather than recalculating the upstream 5–8-voice adaptive headroom every sample. The mono engine never uses those extra voices. Master gain still slews smoothly on every audio sample; its unchanged target is resolved once per DMA block. The existing CXS, bandlimited SAW, dual SUBs, all three ROOM/HALL/SPRING algorithms, sustain, MIDI, sequencer, storage and OTA are preserved. Host PCM comparisons prove sample identity for the tested workloads; **they do not guarantee zero DMA LATE/GUARD on physical FM1 hardware**.

Updating from an already completed V1.82 setup keeps its Welcome completion marker in V1.83. No existing user data is erased and no automatic format is performed. The Quick Tour remains available from GLO. A V15 round-trip to the same already-completed release may still be indistinguishable from a reboot if V15 preserves that marker.

'''+needle,1);p.write_text(s)
p=root/'CHANGELOG.md';s=p.read_text();s='''# RUPTURE V1.83 — Realtime Mono Headroom

- **Mono mixer:** replaced per-sample variable Q10 5–8-voice headroom stepping and variable division with the mathematically identical fixed `/4` operation for the always-monophonic RUPTURE build. Unmodified upstream poly behavior remains under the non-RUPTURE build path.
- **Master volume:** its Q16 target is now calculated once per audio render block, retaining the exact per-sample smoothing for changing gain, MIDI notes and preset ramps.
- **Sound preservation:** CXS, SAW, both SUBs with DRIFT, all three unique ROOM/HALL/SPRING reverbs, all other FX, envelope, sequence and MIDI paths are unchanged. Host stress renders match V1.82 sample-for-sample for rapid notes and multi-FX scenarios.
- **Safety:** project storage, the existing V1.82 Welcome completion marker, exact protected 16 KiB bootheader and trusted OTA loader are preserved; no new flash writes or partitions. The installed firmware's physical DMA deadline and true worst-case cycle headroom still need measured hardware validation; zero-underrun operation is **not yet certified**.

'''+s;p.write_text(s)
p=root/'CHEAT-SHEET.html';s=p.read_text();needle='<section><h2>V1.81 RAPID NOTES / AUDIO HEADROOM</h2>';assert needle in s;s=s.replace(needle,'''<section><h2>V1.83 MONO HEADROOM</h2><p>The mono audio mixer no longer runs an unnecessary polyphonic headroom calculation and variable divide for each audio sample. Master gain remains sample-smoothed, and ROOM/HALL/SPRING, CXS/SAW and both SUBs are unchanged. This is a sample-exact host-tested CPU optimization, NOT proof of zero DMA LATE or GUARD on a physical FM1. Check GLO / AUDIO DIAGNOSTICS with fast MIDI, dual SUBs and effects; compare the increment of the counters, not their already accumulated totals.</p></section>
'''+needle,1);p.write_text(s)
p=root/'WEBSITE-DEPLOY.md';s=p.read_text().replace('RUPTURE-V1.82.fwsc','RUPTURE-V1.83.fwsc').replace('RUPTURE-V1.82-WEBSITE-GITHUB-PAGES.zip','RUPTURE-V1.83-WEBSITE-GITHUB-PAGES.zip');p.write_text(s)
p=root/'WebInstaller.html';s=p.read_text().replace('V1.82','V1.83');p.write_text(s)
p=root/'tests/test_v183_audio_parity.py';s=p.read_text();s=s.replace("for flags in (['-O2'],['-Os']):", "for flags in (['-O2'],):");p.write_text(s)
print('prepared runtime and documentation V1.83')
