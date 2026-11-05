from pathlib import Path
r=Path(__file__).resolve().parents[1]
n=r/'vendor/etch-v1.88-source/firmware/fm1-native'
p=n/'rupture_ui.c'; s=p.read_text();assert '"V1.83","FM-1_968"' in s;s=s.replace('"V1.83","FM-1_968"','"V1.84","FM-1_968"',1);p.write_text(s)
p=n/'build.py';s=p.read_text().replace("'version':'RUPTURE V1.83'","'version':'RUPTURE V1.84'");assert "'version':'RUPTURE V1.84'" in s;p.write_text(s)
p=n/'native_led_policy.h';s=p.read_text();s=s.replace(' * The 12 non-octave function buttons remain lit. Octaves indicate offset.',' * Function buttons have page-aware intensity in RUPTURE V1.84. Octaves indicate offset.');p.write_text(s)
p=r/'README.md';s=p.read_text();needle='### Basic navigation';assert needle in s
s=s.replace(needle,'''### Hardware integration (V1.84)

RUPTURE uses its FM1 function LEDs as direct feedback. The selected hardware section is **bright** (HOME for PLAY / SOUND SHAPE / SUB, ENV, LFO, FX, EDIT for channel EQ, ARP, SEQ or GLO for utility screens). Other function buttons stay discoverable at the existing **high-frequency soft glow**, rather than switching slowly on and off. Pressed buttons immediately take priority. The PLAY LED stays bright while the sequencer is playing, REC while recording is armed/active, and SEQ pulses on the two-bar count-in. Octave and keyboard LEDs continue their established behaviors. During protected update, recovery and first-run screens, the established full-brightness button policy is preserved. These are UI-thread policies; **there are no new audio callback operations or LED timer ISR changes**.

When switching pages (including choosing a different FX engine), the acceleration history of K1–K4 resets. The first physical detent on a newly selected page therefore cannot inherit a doubled step from the previous page. Existing fast-turn acceleration on continuous controls and exact single-detent changes on discrete controls are otherwise retained. Knob focus markers and visible values still respond immediately. These behaviors are software-tested but require a physical FM1 check to confirm perceived LED brightness and encoder feel.

'''+needle,1)
s=s.replace('RUPTURE-V1.83.fwsc','RUPTURE-V1.84.fwsc')
p.write_text(s)
p=r/'CHANGELOG.md';s=p.read_text();s='''# RUPTURE V1.84 — Hardware Integration

- **Contextual function LEDs:** active hardware page is bright; inactive function keys retain the existing fast dim glow. Physical presses always override the page indicator. Sequencer PLAY, armed/live REC and SEQ count-in each have unambiguous LED ownership. Normal octave, note, GUIDE and GRID feedback is preserved.
- **Encoder consistency:** a page change resets K1–K4 acceleration history, so a fast turn on the previous page cannot double the first movement on the new page. Selecting another FX panel also resets the history; discrete controls and existing accelerated continuous controls otherwise behave as before.
- **Realtime isolation:** changes only in UI/control-loop routing and button LED planes. No changes to the synthesizer DSP, three reverb engines, IRQ timing, HAL scan implementation, project schema, Welcome completion logic or native OTA writer.
- **Validation limits:** cross-compiled native firmware and host tests, including function LED priority and count-in logic. Physical LED perceived brightness, responsiveness and DMA/GUARD counts still need verification on actual FM1 hardware.

'''+s;p.write_text(s)
p=r/'CHEAT-SHEET.html';s=p.read_text();needle='<section><h2>V1.83 MONO HEADROOM</h2>';assert needle in s
s=s.replace(needle,'''<section><h2>V1.84 HARDWARE INTEGRATION</h2><p>Only the active hardware section stays fully lit: HOME for PLAY/SHAPE/SUB, FX for all effect pages, EDIT for channel EQ, ENV, LFO, ARP, SEQ and GLO for utility pages. Inactive buttons retain a steady-looking fast dim glow. Pressed buttons take priority; PLAY indicates the running sequencer, REC indicates armed/live recording, and SEQ pulses during the two-bar count-in. K1-K4 acceleration resets on page changes and FX category switches, so the new page starts with one unit per detent. Audio, reverb, MIDI, projects and OTA behavior are unchanged. Physical FM1 validation is still required.</p></section>
'''+needle,1);p.write_text(s)
p=r/'WEBSITE-DEPLOY.md';s=p.read_text().replace('RUPTURE-V1.83.fwsc','RUPTURE-V1.84.fwsc').replace('RUPTURE-V1.83-WEBSITE-GITHUB-PAGES.zip','RUPTURE-V1.84-WEBSITE-GITHUB-PAGES.zip');p.write_text(s)
p=r/'WebInstaller.html';s=p.read_text();assert 'V1.83' in s;s=s.replace('V1.83','V1.84');p.write_text(s)
print('Prepared V1.84 source metadata and documentation')
