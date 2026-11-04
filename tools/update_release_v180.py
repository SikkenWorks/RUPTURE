from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
# Firmware runtime page uses its own string, independent of USB identity.
p=root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c';s=p.read_text()
assert '"V1.78","FM-1_968"' in s;s=s.replace('"V1.78","FM-1_968"','"V1.80","FM-1_968"');p.write_text(s)
p=root/'vendor/etch-v1.88-source/firmware/fm1-native/build.py';s=p.read_text().replace("'version':'RUPTURE V1.79'","'version':'RUPTURE V1.80'");p.write_text(s)
# README remains evergreen, with latest behavior replacing outdated old summary.
p=root/'README.md';s=p.read_text()
start=s.index('## V1.79: audio deadline and headroom improvements')
end=s.index('\n## ',start+5) if '\n## ' in s[start+5:] else len(s)
text='''## Reverb engines and realtime safety (V1.80)

RUPTURE runs three distinct global **ROOM / HALL / SPRING** algorithms. ROOM uses a four-delay feedback-delay network with damping and decorrelated early arrivals; SPRING uses two cross-coupled delay paths and two dispersive allpass filters, including dispersion **inside** feedback. HALL retains its established diffuse reverb algorithm and V1.79 PCM character. These are algorithmic approximations rather than impulse responses of any specific physical room or spring tank. K1 selects the engine, K2 DECAY, K3 SIZE and K4 MIX. SIZE changes actual line lengths; at MIX 100 the output contains only the reverb return. The engines reuse the existing 4,096-sample reverb buffer plus existing 64-sample diffuser, with only a few bytes of additional control state. A new reverb algorithm may alter a preset's sound; save or back up projects before downgrading.

In the shared effects processor, active phaser, tremolo, auto-wah, tape and decimator Q15 processing now uses mathematically exact bounded 32-bit operations rather than unnecessarily expensive per-sample 64-bit arithmetic. The other synthesizer features, oscillators, SUB drift and HALL are unchanged. On a host, the full-HALL all-effects PCM output remains **byte-identical to V1.79**. The architecture does not reduce polyphony, FX availability or sample rate, but the more realistic ROOM network has extra memory reads/writes and may use more CPU than V1.79.

For diagnostics, go to **GLO → AUDIO DIAGNOSTICS**. `DSP MAX` is a historical maximum; `DMA LATE` and `GUARD` are cumulative. After reboot, verify that both counters stay **unchanged at zero** when actively playing both SUBs with fast DRIFT movements, maximum effects and rapid navigation. Even the user's observed 1/1 indicates a past deadline or guard event. Software crossbuilds cannot prove zero glitches or adequate device CPU margin. Do not consider this a hardware-certified studio/live release until full FM1 stress and listening tests pass. Retain a project backup and verified Pico recovery option.
'''
s=s[:start]+text+s[end:];s=s.replace('RUPTURE-V1.79.fwsc','RUPTURE-V1.80.fwsc');p.write_text(s)
p=root/'CHANGELOG.md';s=p.read_text();new='''# RUPTURE V1.80 — ROOM / SPRING Rebuild & Fixed-Point DSP

- **ROOM:** replaced the shallow single-ring reflections with four independently delayed, damped lines in an energy-distributing Householder FDN. SIZE alters line lengths; DECAY controls loop feedback. A fully wet ROOM return no longer relies on low-level taps from an unrelated reverb design.
- **SPRING:** replaced sign-alternating feedback with two differently sized cross-coupled delay loops and cascaded short allpass dispersers, including loop-dependent re-dispersion. Damping and restrained spectral shaping target a more usable spring-like tail rather than the previous brittle metallic sound.
- **HALL:** preserved the existing algorithm and its native PCM behavior. The three engines share the same established 4,096-sample tank and 64-sample diffuser; just one engine processes each frame. K1 TYPE, K2 DECAY, K3 SIZE and K4 MIX remain unchanged. Project format / default legacy-HALL migration unchanged.
- **CPU work:** replaced redundant 64-bit Q15 calculations in phaser, tremolo, auto-wah, tape and decimator with *exact* bounded 32-bit products. Removed per-sample SIZE/DECAY-to-gain conversions from ROOM/SPRING: coefficients are cached at the parameter-change boundary. No new oscillator, FX instance, reverb PCM buffer or UI ISR.
- **Validation:** native FM1 crossbuild, 2-million-frame topology switching and bounds test, SIZE/DECAY response tests, high-gain SUB+FX stress for all three reverb engines, Hall sample-for-sample golden comparison, and native package/hash checks. Original OTA source, protected bootheader and trusted loader preserved.
- **Caveat:** ROOM uses a more capable delay network and can be slightly more expensive. Hardware DSP MAX, live DMA LATE/GUARD behavior and subjective realism still require actual FM1 measurements and listening. Never claim guaranteed zero underruns without physical testing.
- See `docs/TEST-V1.80.md` for results and limitations. Includes original synthesized stereo WAV comparisons in `demos/`.

'''
s=new+s;p.write_text(s)
p=root/'CHEAT-SHEET.html';s=p.read_text();needle='<section><h2>V1.79 AUDIO HEADROOM / SUB DRIFT</h2>';assert needle in s;s=s.replace(needle,'''<section><h2>V1.80 REVERB ENGINES & HEADROOM</h2><p>FX → REVERB: K1 selects SPRING / HALL / ROOM. K2 = DECAY, K3 = SIZE (actual delay geometry), K4 = MIX (100% wet). ROOM now uses a four-delay feedback network with damped early and late reflections; SPRING uses two coupled delay lines and cascaded dispersive allpasses, with dispersion inside the feedback loop. HALL keeps its existing sound. The upgraded ROOM/SPRING use the original 4,096-sample reverb RAM, and the rest of the synthesizer is intact. Phaser, auto-wah, tape, tremolo and decimator Q15 products are optimized exactly. For live safety, open GLO → AUDIO DIAGNOSTICS: DMA LATE and GUARD should not increase during maximum FX + both SUBs. Host tests do not guarantee FM1 hardware headroom; keep backups and recovery ready.</p></section>\n'''+needle,1);p.write_text(s)
p=root/'WEBSITE-DEPLOY.md';s=p.read_text().replace('V1.79','V1.80');p.write_text(s)
p=root/'WebInstaller.html';s=p.read_text().replace('V1.79','V1.80');p.write_text(s)
p=root/'RELEASE-MANIFEST.json';m=json.loads(p.read_text());m['version']='1.80';m['firmware_file']='RUPTURE-V1.80.fwsc';m['hardware_verified']=False;m['physical_fm1_verified']=False;m['v180']={
 'reverb_engines':['Spring: coupled delay lines + dispersive feedback','Hall: PCM-identical prior diffuse tank','Room: 4-line damped FDN'],
 'existing_reverb_sample_buffer':4096, 'new_pcm_buffers':0,
 'fixed_point_exact_other_effects':True,
 'hardware_dma_late_guard_confirmed_zero':False,
 'stress_tests':'host/native only; physical audio and CPU unverified'}
m['limitations'].append('V1.80: ROOM and SPRING use distinct more sophisticated global delay networks; credibility on speakers and hardware DMA deadlines is not certified. ROOM may increase CPU vs V1.79 despite fixed-point optimizations. Do not use as live/studio guaranteed stable without testing on FM1.')
p.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n')
print('Updated display version, README, CHANGELOG, cheat sheet, installer labels and release manifest')
