# RUPTURE V2.0.43

**V2.0.43 herstelt de Note OFF-veiligheid.** In de GROUPS MIDI-leermodus
werden eerder alle MIDI Note OFFs stilzwijgend weggegooid. Een noot die
vóór het openen van die modus al speelde, kon daardoor blijven hangen.
De leermodus onderschept nu uitsluitend echte Note ON-aanslagen; Note OFF
(`0x80` en `0x90` met velocity 0) bereikt altijd de synthesizer.

De mono-engine controleert daarnaast **eens per audioblok** of een fysieke
toets in de notenlijst daadwerkelijk nog ingedrukt is. Verdwenen contacten
worden vrijgegeven, zonder een bewust lang aangehouden toets of MIDI-sustain
af te kappen. Bij USB-/TRS-MIDI-overflow stopt de engine de betrokken
MIDI-noten en behoudt hij onafhankelijk klinkende fysieke toetsen en
sequencernoten. Beide audiocores gebruiken dezelfde beveiligingen.

Dit zijn broncode- en hostgeteste correcties, **geen hardwarebewijs dat een
hangende noot onder alle omstandigheden onmogelijk is**. Gebruik eerst de
STABLE-variant. Voor MIDI-test: laat een noot klinken, open GROUPS MIDI
learn, stuur Note OFF en controleer dat die noot stopt. Herhaal met fysiek
klavier, USB, TRS, sustain, sequencer, zware effecten en CORE2 afzonderlijk.

**Nieuw: jouw eigen SikkenWorks-PNG en echte FX-berekening op core 2 in
een apart, experimenteel firmwarebestand.**

- **STABLE:** één core verwerkt CXS/GROWL/SUB en de complete effectketen.
  Dit blijft de referentie voor veilig dagelijks gebruik.
- **CORE2 EXPERIMENTAL:** core 1 (CPU0) bouwt de droge synthesizermonsters;
  core 2 (CPU1) verwerkt de **volledige effectketen**, 64 samples per
  opdracht. Tijdens de eerste opdracht berekent core 1 de volgende
  64 synthesizermonsters. Daarna wordt de stereo-output samengevoegd,
  zonder extra bufferlatentie van een heel audioblok.
- USB-C-opname op 48 kHz behoudt in de experimentele variant de FIR op
  core 1, zodat twee taken nooit om core 2 concurreren.
- Na een core2-timeout wordt de core gestopt en neemt core 1 de
  onvoltooide effecten over. Dit is nog **niet hardwarebewezen**: een
  fout tijdens een effectmonster is niet gegarandeerd herstelbaar.

Het logo tijdens de ingekorte **5,16-secondenintro** is nu **exact jouw geüploade foto**
`assets/boot-sikkenworks-photo-v243.png`. De transparante buitenranden
zijn verwijderd, de foto is beeldverhoudingsgetrouw met **nearest neighbor**
naar het 240×240-display geschaald. De originele grijstinten blijven
behouden in 57.600 pixels. Het bestaande RGB565-scherm toont deze
**zonder 1-bit-conversie, Bayer-patroon of nieuwe dithering**; tijdens de
fade wordt alleen de helderheid aangepast. De rest van de UI blijft 1-bit.
[Bekijk de echte grijswaardenweergave](assets/boot-sikkenworks-photo-v243-display-240.png).
De filmische tekst, de vaste positie van de overige woorden en de
`Rupture`-naar-`RUPTURE`-flikkering blijven ongewijzigd. De fades zijn
ingekort: logo in/uit 420 ms, tekst in/uit 480 ms. **De bestaande
41-toetsen LED-golfanimatie blijft behouden** en begint zoals voorheen
nadat de splash en de hardware-initialisatie klaar zijn.

De CPU0- en core2-route zijn op de host **sample-exact vergeleken** over
100 geordende effectparen en zes zware ketens van vier effecten:
81.408 stereoframes zonder verschil. Dit bewijst de DSP-logica, maar
niet de snelheid of de geheugenbus op de fysieke tweede core.

In CORE2 EXPERIMENTAL toont `AUDIO DIAGNOSTIC` nu **FX JOBS** (aantal
door core 2 afgeronde opdrachten van 64 samples) en **FB** (terugval naar
core 1). In de USB-/systeempagina staan **FX MAX US** en **FX TIMEOUT**.
Zo wordt de nieuwe effectverwerking niet meer abusievelijk als USB-FIR
gemeld. Het bestaande DMA- en WALL-overzicht blijft gewoon zichtbaar.

**Let op:** geen DMA-LATES-vrijheid, geen 2× performance en geen
foutloze core2-geheugentoegang zijn zonder fysieke FM1-metingen bewezen.
Installeer CORE2 uitsluitend op een test-FM1 met beproefde RP2040-recovery.

## Achtergrond: V2.0.40-intro en eerdere optimalisaties

De eerdere V2.0.40-intro, met identieke tijdlijn maar ouder logo:
Op de oorspronkelijke 240×240-pixelbitmap verschijnt het SikkenWorks-logo
met scherpe 1:1 nearest-neighbor-pixels. Het beeld fadet via 16 niveaus
gecontroleerde pixeldithering in en uit. Vervolgens verschijnen in het midden
de regels:

> And lo, the heavens fell silent, and the earth trembled beneath mankind.
>
> And then the Rupture began.

Alleen het woord `Rupture` trilt en flikkert kort naar `RUPTURE` in een
**vaste, afgegrensde tekstruimte**. De overige tekens staan stil. De complete
tekst blijft even staan, fadet weg en opent dan de gewone RUPTURE-interface.
De intro duurt circa **6 seconden** en wordt vóór het starten van de
audio-engine getoond. Veilige herstel-/opstartmodi behouden hun bestaande
mogelijkheid om de intro over te slaan.

In de historische V2.0.42-STABLE waren audio, FX, CSX/CXS, USB,
presetformaat en MIDI qua algoritme niet gewijzigd. V2.0.43 past
uitsluitend nootafhandeling en foutafhandeling aan; CXS-klankberekening
en FX-sample-algoritmes zijn byte-identiek gebleven. De foto is nearest-neighbor geschaald en behoudt
de oorspronkelijke grijstinten zonder toegevoegde dithering;
vorige DMA-LATES-beveiligingen blijven.
Fysieke LCD-/bootverificatie op jouw FM1 is nog nodig.

## Bestaande V2.0.39 DSP-/DMA-bescherming

**Historische V2.0.39-maatregel:** ALNK0 ontlast bij zware FX en nootwissels. Als drie of meer
effecten actief zijn, kan de optionele scope-/waveform-capture nu 24
audiohalves (~64-70 ms) overslaan na een note-on/off. Bij 20% of minder
resterende DMA-tijd wordt diezelfde optionele scope **altijd** tijdelijk
overgeslagen, ook met alleen MOSFET + REVERB.
Zo verdwijnen overbodige mono-mixing en scope-geheugenkopieën precies op
drukke momenten. Audio-DMA, USB-C-opname, MIDI, CSX, GROWL, SUB en FX
worden **niet** overgeslagen of uitgesteld. De scope kan kort bevriezen.
De bestaande diagnostiek telt de uitgestelde capture in `SCOPE DEFERRED`.
Dit is een veilige broncode-optimalisatie, geen bewezen DMA-LATES=0-resultaat.

**V2.0.38 voegde gerichte DMA-diagnostiek toe.** Onder `AUDIO DIAGNOSTICS`
vind je nu een vierde pagina (`SEL` doorbladeren) met NOTE ON / NOTE OFF,
ON LATES / OFF LATES, en aparte maximale WALL-/RENDER-belasting. De cijfers
gebruiken bestaande ISR-timing, tellen cumulatief sinds inschakelen en
veranderen geen samples. Hiermee kan op de echte FM1 worden vastgesteld of
de note-on of note-off de DMA-deadline veroorzaakt. Samenvallende MIDI- en
panelgebeurtenissen kunnen in één DMA-blok meetellen.

**Zware FX en scherm:** bij drie of meer daadwerkelijk ingeschakelde effecten
blijft de 64ms schermpauze rond noten; tijdens een aangehouden noot worden
alleen cosmetische LCD-animaties bovendien begrensd tot één beeld per 120ms.
Expliciete menu- en knopbediening blijft normaal werken. De FX-keten blijft
op CPU0; het CORE2-experiment gebruikt CPU1 alleen voor USB 48kHz FIR. Dit
is nog geen hardwarebewezen oplossing voor nul DMA LATES.

**V2.0.37 note-on/note-off DMA protection:** met minstens drie ingeschakelde
FX-slots worden uitsluitend overbodige LCD/SPI-schermupdates 64 ms uitgesteld
na iedere FM1-toets- of MIDI-note-edge. De vorige note-burst guard werkte
pas na drie snelle toetsaanslagen; bij een losse aanslag kon het scherm nog
precies met de zwaarste effectberekening om de geheugenbus concurreren.
Deze aanpak verandert GEEN FX-, CSX/CXS-, GROWL-, SUB- of USB-audiosamples,
drukt geen toetsen of MIDI weg en vertraagt geen menu- of foutmeldingen.
De afzonderlijke tweede core blijft voor USB-FIR op 48 kHz beschikbaar.
**De FX-keten draait bewust nog op core 0.** Een gedeelde real-time FX-state
over twee cores is nog niet veilig op een FM1 bewezen. Alleen een echte
hardwaretest kan aantonen dat de DMA LATES voor jouw drie-effectencombinatie
verdwijnen. Vergelijk de LIVE DMA LATES/GUARD voor en na 20x note-on/off.

**Iedere iteratie vanaf V2.0.36: TWEE echte `.fwsc`-bestanden en één
opgeschoonde, volledige project-ZIP.**

| FWSC | Bedoeling |
| --- | --- |
| `RUPTURE-V2.0.40-STABLE.fwsc` | De nieuwe intro, bestaande veilige CPU0-audioroute en DMA-bescherming. |
| `RUPTURE-V2.0.40-CORE2-EXPERIMENTAL.fwsc` | Zelfde intro en bescherming, plus 48 kHz USB-FIR op core 2. **Hoog risico.** |

Beide pakketten behouden de vertrouwde OTA-loader en dezelfde beschermde
bootheader. **WebInstaller.html en index.html bieden nu beide versies in een
keuzemenu aan. STABLE is standaard geselecteerd.** Voor CORE2 EXPERIMENTAL
moet je eerst een extra waarschuwing en een eigen herstelbevestiging accepteren.
Iedere gekozen `.fwsc` wordt apart gecontroleerd op exacte grootte, SHA-256
en apparaatidentiteit voordat er een OTA-actie mogelijk is.

**Waarschuwing:** de dual-core-versie is nog NIET getest op een fysieke FM1.
Core 2 kan op sommige exemplaren gedeeld geheugen verkeerd lezen. Installatie
kan leiden tot vastlopers, gekraak of een recovery-/transporterprocedure.
Flash deze versie alleen op een test-FM1 nadat herstel is bevestigd. Een
foutvrije softwarebuild garandeert geen foutvrije audio. Bij 44.1 kHz werkt
de directe USB-route, dus core 2 wint daar geen DSP. De klankalgoritmen van
CSX, SUB, GROWL, MOSFET, DISTORTION, REVERB en de overige FX zijn ongewijzigd.

De project-ZIP houdt de actuele scripts, complete source, alle hosttests,
eigen assets, huidige documentatie en **precies twee** FWSCs bij elkaar.
Historische FWSCs, verouderde release-packagers, caches en tijdelijke builds
worden niet meer opgenomen. `docs/TEST-V2.0.40.md` en
`docs/RELEASE-LAYOUT.md` beschrijven de procedure.

## Previous release: V2.0.35

**V2.0.35: CPU1 live USB recording safety gate, still LAB ONLY.**

In the opt-in live 48 kHz CPU1 FIR build, CPU0 now watches new DMA LATES,
blocked DMA blocks, USB underruns, USB staging drops, and audio stops. Any
new fault permanently retires CPU1 for that power session. If a CPU1 reply
is not ready while fewer than 64 48 kHz frames remain in the USB PCM ring,
or arrives beyond 1.5 ms, CPU0 refuses to use it and falls back to the
original FIR. A previously retired core can never supply a pending block.
`SYSTEM > USB AUDIO` shows maximum CPU1 wait in microseconds, fault trip
count and fault bitmask when running the live lab image.

**Normal installable firmware remains single-core and exactly sound-identical
to V2.0.32, with only its version string changed.** These protections are
guard rails for a *separate unverified lab binary*, not a promise of zero
DMA LATES, crackle-free audio, or error-free core2 RAM. A USB underrun that
already happened cannot be undone, and the eight live PCM spot checks can
miss a single corrupted sample. Do not install the live lab binary without
a proven physical recovery method. See `docs/TEST-V2.0.35.md`.

## Previous release: V2.0.34

**V2.0.34: actual core2 offload of USB-C 48 kHz FIR, opt-in LAB ONLY.**

This edition includes THREE compiled variants: normal single-core `.fwsc`,
the previously isolated CPU1 shadow-FIR `.bin` and a new HIGH-RISK
`cpu1-live-uac-app.bin` which delegates 64-sample real USB FIR blocks to
CPU1 after a full-parity probation. CPU0 retains every live audio buffer,
all deadlines, exact FIR state and a CPU0 fallback. Native CXS, SUB,
GROWL and all effects are untouched. 142,664 host PCM outputs exactly match
the previous calculation.

The live path uses eight independent spot checks per block after probation.
**It cannot mathematically guarantee detection of every memory misread.**
There is NO physical FM1 validation. The normal installable FWSC remains
one-core, bit-for-bit identical to V2.0.32 except its version label. The
separate experimental live app is NOT flashable through the included updater.

Read `docs/TEST-V2.0.34.md` carefully before hardware qualification.

## Previous release: V2.0.33

**V2.0.33: CPU1 performs real FIR math in a protected shadow test.**

The optional lab firmware now exercises the same 24-tap Q30 coefficients
used for USB-C 48 kHz conversion. CPU0 recomputes every result before accepting
the diagnostic. The probe runs only without voices, sequencer or USB capture;
it **never** writes to audio. An incorrect read, output or deadline retires the
second core. `SYSTEM > AUDIO DIAGNOSTIC` reports verified FIR probes.

The ordinary installable `RUPTURE-V2.0.33.fwsc` remains single-core and
retains identical CXS/FX/USB algorithms. **There is no live DSP speedup yet.**
No physical FM1 has validated dual-core stability. See
`docs/TEST-V2.0.33.md` before any hardware experiment.

## Previous release: V2.0.32

**V2.0.32: controlled second-core hardware qualification.**

The standard firmware remains **single-core and sound-identical** to V2.0.31.
A separate, explicitly enabled `--dual-core-lab` build can start CPU1 and
give it independent SRAM integrity jobs. This is not a 2x DSP release: **no
audible CXS, SUB, GROWL, FX or USB resampler work moves to CPU1 yet**.

The worker uses a dedicated SRAM range, guarded messages, per-word input
checks and core-0 reconstruction of every result. Any bad word, late response
or CPU1 fault retires core 1 until the FM1 loses power. A prior soft reset
while CPU1 was active also prevents restarting it. The audio DMA buffer is
never shared with the worker. This protocol is host-tested, but has **not**
been tested on physical hardware. Do not install a dual-core lab build on a
device without a confirmed restore path.

The experimental build also scans for held **REC / OCT- before CPU1 starts**.
Recovery keys retain priority. Fully power-cycle after leaving a lab image.

See `docs/TEST-V2.0.32.md` for precise risk boundaries and the staged path
to eventually moving USB 48 kHz conversion and independent DSP jobs.

## Previous release: V2.0.31

**V2.0.31: Settings > USB-C AUDIO > 44.1 KHZ / 48.0 KHZ.**

- The selected sample rate is advertised to the USB host. Changing it
  disconnects and reconnects USB audio, so do not change it mid-recording.
- The setting is saved device-wide after a short quiet period, independent
  of project/preset storage. Existing stored projects remain compatible.
- 44.1 kHz is the default/direct path. 48 kHz uses the existing foreground
  FIR, not the audio DMA interrupt. Both paths retain the existing sound.
- The second DSP core is **not activated**: stable shared-memory ownership,
  fault handling and testing on the physical FM1 are required first.
- **CXS, SUB OSC 1/2, GROWL and every FX algorithm are unchanged.** No
  claim of zero DMA LATES until a physical test with GarageBand iOS.

See `docs/TEST-V2.0.31.md` for the USB/FX test sequence.

## Previous release: V2.0.30

**V2.0.30: further realtime headroom without CXS or audible FX changes.**

SHRUB (Xfer Records) is a closed-source FM1 firmware. Its precise DSP
implementation cannot be inspected, so no claim is made to copy its
algorithms or prove its physical timing. RUPTURE instead continues the
same safe embedded-design strategy: keep the audio deadline primary,
avoid redundant sample arithmetic, and defer optional visualization.

On top of V2.0.29's foreground USB 48 kHz FIR, the stereo PCM transfer
remains **packed 16-bit** all the way into the FIR, skipping the previous
Q15-to-int32-to-Q15 roundtrip. The 128-frame transfer buffer saves **512
bytes of SRAM**, bringing `.uac_ram` from 7628 to **7116 bytes**. All FIR
coefficients, phase, output rounding and clipping are preserved exactly.

Under sustained audio/UI pressure or a MIDI backlog, the ISR skips
calculating mono oscilloscope frames that would never be displayed.
The oscilloscope capture decision is unchanged across all tested conditions.
**CXS MAIN, SUB OSC1/2, GROWL, all effect algorithms, output gain and
storage are unchanged in sound and behavior.** The protected bootheader and
OTA loader remain fixed.

The new changes have been tested in software only; **hardware DMA LATES
zero and GarageBand USB capture quality still require an FM1 test**.
For the full hardware matrix, see `docs/TEST-V2.0.30.md`.

## Previous update: V2.0.29

**V2.0.29: USB-C recording / GarageBand iOS DMA deadline investigation.**

On the user's FM1, **CXS MAIN + SUB OSC1 + REVERB** produced DMA LATES
while recording over USB-C into GarageBand on iOS. The selected UAC USB
sample rate is not yet known. SYSTEM > USB AUDIO now displays the **actual
rate negotiated by the host**, either `RATE 44.1K DIRECT` or
`RATE 48K FOREGROUND`.

When the host requests **48 kHz**, the existing stereo 24-tap Q30 USB
sample-rate conversion runs from the firmware's foreground service via a
bounded 1024-frame PCM queue, rather than from the time-critical ALNK0
audio DMA interrupt. The FIR coefficients and arithmetic are unchanged.
At **44.1 kHz**, the direct recording route is unchanged. Buffer overflow
counters explicitly expose loss if the foreground conversion cannot keep up.

**No change to CXS, SUB1, oscillator/filter sound, REVERB, MOSFET,
DISTORTION, SANDPAPER, any FX algorithm, or FX ordering.** Host PCM tests
verify 48 kHz FIR bit identity and retain the sample-exact FX matrix.
Neither the absence of physical DMA LATES nor flawless GarageBand recording
is established by a build/test. Follow `docs/TEST-V2.0.29.md` and keep a
copy of the previous working firmware before updating.

## Previous update: V2.0.28

**V2.0.28: emergency AUDIO STOP investigation release.**

On a real FM1, V2.0.27 was reported to delay the first notes and to trigger
**AUDIO STOP** with MOSFET + DISTORTION + REVERB. This V2.0.28 candidate
**disables the unverified partial SRAM FX migration** and its long-call
compiler option, returning the original FX instruction path to flash/XIP.
It retains the unchanged, sample-exact SANDPAPER/FX code introduced earlier.

The three-effect onset can also trigger several simultaneous FX slot fades.
The old code multiplied and divided two 64-bit values for *each fading slot*
*and each sample*, even though the result provably fits signed 32-bit PCM.
V2.0.28 uses the exactly equivalent 32-bit arithmetic and retains a
compile-time 64-bit reference for direct PCM parity tests. At boot, a held
panel note no longer silently advances its envelope while the DAC output
is deliberately muted for codec/LCD warm-up. All audible CXS oscillator
math, filter tone, reverb topology and distortion character remain intact.

**Hardware verification is mandatory:** Neither the cause of the user's
reported AUDIO STOP nor freedom from DMA LATES is confirmed without A/B
testing on that FM1. See `docs/TEST-V2.0.28.md`. Back up presets first.

## Previous update: V2.0.27

**V2.0.27 (experimental, reverted): move FX renderer to on-chip SRAM.**

The FM1's audio, UI and flash all compete for XIP instruction-cache bandwidth.
This release relocates the existing ordered FX dispatcher, the three REVERB
topologies, SANDPAPER, channel EQ and wet limiter into the board's already
initialized and PC-guarded `.ram_text` segment. There is no algorithm switch,
new coefficient, resampling or dropped effect. **All CXS sources remain byte-
for-byte unchanged.** An explicit linker cap leaves SRAM headroom.

The new layout should reduce FX timing spikes caused by flash-cache eviction.
That is a *hardware hypothesis*, not yet a proven drop in FM1 DMA LATES.
The existing AUDIO DIAGNOSTICS and hard DMA ownership guard remain unchanged.
The released package requires live V2.0.26 vs V2.0.27 diagnostics under the
same SANDPAPER+REVERB+SUB1 presets. See `docs/TEST-V2.0.27.md`.

## Previous update: V2.0.26

**V2.0.26: SANDPAPER + REVERB runtime audit, exact SANDPAPER arithmetic.**

On the FM1, a user reports DMA LATES with SANDPAPER, especially alongside
REVERB. MOSFET + REVERB + SUB1 works on the same unit. This version moves
SANDPAPER's unchanged MIX conversion out of the per-sample loop and uses
an exact bounded Q15 division in both legacy and independent insert paths.
All filter poles, bass reinforcement, wet/dry ramps, clipping and settings
remain mathematically unchanged. No DSP quality reduction is enabled.

The host FX matrix checks **48,064 configurations and 7,532,032 stereo
frames** against a compile-time reference renderer, including 5,860
ordered one-to-four-effect chains, all distinct subsets up to eight effects,
all reverb types, duplicate inserts, and 491,520 frames with real SUB1 CXS
DSP. Both renderers produce the same 64-bit PCM hash.

`docs/TEST-V2.0.26.md` describes coverage and FM1 checks. Host timing is
NOT physical FM1 CPU timing: run AUDIO DIAGNOSTICS with your reported
SANDPAPER+REVERB and MOSFET+REVERB+SUB1 patches before any performance claim.

## Previous update: V2.0.25

**V2.0.25: exact GROWL wavetable mip-decision caching.**

GROWL's existing seven bandlimited ROM tables are selected based on pitch.
During a held note the old hot path repeatedly inspected the same increment
to select the same wavetable level and crossfade ratio on every sample.
RUPTURE now computes that decision once, then reuses it while the exact
increment is unchanged. Glide, pitchbend, LFO, sequencer pitch changes and
fresh DMA blocks recompute immediately. The original per-sample waveform
lookup, interpolation, crossfade and GROWL formant filters are unchanged.

In **1,048,576 host carrier and GROWL formant frames**, all samples matched
the original renderer exactly; 646,656 pitch decisions were reused in a
test mix of sustained notes and rapidly changing pitches. This adds at most
12 bytes of callback-local state and no additional oscillator, delay tank,
global flash record or DSP waveform. The original generated GROWL tables,
CXS MAIN, both SUBs, DISTORTION and all FX are untouched.

`docs/TEST-V2.0.25.md` records validation and physical FM1 follow-up.
Device DMA headroom and the after-reboot preset-open crackle are still
unverified until measured on the hardware.

## Previous update: V2.0.24

**V2.0.24: cheaper DISTORTION control work, unchanged clipping and tone.**

The three per-slot fixed-point DISTORTION setup coefficients (DRIVE gain,
BIAS and TONE smoothing) are now calculated once per unchanged 32-sample
control segment. Every output sample still runs the original tight-bass,
asymmetric saturation, tone filtering, wet/dry and bypass-fade equations.
The coefficient cache is reset when controls are published or when the FX
chain enters the dry compatibility path. Independent and repeated inserts
receive their own exact coefficient values.

**524,288 host stereo samples** matched V2.0.23 bit-for-bit across dynamic
knobs, two or more distortion slots, preset-like drive extremes, bypass
ramps, hidden modulation and DSP reset. The private cache requires at most
50 bytes of callback-local memory, no PCM ring, and no flash storage.
CXS/GROWL/SUB and PJT35 are untouched. A real FM1 must still verify the
DMA margin and the after-restart preset-open crackle.

Test details: `docs/TEST-V2.0.24.md`.

## Previous update: V2.0.23

**V2.0.23: control-rate caching for FX coefficients, preserving PCM.**

RUPTURE now verifies the legacy FX coefficient inputs once per 32-sample
control segment instead of comparing seventeen fields again on every PCM
frame. The cache is refreshed whenever the actual FX control snapshot updates;
it also falls back to the original comparison after an FX reset, after an
early dry exit, and whenever hidden ETCH modulation is active. The DSP itself
still processes all audible samples at 44.1 kHz.

**368,640 host stereo frames** with moving control values, C-FX-style
modulation boundaries, bypass fading, independent FX, hidden legacy effects
and mid-segment resets match the V2.0.22 render exactly. The public FX API,
CXS/GROWL and SUB timbre, PJT35 codec and protected OTA remain intact. A
physical FM1 test remains essential to establish DMA headroom after loading
a saved preset. See `docs/TEST-V2.0.23.md`.

## Previous update: V2.0.22

**V2.0.22: sparse FX slots are scheduled once per audio block.**

If DISTORTION, DELAY or another FX occupies a later insert slot, the PCM
callback no longer traverses empty positions before it. The render callback
prepares an ordered list of the occupied slots once per 128-frame block.
Bypassed slots remain on the list until their fades are resolved; nothing is
reordered. Dense chains keep their original simple scan. The public DSP APIs
and all effect formulas remain compatible.

In host regressions, **983,040 stereo frames** across 120 layouts (all ten
effect types, early/late slots, independent/repeated inserts, bypasses and
removals) matched the previous FX renderer sample for sample. This is a
runtime scheduling improvement, not a change to CXS/GROWL/SUB tone, project
saves or the OTA boot path. Real FM1 DMA margins and the after-restart
PRESET OPEN crackle still require a hardware test.

Test procedure: `docs/TEST-V2.0.22.md`.

## Previous update: V2.0.21

**V2.0.21: less per-sample FX chain scanning, exactly the same sound.**

RUPTURE's effect chain used to check all eight insert slots for every single
PCM frame, even when the rightmost slots were EMPTY. The new block cache
finds the last occupied slot once per 128-frame audio callback. Its ordered
FX processor scans only up to that slot, including any necessary empty
positions in the middle. Enabled effects, FX fades, hidden legacy modulation,
delay/chorus/reverb tanks and stereo mixing all use the original DSP.

294,912 host PCM stereo frames across 24 effect-chain scenarios were compared
sample for sample against the original eight-slot loop; **all frames match**,
including bypass transitions and removing the final effect. This is a
bounded control/dispatch optimization, not a claim that the hardware's
end-to-end DMA burden has been measured. No CXS/GROWL/SUB oscillator, DSP
coefficient, effect formula, preset format or bootloader was retuned.

The third diagnostic page added in V2.0.20 is still available to measure
the actual FM1 PRESET OPEN load and distinguish load mismatch from underrun.
Check `docs/TEST-V2.0.21.md` before claiming the physical crackle is fixed.

## Previous update: V2.0.20

**V2.0.20: PRESET LOAD timing profile + exact parameter audit.**

The physical FM1 still needs to prove whether preset loading is causing
DMA deadlines or merely restoring different CXS/SUB/FX settings. The existing
audio diagnostics now have a **third page** (press SELECT twice) with per-preset
worst-case CPU wall/render percentages, DMA LATES and pre-write GUARD hits
since the last PRESET OPEN. When a preset has been saved in the **same session**,
the diagnostic also compares complete CXS/SUB and FX parameter fingerprints
after reload. `MATCH`, `CORE DIFF`, `FX DIFF`, and `BOTH DIFF` separate the
two failure modes; `NO SAVE REF` is expected after a restart or when opening
an older preset, because the reference is intentionally held only in RAM.
The numeric hashes are debugging fingerprints, **not** audio sample hashes.

No changes were made to CXS, GROWL, SUB, distortion, the FX audio calculations,
or the 256-sample output crossfade. No new CPU-heavy measurements run outside
the existing lightweight ISR counters. Nothing is written to flash by the
diagnostic itself, and no presets/project format need migration. Hardware
confirmation is still required.

## Previous update: V2.0.19

**V2.0.19: PRESET OPEN crackling / DMA-load correction.**

On a physical FM1, configuring GROWL or CXS with a SUB manually is now
reported working, but opening the same sound as a PRESET can crackle.
The live preset change path had two extra jobs compared with manual edits:
it requested a 256-frame output crossfade and marked the old ETCH wavetable
editor dirty, potentially rebuilding three large mip banks while GROWL,
SUB and DISTORTION were already rendering. RUPTURE's CXS/GROWL carrier
does not play those editor tables. Now the optional wavetable rebuild waits
until all held notes and release tails end. The output crossfade keeps its
exact audio samples while using division-free signed Q8 arithmetic.

Nothing is deleted: the four SUB modes, distortion and eight-slot effects,
GROWL expression smoothing, project and preset saves, and the OTA update
loader remain intact. Firmware build and host tests do not measure actual
FM1 DMA margin; verify before claiming the physical crackle is resolved.

## Previous update: V2.0.18

**V2.0.18: all SUB modes use less CPU + smooth GROWL EXPRESSION.**

Actual FM1 testing showed DMA LATES when just one SUB was enabled under
either CXS MAIN or GROWL MAIN. Every active SUB (CXS, CLASSIC, SQUARE,
SAW) now executes one oscillator tick for every four PCM samples, with
fractional interpolation and matched CXS pole smoothing. This cuts
sustained CPU cost without deleting any output frames or changing the
CXS/GROWL MAIN sound engine. OFF is still sample-exact.

The GROWL PRESETS knob now sweeps smoothly between its integer detents:
its 0–100 EXPRESSION target is followed by a Q8 fractional coefficient
interpolator calculated once per DMA block. A single click no longer
triggers an abrupt extra growl transient. LATCH, RETURN, automatic motion,
audible EXPRESSION=0 and PJT35 saved presets remain in place.

Host testing shows close spectral/time correlation with the old SUB modes,
but **real FM1 DMA margin still needs testing**. See
`docs/TEST-V2.0.18.md` for the exact before/after test matrix.

## Previous update: V2.0.17

**V2.0.17: GROWL + SUB + DISTORTION preset save/reboot fix.**

A saved preset could sound different or overload the audio path after reboot
because the common SAVE handler still encoded projects as **PJT34**, silently
dropping the PJT35 eight-slot FX chain and its individual DISTORTION settings.
In addition, the preset-only SAVE route missed the latest SUB BLEND levels.

All save routes (PRESET, PROJECT and autosave) now commit **PJT35** and capture
the active effect order/enables, independent parameters, SUB BLEND, bypass
and the pre-existing GROWL state. Older PJT34 projects still open correctly.
No DSP/sample-loop changes or new audio resources are required.

**If your distortion preset was already saved by V2.0.16**, those FX-chain
bytes were not written to flash. They cannot be recovered from that save.
On V2.0.17, open the sound, add/configure DISTORTION again, SAVE it once,
then restart and reopen it. Test on the actual FM1 for DMA LATE and clipping;
the software reproduction does not prove real-time hardware headroom.

## Previous update: V2.0.16

**V2.0.16: Automatic GROWL motion, inspired by free LFO + smooth chaos.**

GROWL now moves organically even while the PRESETS knob is stationary:

- **AUTO EXPRESSION:** a slow 5.4-second breathing/formant sweep (about
  0.185 Hz), with a second small 1.7-second motion (~0.588 Hz).
- **SMOOTH DRIFT:** a tiny, non-repeating vowel/formant offset changing
  over approximately 8 seconds. This is *timbre drift*, not detuning.
- **Manual priority:** turning PRESETS briefly suppresses automatic motion;
  it fades back over 450ms so your hand always controls the sound first.
- **LATCH/RETURN unchanged:** LATCH holds the user's base EXPRESSION; RETURN
  still glides that base toward 50 after the existing 125ms delay. Automatic
  motion floats gently *around* the current value in either mode.
- **GROWL only:** CXS, SUB oscillators, user LFO routing, pitch, saved
  presets and all other pages continue as before.

The automatic movement is enabled by default at a deliberately subtle fixed
depth. Its two modulation values are computed only in the existing **25ms
foreground service**, not in the audio sample loop. GROWL only adds two
bounded coefficient changes per DMA block. No extra oscillator, voice,
flash writes, DSP buffers or project-format modifications are introduced.

`demos/GROWL-V2.0.16-AUTO-MOTION-D1.wav` is a 12-second D1 demonstration
with the physical EXPRESSION knob untouched. Physical FM1 DMA timing and
acoustic similarity still require testing on real hardware.

## Previous update: V2.0.15

**V2.0.15: GROWL timbre recalibrated to the supplied Seaboard recording.**

- The fourth and third harmonics now lead the bass sound, closer to the
  growling, nasal D1 voice in the 19-second reference. The oscillator remains
  fully synthesized and bandlimited: it does not contain sampled video audio.
- GROWL EXPRESSION covers a more dramatic *closed throat -> open vowel ->
  strained rasp* sweep, with a nonlinear response at the extremes.
- Rapid encoder moves briefly add pressure/rasp; after a move the timbre
  settles naturally. LATCH holds its selected vowel and RETURN still moves
  toward neutral 50. Both settings persist separately from CXS.
- EXPRESSION=0 is still audible, with extra gain compensation only at the
  very darkest end. PLAY LEVEL=0 still intentionally mutes.
- All changes run in coefficient generation per audio block or in offline
  precomputed ROM tables. **No additional per-sample PCM operations**, voices,
  buffers, flash writes or changes to the protected OTA loader.

`demos/` contains an 8-second synthetic D1 expression preview. The actual
FM1 must still be tested for sound and DMA LATE under heavy SUB/EQ/FX load.

## Previous update: V2.0.14

**V2.0.14: GROWL EXPRESSION at 0 stays audible; two performance modes.**

SETTINGS > PRESETS KNOB > GROWL > GROWL PERFORMANCE:

- **EXPRESSION LATCH (default):** The turned value stays where you leave it.
- **EXPRESSION RETURN:** 125ms after the last detent the value glides back
  toward the neutral 50, like BEND. The physical encoder does not spring back.

With EXPRESSION set to 0 the bass oscillator is still audible, even when
BRIGHT and BODY are set to 0. LEVEL=0 remains an intentional volume mute.
Neutral EXPRESSION=50 and positive expression preserve the previous sound.
Both choices have stable, separately saved GROWL/CXS PRESETS assignments.
Return is serviced in the main loop with no additional PCM per-sample cost.
Physical FM1 DMA LATE testing is still required.

## Previous update: V2.0.13

**V2.0.13: Separate PRESETS assignments for CXS and GROWL, with live GROWL
EXPRESSION.** In SETTINGS > PRESETS KNOB, choose **CXS** or **GROWL** first,
then select the knob assignment for that mode. The saved CXS
assignment stays unchanged (default TEMPO / BPM). GROWL defaults to
**EXPRESSION** (English label), a 0–100 live-performance gesture. Each
physical detent moves 5 points, so 10 detents reach an extreme from center.

To use: open SETTINGS > PRESETS KNOB > GROWL > GROWL PERFORMANCE >
EXPRESSION. Then select GROWL on PLAY with ALGORITHM, hold a note, and turn
the physical PRESETS knob back and forth. It continuously changes several
dimensions of the same GROWL oscillator at once:

- **Timbre / slide:** two voice-like formant filters move at different rates.
- **Pressure:** nonlinear overdrive responds increasingly as you turn right.
- **Brightness:** throat/body/upper-band balance shifts independently.
- **Tension:** clipping asymmetry and wavefold evolve into more aggression.

The exact neutral position is **50**, which retains the V2.0.12 sound.
Unlike an actual four-dimensional MPE keybed, one encoder produces a guided,
multi-axis timbral gesture. It does not sense individual finger
pressure, horizontal position or per-note pitch bend. This is intentional:
the prior FM1 DMA overload makes adding continuous pitch multiplication to
the already-heavy audio callback undesirable. The controls remain editable
individually. This gesture is temporary performance state, **not a patch
edit**, so moving the knob never causes flash writes or auto-save.

Both mappings are saved in one compact, CRC-checked global preferences
record and survive project swaps and A/B journal compaction. Older CXS
mappings migrate automatically. SEQ + PRESETS still shifts steps. The
GROWL control target is slewed once per DMA render block; no extra oscillator,
per-sample coefficient conversions, effects, buffers or storage sectors.
The OTA loader and protected flash header are unchanged. Physical FM1 DMA
LATE verification is still required before this firmware is declared stable.

## Prior update: V2.0.12

**V2.0.12: Stronger GROWL, dramatically wider controls.** After testing
V2.0.11, the user reported that the sound was not aggressive enough and the
knobs provided too little variation. The new recording also shows several
PLAY controls at maximum. This release keeps the source wavetable and the
trusted FM1 runtime, but rewrites the GROWL tone circuit:

- **BRIGHT** opens the spectral range far wider, from a thick throat to
  raw upper harmonics. Even at 0 there is an audible bass core.
- **EDGE** goes from clean to a pronounced asymmetric overdrive and
  harmonic wavefold, instead of the previous very mild soft clipping.
- **BODY** raises the low-frequency growl independently of EDGE.
- **LEVEL** now reaches actual silence at zero, unity near 68 and a
  moderate boost at 100.
- **SHAPE EDGE, WEIGHT, BITE and PUNCH** influence formant strength, low
  body, fold aggression and upper-mid presence much more clearly.
- All GROWL coefficient calculations are now cached **once per DMA audio
  block**, instead of repeating their 8-knob conversion every PCM sample.
  The extra distortion uses 32-bit fixed-point arithmetic only, with no
  FFT, float, allocation, or additional delay buffers.

The included 12-second demonstration WAV plays
the same D1 note in six successive configurations: dark/clean,
bright/clean, bright/edge, bright/body, formant/bite and full GROWL.
The original video repeatedly uses D1 (36.7 Hz). The newer FM1 recording
contains higher notes (roughly A2 and F#3), so listen at a similar pitch
when comparing timbre. A note in the reference music can also be layered
or processed: this synthesizer is a synthesis approximation, not a
copyrighted sample.

The former SAW storage bit remains compatible. CXS, both SUB oscillator
types, effects, EQ, reverbs, PRESETS, patch codec and OTA are unchanged.
Software tests do not prove that DMA LATE is zero on a physical FM1:
please test the firmware there before treating this as a stable release.

## Prior update: V2.0.11

**V2.0.11: New GROWL main oscillator from the supplied Seaboard Block clip.**
The 19-second reference video contains a deep bass with a prominent
second, third and fourth harmonic above a weak fundamental (about 36.7 Hz
in the repeated low note). The former main **SAWTOOTH** mode now plays a
new ROM-generated, bandlimited GROWL waveform. Two small integer one-pole
states animate its throaty formant-like spectrum as you change BRIGHT;
EDGE, BODY, LEVEL, WEIGHT, SHAPE EDGE, BITE and PUNCH all remain available.
It is an original synthesis approximation of the recorded sound, not a
copy or playback of the copyrighted recording. The video also has a
recorded performance, processing and mix that cannot be reproduced
perfectly by a single oscillator.

Use ALGORITHM on PLAY or SOUND SHAPE to select **CXS / GROWL**. Existing
patches saved with main SAW automatically use GROWL with their original
parameter values; no storage migration is needed. **SUB SQUARE and SUB
SAW remain unchanged**, as do the CXS carrier, all EQ/reverb algorithms,
FX slots, MIDI, PRESETS and trusted OTA. Physical DMA LATE and listening
verification on a real FM1 are still required.

## Prior update: V2.0.10

**V2.0.10: Channel EQ low-setting and sustained-note DMA hotfix.** On the
FM1's 44.1 kHz audio path, any active four-band Channel EQ could defeat the
eight-slot FX rack's fast bypass. More seriously, the old shortcut checked
the three saved legacy EQ gain fields and the fourth HIGH AIR field, making
almost every EQ edit divert execution through the full FX dispatcher even
when there were *no inserted effects*. Its two routes also disagreed on the
wet peak guard: boosted EQ could hit abrupt clipping on the path without it.

V2.0.10 isolates Channel EQ when the eight insert slots are silent. The same
four original 26-bit RBJ biquads run directly with the original continuous
wet soft-knee and a bounded one-index-per-sample cleanup of previous FX
history. All four saved EQ controls (negative/positive gains, minimum
frequencies and Q extremes) use the same path. With a host benchmark, an
isolated low-frequency EQ went from about 50 to 25 ns per audio sample;
the actual pi32v2 build is also compiled and audited. Eight EQ scenarios
compare 6.4 million reference/saved-control PCM samples, identical to the
V2.0.9 *correct guarded EQ* output. The previously unguarded clipping route
is deliberately corrected. The reverb-to-EQ handoff also stays sample-exact.

**No oscillator, reverb algorithm, EQ coefficient, sample rate, USB-MIDI,
project format, bootloader or PRESETS behavior was removed.** V2.0.9's CXS,
SUB and reverb performance fixes remain. A real FM1 must still verify that
DMA LATE stops increasing with sustained notes and low EQ values.

## Previous release: V2.0.9

**V2.0.9: sustained-note DMA LATE recovery.** Users reported V2.0.8
made late DMA blocks increase during held notes, especially with two SUB
oscillators and reverb. Target pi32v2 disassembly revealed the cause:
the V2.0.8 32-bit CXS exchange introduced conditionals and a much larger
inner loop on a processor with **native 64-bit multiply and division**.
V2.0.9 restores V2.0.7's more compact oscillator-exchange machine code
and the original spring, hall and room feedback engines. The unstable
ROOM loop unrolling and bypass-fade edits have been removed. No reduction
to audio fidelity or effects.

Instead, the active dual-SUB pitch/detune/drift path now uses a single
native paired-register wide multiply, replacing the legacy manually split
32-bit implementation. The isolated pi32v2 instruction footprint falls
from 202 to 106 bytes, and 2,154,880 old/new sample-increment cases
match exactly, including saturation/overflow. Sustained SUB notes, glide,
detune, drift and octave handling remain fully sample-rate, not decimated.
In the ordered FX chain, where input and reverb values are already clipped
to Q15, the reverb wet/dry mixer skips four redundant bounds checks.
Native instruction footprint falls from 66 to 18 bytes; six independent
and legacy stereo reverb tests match V2.0.7 sample-for-sample.
The LED wave, global PRESETS, project storage and trusted OTA are unchanged.
**Physical FM1 sustained-note DMA validation is still required.**

**V2.0.8: PCM-exact realtime optimizations for reverb + dual SUB load.**
The CXS cycle-exchange step now avoids 64-bit audio-interrupt division in
normal playing, including the original unsigned-excitation behavior. No
oscillator exchange, drift, phase or note sample has been approximated or
decimated: the baseline V2.0.7 emulator and the new code agree on 352,000
PCM samples and their complete state across 90,870 actual oscillator wraps.
Reverb's ordered-chain wet/dry interpolation and insert-bypass fades now use
provably safe 32-bit products with **identical truncation**. The ROOM
reverb's four feedback lines are statically unrolled by the FM1 compiler,
without removing any line, reflection or damping stage. Six additional
60,000-frame mixed-FX cases compare all three reverbs against V2.0.7
sample-for-sample. Memory, sample rate, every effect, boot LED animation,
PRESETS and project compatibility are retained. This is a CPU-path fix,
not a reduction in quality. **These arithmetic edits have been reverted in
V2.0.9 because their pi32v2 execution cost regressed on physical hardware.**

**V2.0.7: PRESETS encoder truly functional on RUPTURE.** The V2.0.6
parameter dispatcher was accidentally located after RUPTURE's unconditional
main-loop `continue`. Therefore it was never executed, even though encoder 6
and the global assignment menu were functional. V2.0.7 moves dispatch into
the active 25 ms RUPTURE loop, ahead of the `continue`. Each detent now calls
the assigned parameter editor and displays the existing live-value popup.
Holding SEQ while turning PRESETS rotates the pattern instead. Assignment
picker modal blocks accidental changes, and turning PRESETS wakes the saver.
Native routing tests verify this exact control-flow condition. Real FM1
hardware still needs validation.

**V2.0.6: live PRESETS parameter popup and progressive LED boot wave.**

Each turn of the physical PRESETS knob now displays the globally assigned
parameter's actual value in a 1.55-second foreground popup, even if that
parameter belongs to a different page. Assignments still save independently
of projects. The 27 keyboard and 14 function LEDs now rise individually in
a one-way, fading boot wave over about 1.27 seconds. Real key/button input
cancels the wave immediately. No extra PCM or scan-ISR work is introduced.

**V2.0.5: sequencer readability, global PRESETS knob assignment, note-on DMA optimization.**
The sequencer uses two nonoverlapping rows for STEP, note, HOLD and SLIDE,
including filled notes, long sequences and empty steps. In SETTINGS, select
PRESETS KNOB, choose one of nine logically arranged parameter categories,
and assign any of the supported parameters, including SCALE GUIDE ROOT.
The encoder still controls BPM by default. On selection, a progress overlay
shows the global settings write and confirms its verified completion. This
assignment uses a separate CRC-protected global NOR journal, preserved across
project switches, manual saves, autosaves and journal compaction.
The initial deterministic CXS exchange for the main + SUB oscillators is
stored once in ROM; five note-on oscillator resets no longer execute that
expensive multi-division exchange in the audio ISR. A 262,144-sample host
parity regression confirms byte-exact oscillator state and output.
Hardware confirmation of DMA LATE reduction on the FM1 remains required.

**V2.0.4: real-time quality and performance.** The BEND encoder now
de-zippers its pitch on every audio sample, even with GLIDE OFF. GLIDE offers
LINEAR, EXP, LOG, SOFT, FAST and FIRM curves, persisted per preset in the
existing project bytes. The FX browser groups ambience, modulation, drive
and dynamics while preserving existing effect IDs and saved signal order.
DISTORTION now applies tight low-end filtering and high-gain hard metal
clipping (different from the soft MOSFET sound). ROOM has less boxy damping;
HALL has corrected allpass diffusion and denser, brighter reflections.
SPRING remains unchanged. SUB OSC caches only mathematically unchanged
per-sample values; CXS selects the identical waveform segment in three
comparisons instead of seven and uses exact divide-by-four smoothing for the
fixed SUB EDGE=50 value. PCM sample regression and real FM1 DMA
timing validation are reported separately. No additional PCM buffers.

**V2.0.3: fixed SUB OSC 1 sustain stare.** After the head turns toward the
viewer (frame 12), that precise face stays in place for as long as a note is
held, with a subtle 1-pixel found-footage camera tremble. No subsequent facial
frames cycle during sustain. On release, it returns to the initial still
frame. SUB OSC 2's original behavior, sound, GLIDE, save/autosave, artwork,
boot animation, and OTA remain unchanged. Hardware validation is pending.

**V2.0.2: faster SIGNAL BREACH boot and two new SUB OSC animations.** The opening 22 animation frames now use 35 ms steps instead of 50 ms, while the final **RUPTURE V2** frame remains visible for 1000 ms instead of 780 ms. All 23 original/intermediate 240×240 images remain pixel-exact. SUB OSC 1 and SUB OSC 2 now display their supplied sixteen 224×30 monochrome fear-face frames: the first is still when no note is held, frames 2–12 turn the man toward the viewer on note-on, and frames 13–16 loop the frightened eyes while the synthesizer voice is held. Note-off returns to the still first frame. This is UI-only: no DSP, MIDI, GLIDE, project persistence or OTA code was modified. Original source artwork is preserved at `assets/subosc-v2/`, with a reproducible ROM converter. FM1 hardware validation remains necessary.

**V2.0.1: smoother SIGNAL BREACH loading animation.** The twelve original monochrome 240×240 keyframes remain pixel-exact. Eleven deterministic 1-bit intermediate images now bridge the original frames, giving 23 display frames at a regular 50 ms cadence, followed by a 780 ms final hold. Total nominal animation delay is unchanged at 1880 ms. All images are losslessly compressed and decoded into the existing screen buffer. The independent four GLIDE parameters (TIME, MODE, CURVE, ENV RETRIG) continue to be included for each preset in both manual SAVE and guarded AUTO SAVE, via the same project snapshot. No audio, synth, GLIDE animation, SCALE GUIDE, MIDI, project data format, other interface screen or OTA loader change was made. FM1 hardware validation remains necessary.

**V2.0: SIGNAL BREACH loading animation.** RUPTURE now boots with the supplied twelve 240×240 monochrome SikkenWorks/RUPTURE V2 frames. The 1-bit source pixels remain exact through lossless RLE compression and decode directly into the existing display framebuffer, without creating additional full-screen image buffers. Each frame uses its supplied timing, including an 800 ms final hold (1880 ms total). This replaces only the boot/loading visual; GLIDE motion, GUIDE/STRICT/LOCK, preset SAVE/AUTO SAVE, sound engine, effects, sequencer, MIDI, other animations and trusted OTA recovery are unchanged. The source spritesheet, individual frames, preview and repeatable converter are under `assets/boot-v2-signal-breach/`. Firmware builds and host tests are performed, but FM1 hardware validation remains required.

**V1.99:**

**V1.99: GLIDE animation is note-triggered, and GUIDE shows names rather than numbers.** The GLIDE railway-tunnel scene stays on its motionless first frame whenever no synth voice is held. Playing keys, sequencer notes, arpeggiator notes, and USB/TRS MIDI can drive the 85 ms found-footage camera and walking sprites. On release, both the walk and camera shake reset immediately. STRICT-muted notes do not trigger movement. The SCALE GUIDE third knob now displays OFF, GUIDE, STRICT or LOCK (in that encoder order), while retaining existing persisted values and note filtering. No DSP, saved GLIDE parameters, effects, MIDI parsing or other pages were changed. All automated tests must pass before release; physical FM1 validation remains required.

**V1.98: GLIDE settings are included in SAVE and AUTO SAVE.** TIME MS, MODE, CURVE and ENV RETRIG are stored separately for each preset in the existing project format. Changes are staged in memory immediately so a quick preset switch cannot discard them while a safely deferred autosave is pending. Manual SAVE and AUTO SAVE both commit the exact same project snapshot. Auto SAVE remains deliberately guarded: the project must have been saved or opened before, auto saving must be enabled, and recording, held notes, OTA/recovery and unsafe storage conditions defer writes. Knob changes never trigger flash writes directly. The tunnel animation and audio engine are identical to V1.97. Software validation passes; physical FM1 testing remains necessary.

**V1.97: GLIDE tunnel found-footage animation.** The GLIDE display now shows the approved twelve-frame, pixel-perfect **224 × 30**, monochrome point-of-view walk through a railway tunnel toward the exit light. A gentle, independent 1–2 pixel camera shake ticks approximately every 85 ms, and the walking frames advance every third tick. The existing PORTAMENTO heading moves directly below the scene; all four GLIDE knobs, values, operating modes and footer remain unchanged. Animation data stays in ROM and uses the existing packed display blitter and audio-prioritized LCD scheduling. **No oscillator, GLIDE audio algorithm, effect, MIDI or other-page logic was modified.** The hardware display update is not yet physically verified on an FM1.

**V1.96: STRICT SCALE GUIDE.** The scale mode knob now cycles **OFF -> GUIDE -> STRICT -> LOCK**. STRICT highlights notes in the selected scale exactly like GUIDE, but physical keys outside that scale remain dark and cannot sound or be recorded in live sequencer entry. Outside-scale MIDI note-ons are also blocked (incoming and USB keyboard output), while note-offs remain permitted to prevent stuck notes. GUIDE still only highlights (every key plays), and LOCK still quantizes to scale. Legacy projects saved with numeric LOCK=2 retain that behavior; STRICT uses numeric code 3. The playable-key mask is built once for synth and once for USB MIDI output per audio DMA block only when STRICT is active, and does not add any processing to the sample-by-sample synthesis or FX loop. Software audio/key/LED/MIDI and save/load tests pass; hardware live-stability still needs physical FM1 validation.

**V1.95: live-path CPU optimization without changing the sound.** Each Wah insert now caches the exact attack and resonance control conversions and recomputes them only after a relevant knob edit; its response scaling uses an exact integer identity (5/8) in place of division. The V1.95 PCM regression oracle requires exact V1.94 left/right samples in 20 independent-FX scenarios: each of the 10 types in 1 and 8 slots, with differing controls, bypass and reordering. Additional exact-oracle tests include older project routing, hidden modulation toggled during audio, and three WAHs with live knob edits and slot movement. The old modulation coefficient cache remains warm to preserve the first sample of a newly activated hidden effect. The synth oscillators, patches, GLIDE, EQ, FX functionality, large PCM histories, flash loader and project storage stay intact. Host CPU measurements do **not** establish hardware interrupt headroom; verify on the physical FM1 before a live performance.

**V1.94: independent controls for duplicate FX slots.** Each of the eight slots now stores four separate parameter values. Changing DELAY 2's TIME/FEEDBACK/MIX, WAH 2's SENS/SPEED/RESONANCE/MIX, etc., never changes the displayed or saved settings of DELAY 1 or WAH 1. Moving a slot moves its parameter bank; replacing or removing it resets only that slot. All four knobs and SEL open the selected slot's own values. V1.94 project saves append a strictly tagged, 66-byte parameter extension to PJT35; old PJT35 records still open and retain the old shared-parameter DSP until an FX slot is edited. Newly saved projects must be backed up before downgrading to V1.93.

**Real-time implementation:** MOSFET, Compressor, Tilt, Fuzz, Sandpaper, Distortion and Wah have independent per-slot DSP state, without additional audio delay buffers. Chorus uses independently modulated taps and mix on the existing shared PCM ring. Delay uses independent tap length and mix, with its separate feedback values combined in the one shared echo history. Multiple Reverbs have independent mix but **still share one reverb tank**; the first active reverb sets the tank engine, size and decay. Those limitations cannot be eliminated without adding substantial PCM RAM and CPU. Single-instance FX output is sample-for-sample identical when the existing controls are migrated. **Eight simultaneous active copies do cost more processing than one.** Host CPU tests show this clearly, so the strict requirement “no more audio CPU under every possible chain” and fully independent heavy FX cannot both be met on this fixed hardware. This is an FM1 test candidate, not a verified live release; measure LATE/GUARD and DMA headroom on the physical device before using it on stage.

**V1.93: repeatable FX and a cleaner GLIDE screen.** Removed the two-line GLIDE explanation, leaving a simple **PORTAMENTO** heading. Each of the eight FX slots may now use any of the ten effect types, including repetitions of the same type. The ON/OFF control and bypass fade are independent per slot; reordering and project save/load preserve every slot. Previously saved unique-FX chains retain their exact tested audio output. To avoid allocating new 4–32 KiB audio rings, repeated CHORUS, DELAY and REVERB slots deliberately reuse their single existing buffer and that frame's wet return, so they **are not independent instances with separate parameters or separate echo/reverb histories**. WAH, compressor, and tone processors also advance their shared envelope/filter states once per audio frame, then apply the current result at their requested slot positions. Repeated Distortion calculates each actual clip but only advances its smoothing state once per frame. Every effect retains its existing project-wide parameters. This is a bounded-resource shared-engine arrangement, not eight full independent effects processors. Any additional active processing can increase CPU; physical FM1 DMA margin and live reliability remain unverified. **V1.92 rejects a project containing repeated effect IDs; remove repetitions and back up projects before downgrading.**

**V1.92: DISTORTION and WAH WAH in the eight-slot FX chain.** The FX library now offers **ten different effect types** for the same **eight insert slots**. DISTORTION has K1 DRIVE, K2 BIAS, K3 TONE and K4 MIX. WAH WAH is an envelope-controlled filter with K1 SENS, K2 SPEED, K3 RESONANCE and K4 MIX. Add either effect to an empty/replaced slot via K1 or SEL on empty; SEL on a filled slot opens the parameter page, and FX-list K4 controls its ON/OFF status. ALGORITHM still changes the true PCM processing order. The two additions use previously persisted effect parameter fields, require no new audio ring buffers, and do no DSP work while not in the active slot list. Original 8-slot and GLIDE project layouts remain PJT35. **Important:** an older V1.91 firmware will refuse to load PJT35 projects that contain new effect type IDs 8 or 9. Back up projects before upgrading or downgrading. All eight-effect CPU/DMA stress tests on a physical FM1 still require validation before live use.

**V1.91 FX reliability and real-time optimization:** The FX slot screen now uses **SEL** to open the selected effect's parameter page and **K4** to switch the selected effect ON (turn clockwise) or OFF (turn counterclockwise). SEL on an empty slot opens the FX picker; SELECT chooses a slot, K1 opens the picker, and ALGORITHM changes real processing order. A selected effect is ON immediately when inserted. FX routing edits update the audio-visible nine-byte chain in a short interrupt-guarded transaction. The processing code uses a direct dry signal path for an empty/bypassed chain, a bounded delay/reverb-tail flush, a ROM transfer curve instead of per-sample divisions in MOSFET, and a shared original-order dynamics/tone/distortion path when its conditions permit. These optimizations introduce no new audio voices or heap allocation. **Eight simultaneously active processors can still require more computation than the older four-effect engine. Real-time DMA headroom, glitch resistance and live reliability have not been measured on a physical FM1. Treat V1.91 as a test candidate, not a certified live release.**

**V1.90 GLIDE + editable FX chain:** GLIDE follows SUB OSC 2 in the HOME pages. K1 sets 0–2000 ms, K2 chooses OFF/LEGATO/ALWAYS, K3 LINEAR/EXP and K4 envelope retrigger. In the stopped sequencer, hold SEQ and turn K4 to toggle SLIDE on the chosen step. Each project's saved presets keep their own glide settings.

The FX page now contains **eight initially empty slots**. Turn SELECT to scroll, press SEL on an empty slot to choose an effect, or turn K1 to open the FX library for a selected slot. A newly added effect starts ON. On a filled slot, SEL opens its four-knob parameter page, K4 clockwise enables the slot and K4 counterclockwise bypasses it, while ALGORITHM moves the selected effect up/down the actual audio processing order. EDIT also opens its parameters. HOME returns from parameters to the slot list. Since V1.93, effect types may repeat in other slots; since V1.94, each insert saves independent knob values. Large time effects still share their internal buffer. Save the project to retain the chain. Opening an older project preserves the legacy sound; editing a slot migrates it to the new controls. The project format remains PJT35 with a V1.94 extension; **back up projects before downgrading firmware**. Host tests cannot substitute for physical FM1 testing.

**V1.89 speaker startup pop mitigation:** Based on the complete V1.88.3 firmware, with the three isolated startup changes from the user-supplied V1.88.2 POPFIX source. The codec and silent DMA clock start before the LED bank. Codec control pins PC0/PC2/PC1 are enabled after the silent I2S clock and receive a 2 ms step and 5 ms settling delay. A watchdog-fed 300 ms quiet interval precedes LED startup; the existing PC6 analog gate still requires the LED bank to have been stable for 400 ms and the USB/clock settle conditions. The 20-frame ENV and reverse release from V1.88.3 remain intact. Trusted OTA loader, protected bootheader, Web MIDI connection fix and both FM1 flash-chip IDs are preserved. Physical speaker-pop performance is not yet independently verified on hardware.

**V1.88.3 ENV animation:** The eruption now plays all 20 native 1-bit frames, including the smoke settling into an exposed jagged hole. While a note is held the hole remains, with no frozen smoke plume. On key release the exact animation reverses from its current frame back to the intact wall. The OTA connection fix, trusted loader, protected bootheader and compatibility with both FM1 flash-chip batches are retained. Physical FM1 validation remains pending.

**V1.88.2 BEND crop correction:** All 40 frames use 1:1 source pixel crops centered in the unchanged 224×30 viewport. No stretching. Audio and OTA retained.


**V1.88.1 poster-frame correction:** V1.88 unintentionally shipped 40 newly drawn substitute sprites instead of the approved 5 × 8 illustrations. V1.88.1 takes the exact artwork cells from the approved JPEG reference, crops and downscales them once into FM1-native 224×30 1-bit read-only bitmap assets. The source poster, cropped cells, repeatable converter and byte-for-byte header parity test are included. Existing K1/K2/K3 bend operation, 4-of-5 shake, pre-FX audio processing and OTA/connection fix are unchanged. The poster does not contain lossless individual source sprites, so there is unavoidable detail loss at 30 pixels high.

**V1.88 BEND visuals:** K3 now controls five distinct 8-frame monochrome animations: LINEAR Everest, SOFT Monolith, HARD pirate flagship erupting from a meadow, S-CURVE close-up camera rising from hull to pirate flag, EXP Kraken. Four eruptions shake; the ship close-up stays stable. All frames are packed read-only 1-bit ROM bitmaps with no additional audio/DSP/ISR work. The V1.86 connection/OTA fix and dual JEDEC ID support are retained.

**Installer connection note (V1.86.1, retains the V1.86 connection fix):** Use desktop Chrome/Edge on HTTPS or `http://localhost`. After clicking **CONNECT FM1**, approve Web MIDI / SysEx access. The installer reads the running FM1 identity, including on both supported flash-chip batches; generic Windows MIDI port names are now probed by read-only identity query. If no FM1 appears, confirm that Windows shows a MIDI input and output for the powered-on FM1, close software holding the port, and use manual port selection. The V1.86.1 firmware adds only BEND K3 response shaping; the verified V1.86 Web MIDI connection fix and trusted OTA protocol are retained.
**A monophonic bass synthesizer for the M-VAVE FM1.**  
Cycle Exchange Synthesis (CXS), a selectable bandlimited SAW oscillator, two suboscillators, modulation, sound shaping and effects.

**SikkenWorks. Born out of passion.**

## Getting started

RUPTURE runs on the compatible M-VAVE FM1 hardware. Before installing custom firmware, back up your projects and make sure you have a verified recovery method. Keep the FM1 powered and connected throughout an OTA transfer. Do not interrupt the separate OTA loader while it is writing flash.

On first launch, an on-device welcome overlay walks through backup confirmation, an optional storage format, the CRT filter preference and **CONTINUE TO RUPTURE**. On the first boot of **V1.82**, the firmware checks a new CRC-protected per-version setup marker. An older `welcome_done` flag by itself no longer suppresses the full wizard. The backup question, optional protected formatting, CRT setting and optional 43-screen Quick Tour are shown once. After finishing, a V1.82 marker is saved; subsequent normal restarts skip the full wizard. Existing valid RUPTURE storage is **never reformatted** by that wizard. No format occurs without the explicit confirmation. A change to stock V15 and back to the **exact same** V1.82 build may be indistinguishable from an ordinary restart when V15 preserves all RUPTURE data: automatic detection of that round-trip cannot be guaranteed. Reopen the Quick Tour through GLO. The marker is version-specific; future releases must allocate their own marker if their welcome needs to reappear. Answering NO to the backup question never prevents you from continuing. Formatting never starts automatically: select YES, then deliberately hold SAVE for three seconds to confirm a destructive initialization. If the storage already contains a valid RUPTURE project catalog, the wizard **does not erase existing projects**. Selecting NO skips formatting. CRT defaults to OFF unless a previous global preference exists. The wizard displays a clear, persistent formatting result: **FORMAT SUCCESSFUL**, **STORAGE ALREADY READY**, or **FORMAT FAILED**. Press SEL after reading the confirmation. The completion preference is stored with the existing device-global journal when storage is initialized; when storage is unavailable/foreign and formatting is skipped, completion cannot be saved and the wizard can reappear after reboot.

### Guided Quick Tour (first run or any time)

After the existing first-run setup, RUPTURE asks **VIEW QUICK TUTORIAL?** Choose **YES** with SELECT and confirm using SEL to begin the optional **49-screen** tour, or choose **NO** to start playing immediately. The tutorial displays the actual 1-bit RUPTURE screens with a small text overlay explaining how to reach each page and what each functional K1–K4 knob does; separate views cover all ten FX processors and the new Glide and FX-slot views. The normal audio and projects are not edited by the walkthrough.

**Tutorial controls:** SEL advances, SELECT goes back or forward, ALGORITHM hides/shows the explanation overlay to reveal the entire underlying screen, and HOME exits immediately. The tour also shows the fifth SUB BLEND control and non-knob pages such as project management and diagnostics. It never saves, reformats, changes the sound, or performs an OTA operation. After setup has already been completed, reopen it from **GLO → QUICK TOUR**. Replaying the tour does **not** repeat the format or first-run configuration and does not write the setup journal. The first-run completion flag remains handled by the established storage setup.

The rotary knobs have a smooth, time-based indicator and brief focus markers. UI rendering is separate from the audio processor; if the device reports actual DMA misses, screen frames may be reduced temporarily to protect the audio.

### Speaker-pop / analog-enable sequencing (V1.84.2)

The earlier LED soft start remains in place. V1.84.2 additionally removes the abrupt **all-twelve-button-LEDs-at-full-brightness** transition on **audio faults, SAFE and protected screens**. A pressed button still illuminates immediately. The analog PC6 power gate is now released only after the *observed* button LED masks have stopped changing for at least 400 ms, alongside the existing digital-silence and USB-stability checks. The welcome wizard's low-power LED ramp continues while its UI owns navigation.

These are deliberately limited current/transient mitigations; **PC6 is a binary hardware power-control pin and its own analog bias pop cannot be guaranteed silent by a digital fade or a timer.** There is no added audio work, no new GPIO PWM, and no change to the 10 kHz scan ISR, reverb algorithms, project data or firmware updater. On a physical FM1, compare speaker output at low volume with V1.84.1 and with GLO-held PC6-muted diagnostic mode. If the pop remains with the ordinary LEDs stable, inspect the speaker-enable/power-rail circuit instead of increasing delays or adding unstable PWM.

### Button LED startup pop mitigation (V1.84.1)

At power-on, the twelve function-button LEDs no longer switch directly from dark to a full-current group during Welcome. They enter their existing low-current dim states gradually over about 260 ms. When the normal RUPTURE screen is reached, all V1.84 button, countdown and page indicators behave exactly as before. This does not alter audio, the amp power-control pin, the LED scanning interrupt, storage or OTA. **An audible plop from the external amp's separate PC6 hardware-enable edge is still possible; confirmation on a real FM1 is required.**

### Hardware integration (V1.84)

RUPTURE uses its FM1 function LEDs as direct feedback. The selected hardware section is **bright** (HOME for PLAY / SOUND SHAPE / SUB, ENV, LFO, FX, EDIT for channel EQ, ARP, SEQ or GLO for utility screens). Other function buttons stay discoverable at the existing **high-frequency soft glow**, rather than switching slowly on and off. Pressed buttons immediately take priority. The PLAY LED stays bright while the sequencer is playing, REC while recording is armed/active, and SEQ pulses on the two-bar count-in. Octave and keyboard LEDs continue their established behaviors. During protected update, recovery and first-run screens, background function LEDs stay dim instead of all switching to full current; physical button feedback is unchanged. These are UI-thread policies; **there are no new audio callback operations or LED timer ISR changes**.

When switching pages (including choosing a different FX engine), the acceleration history of K1–K4 resets. The first physical detent on a newly selected page therefore cannot inherit a doubled step from the previous page. Existing fast-turn acceleration on continuous controls and exact single-detent changes on discrete controls are otherwise retained. Knob focus markers and visible values still respond immediately. These behaviors are software-tested but require a physical FM1 check to confirm perceived LED brightness and encoder feel.

### BEND play parameter (FX long press)

**Tap FX** to open the usual effects page, or **hold FX for approximately 700 ms** to enter **BEND**. **K3 CURVE** selects `LINEAR`, `SOFT` (fast at the start), `HARD` (accelerating), `S-CURVE` (gentle at both ends) and `EXP` (strongest bend near the end). All five return automatically when K1 stops and reach the exact K2 target at maximum K1 bend. The BEND screen now includes a mini curve chart and three labeled knobs. Curves are momentary performance state, not saved.  K1 is a momentary guitar/bass-style pitch bend: keep turning K1 to hold/raise the pitch, and when you stop turning, it springs rapidly back to the unbent note. Turn K2 to choose `1/4`, `1/2`, `FULL`, `1 1/2` or `DOUBLE` (up to two semitones). The 240×240 BEND panel shows the live semitone offset, the target, the bend meter and a return indicator. The rising mountain is a prepacked eight-frame 1-bit sprite animation. Pitch bending happens at the oscillator, **before** delay, drive, chorus, reverb and other effects. BEND is a live performance control; it does not change project files or preset data. Test at low volume on the physical FM1; hardware and user-perceived latency are not yet validated.

### Basic navigation

- **PLAY:** Main CXS/GROWL oscillator. The four knobs control PRESSURE/BALANCE/MEMORY/DENSITY in CXS, and BRIGHT/EDGE/BODY/LEVEL in GROWL. GROWL uses the harmonic profile of the supplied Seaboard Block bass video. BRIGHT sweeps a throaty low-mid resonance; EDGE adds drive and BODY supports its bass. Start around BRIGHT 35–60 and EDGE 50–65. The separate SOUND SHAPE controls add weight, vowel-like colour, bite and punch. This is an oscillator-based recreation, not a stored sample.
- **SOUND SHAPE:** WEIGHT/EDGE/BITE/PUNCH. ALGORITHM switches the main oscillator. CXS WEIGHT boosts low-frequency body relative to the highs; CXS PUNCH also responds during sustained notes. Presets saved with nonzero WEIGHT/PUNCH sound different from V1.65.
- **SUB OSC 1 / SUB OSC 2:** Each sub has independent type, octave, detune, drift and blend.
- **FILTER:** Resonant filter controls.
- **ENV:** ADSR envelope controls.
- **LFO:** Synthesis modulation controls.
- **FX / EDIT:** Eight insert slots determine the actual processing order. SELECT scrolls, SEL adds or toggles, K1 opens the FX library, EDIT opens the selected effect's parameters and ALGORITHM reorders. REVERB has three independent algorithms: SPRING, HALL and ROOM. Its K1 selects the engine, K2 controls DECAY, K3 SIZE and K4 MIX. The reverbs share their original bounded native memory rather than allocating additional audio buffers. Channel EQ stays separate from the eight slots. Older projects preserve the old FX order until the chain is edited.
- **SEQUENCER / ARP:** Pattern and arpeggio features. In the stopped sequencer, SELECT chooses a step (STEP 1 is the first step); tap the same musical key to toggle its note, or press SEL to clear the entire selected step, including notes, HOLDs and parameter locks. Linked HOLD events on later steps that refer to these removed triggers are also cleared; other steps are preserved. REC never starts transport on its own. If the sequence is already playing, pressing REC immediately begins overdubbing as before. If playback is stopped, press REC to arm, then PLAY to hear a two-bar (eight quarter-note) metronome count-in. The sequencer grid and SEQ LED pulse with each beat, then playback and recording begin together on step 1. PLAY or REC during count-in cancels it without recording any notes. The keyboard stays playable during the countdown. During active REC the keyboard overdubs rather than erases, and holding a key over several steps records sustain until Note OFF. Hold SEQ and turn PRESETS to shift the complete pattern one step per detent (left/right, wrapping, including HOLD links and locks); release SEQ to keep the current page. Long-press SEQ no longer opens the library; on SEQUENCER, SAVE opens the eight named sequence slots. PRESETS controls its globally assigned parameter (BPM by default), and ALGORITHM controls the independent metronome volume (0–100); ALGORITHM returns to note-sustain editing when recording ends. The screen playhead follows the audio step; LED timing stays independent of LCD drawing. Intermediate-brightness key LEDs use a faster 455 Hz scan (303 Hz with DIM LO) instead of the former 114/76 Hz pattern, reducing visible flicker while preserving the hardware scan driver and leaving the audio engine unchanged.
- **PROJECTS / STORAGE:** Save and manage user projects.
- **SETTINGS:** Master volume, autosave, startup project, storage and CRT/VCR appearance.

**Settings controls:** Open GLO > SETTINGS. On the MASTER VOLUME row, K1 immediately adjusts the percentage; SEL opens the large volume display. AUTO SAVE toggles directly with SEL, without an extra confirmation dialog. GLO > AUDIO DIAGNOSTICS displays the DMA timing and protection counters.

Use **HOME** to cycle the main oscillator pages. On any ordinary screen where SELECT has no dedicated function, turning **SELECT** opens the exact same screen list as on PLAY. Where SELECT already controls a step, FX selection, slot, scroll position or setting, that function remains unchanged. **SEL** confirms selections. Protected update, recovery and welcome screens are excluded from the global navigation shortcut. On pages with K1–K4 rotaries, the last turned knob is highlighted briefly with two small bars beside its circle. FX, FILTER, ENV, LFO, PLAY, SOUND SHAPE, SUB, ARP and SEQUENCER use the same indicator. In Channel EQ, the selected value row is marked instead; the fifth ALGORITHM knob on SUB shows the same focus when adjusting BLEND. User-defined projects use the existing RUPTURE format. The display has dedicated 1-bit animations for PLAY, LFO, ENV and SOUND SHAPE. Animations are rendered in the UI loop, outside the audio callback. On **ENV**, the wall-break dust and debris settle through eight short frames, while the broken wall continues to tremble subtly until Note OFF. On **LFO**, small debris keeps falling into the sinkhole in an eight-frame loop while a note is held; releasing the note closes the scene. PLAY and SOUND SHAPE retain their existing hold loops and shake.

## UC25HQ80 / Puya flash compatibility (introduced in V1.85.2)

The FM1 can contain either Puya **P25Q80** (JEDEC `0x856014`) or UCUN **UC25HQ80** (`0xB36014`) 1 MiB SPI NOR. V1.85.2 explicitly recognizes **both** IDs in its common flash-ID helper. The ID is read once while audio is muted during boot and cached in RAM. Both chips are permitted for the existing explicit project SAVE/FORMAT and firmware OTA staging writes. A **readable unknown** chip ID is refused for writes; a zero/unreadable ID leaves V1.85.1's existing behavior unchanged to avoid unexpectedly disabling older hardware. The project format, flash map, SPI opcodes, timeouts, UI and DSP were not changed.

MIDI SysEx update identity (`cmd 0x11`) now includes the cached JEDEC ID in bytes 24–26 of the 27-byte reply (or all zeroes if unreadable). The null-terminated `FM-1_968` identity is identical for older clients. The *existing, precompiled and audited* app-only update loader has **no separate JEDEC-ID allowlist** and is kept byte-identical to V1.85.1; it would be unsafe to change this trusted binary without source and verification. No bootloader replacement is included.

**Important:** Software checks and FM1 cross-compilation do not prove project persistence or OTA install on the new flash. On an actual UC25HQ80 FM1, back up the full flash, verify JEDEC ID, save a test project, power off completely, start again and confirm the save. Then test the update path using an established recovery method. Older firmware's loader/update restrictions cannot be cured retrospectively by a newer package; return to stock using a known working route if the older firmware refuses the transfer. Physical testing is pending.

## Sequencer LENGTH=4 playback fix (introduced in V1.85.1)

SEQ K1 LENGTH=4 now really plays only steps 1–4, rather than displaying four steps while the audio plays sixteen. Shrinking LENGTH while playback runs immediately confines the next note trigger to the new loop. The hidden notes are preserved and return when you increase LENGTH again. No other firmware functionality has been changed. Physical FM1 testing remains necessary.

## Realtime Reliability (USB MIDI + DMA diagnostics; V1.85)

- The USB-MIDI input queue still retains **all** received packets in FIFO order. Exceptional bursts are serviced in **batches of up to 32 events per 128-frame audio half**, instead of allowing up to 64 events to consume the entire render window. Remaining packets are processed on the next block; existing queue-overflow **panic/Note OFF safety** is unchanged. This may add one block of latency to events at the back of a very large simultaneous burst. Ordinary note traffic is unchanged.
- FM1 panel-key-to-USB-MIDI scanning now skips redundant checks when key state, octave and connection are unchanged. **Pending USB transmit retries are never skipped**: if the USB TX ring was full, Note OFFs and Note ONs keep retrying until sent or disconnected. Randomized host parity against V1.84.2 verifies the exact MIDI packets for 120,000 state transitions, including disconnect/reconnect, octave changes and output backpressure.
- Optional waveform-scope copying is suspended on blocks with pending MIDI input, reducing contention. The audio stream, notes, sequencer, effects and sound are never gated by this choice.
- **GLO → AUDIO DIAGNOSTICS → SEL** now alternates between the original eight DMA diagnostics and a new, read-only detailed page. `CONTROL MAX` includes pre-render snapshot/MIDI/panel processing, `NESTED MAX` covers nested TIMER5 activity, `SLACK LAST` is the most recently observed DMA slack, `MIDI BATCH` and `MIDI QUEUE` capture peak USB-MIDI processing/backlog, `MIDI DEFER` counts blocks that had pending input events, `GUARD` counts pre-publish DMA ownership failures and `POST LATE` counts late completions after publishing. All percentages are relative to the existing measured DMA-half budget. The new pre-render/MIDI maxima and deferred counts reset on audio restart; `LATE`, `GUARD` and `POST LATE` remain cumulative across recovery restarts. `NESTED MAX` is a historical peak.
- The **CXS/SAW oscillator, two SUBs, all three ROOM/HALL/SPRING reverbs, PC6 speaker-pop sequencing, Welcome/format, storage format, device identity, and bootloader/OTA C module are unchanged**. There is no new interrupt, allocation, flash partition or audio effect.

**Physical acceptance required:** Start with volume low, backed-up projects and a proven Pico rescue route. On the FM1, compare `LATE`, `GUARD`, `POST LATE`, `CONTROL MAX`, `MIDI QUEUE` and `SLACK LAST` while playing rapid notes (USB-MIDI and panel) with two SUBs, DRIFT, FX and active screen. A new GUARD or LATE is a real failed deadline; new code does not conceal these counters. The firmware has been cross-built and host-tested, not physically certified, and does not guarantee zero overruns on the FM1.

## Realtime mono headroom

The RUPTURE mono renderer uses a fixed four-to-one gain divider rather than recalculating the upstream 5–8-voice adaptive headroom every sample. The mono engine never uses those extra voices. Master gain still slews smoothly on every audio sample; its unchanged target is resolved once per DMA block. The existing CXS, bandlimited SAW, dual SUBs, all three ROOM/HALL/SPRING algorithms, sustain, MIDI, sequencer, storage and OTA are preserved. Host PCM comparisons prove sample identity for the tested workloads; **they do not guarantee zero DMA LATE/GUARD on physical FM1 hardware**.

Updating from an already completed V1.82 setup keeps its Welcome completion marker in V1.83. No existing user data is erased and no automatic format is performed. The Quick Tour remains available from GLO. A V15 round-trip to the same already-completed release may still be indistinguishable from a reboot if V15 preserves that marker.

## Rapid-note realtime reliability (V1.81)

V1.81 targets rising **DMA LATE** and **GUARD** counts when playing rapid notes. The monophonic synth now avoids needless eight-voice rate and modulation housekeeping on note changes, and performance-gate countdowns no longer copy the full held-note stack on every PCM sample. The existing MIDI, panel-key, sustain, sequencer and ARP behavior is preserved. On a desktop host, long fast-note and sequence-gate PCM comparisons match V1.80 exactly; on-device cycle savings remain to be measured.

The LCD formerly treated every Note ON/OFF edge as an urgent full-screen transfer, even during dense playing. At rapid note rates V1.81 coalesces nonessential LCD frame updates to 75 ms, or 100 ms under recent audio pressure, with a final frame when activity stops. **No key, MIDI event, rendered audio sample or DSP parameter is delayed or dropped.** Explicit screen/menu controls retain priority. Visual feedback can become less frequent during very fast playing to protect the audio bus. The screensaver, UI assets and existing three reverb algorithms remain unchanged.

**Hardware validation is still required.** Restart your FM1, note the baseline DMA LATE, GUARD, DSP MAX and MIN SLACK, then repeat the fast-note stress that raised the counters in V1.80. The important result is that LATE and GUARD **do not increase**. Host PCM identity and cross-compilation cannot guarantee zero underruns on a real FM1. Keep a project backup and a verified Pico rescue option.

## Reverb engines and realtime safety (V1.80)

RUPTURE runs three distinct global **ROOM / HALL / SPRING** algorithms. ROOM uses a four-delay feedback-delay network with damping and decorrelated early arrivals; SPRING uses two cross-coupled delay paths and two dispersive allpass filters, including dispersion **inside** feedback. HALL retains its established diffuse reverb algorithm and V1.79 PCM character. These are algorithmic approximations rather than impulse responses of any specific physical room or spring tank. K1 selects the engine, K2 DECAY, K3 SIZE and K4 MIX. SIZE changes actual line lengths; at MIX 100 the output contains only the reverb return. The engines reuse the existing 4,096-sample reverb buffer plus existing 64-sample diffuser, with only a few bytes of additional control state. A new reverb algorithm may alter a preset's sound; save or back up projects before downgrading.

In the shared effects processor, active phaser, tremolo, auto-wah, tape and decimator Q15 processing now uses mathematically exact bounded 32-bit operations rather than unnecessarily expensive per-sample 64-bit arithmetic. The other synthesizer features, oscillators, SUB drift and HALL are unchanged. On a host, the full-HALL all-effects PCM output remains **byte-identical to V1.79**. The architecture does not reduce polyphony, FX availability or sample rate, but the more realistic ROOM network has extra memory reads/writes and may use more CPU than V1.79.

For diagnostics, go to **GLO → AUDIO DIAGNOSTICS**. `DSP MAX` is a historical maximum; `DMA LATE` and `GUARD` are cumulative. After reboot, verify that both counters stay **unchanged at zero** when actively playing both SUBs with fast DRIFT movements, maximum effects and rapid navigation. Even the user's observed 1/1 indicates a past deadline or guard event. Software crossbuilds cannot prove zero glitches or adequate device CPU margin. Do not consider this a hardware-certified studio/live release until full FM1 stress and listening tests pass. Retain a project backup and verified Pico recovery option.

## Project safety and editing

An asterisk in the screen header or next to the ACTIVE project indicates unsaved changes. The star disappears only when a project is successfully saved or a stored project is deliberately loaded; a failed automatic save does not remove it. Opening a project while unsaved changes exist requires another SEL confirmation; HOME cancels the pending action. The separate STORAGE > PROJECTS route uses the same warning.

A saved project is acknowledged only after the storage operation completes. Loading and saving show a distinct successful or failed result. A single **HOME + SEL** chord can undo the most recent K1–K4 knob gesture on the PLAY, SOUND SHAPE, SUB, TILT, FUZZ and SANDPAPER pages. Hold HOME, then tap SEL promptly. This is RAM-only. Other parameters, sequencer edits and project loads cannot be undone through this shortcut. Create a project backup before changing firmware.

## Firmware installation and recovery

The WebInstaller is available in `index.html` and `WebInstaller.html`. Use a supported desktop browser with Web MIDI SysEx permission, through HTTPS or localhost. The firmware package is `RUPTURE-V1.96.fwsc` and uses identity **FM-1_968**. Its protected bootheader and trusted OTA loader are retained from the prior working release.

The installer can show a confirmed transfer at 100% while the device is rebooting; confirmation that the new firmware is running requires reconnect/identity verification or confirmation on the FM1 itself. Keep the USB connection stable until the process completes. A firmware write is always a recovery-sensitive operation.

RUPTURE retains the native cross-firmware OTA handling and the official V15 return-to-stock verification path. Not every custom firmware is guaranteed compatible. For Windows-based recovery, use the **FM1 Recovery Windows** link on the website. Keep a working hardware rescue route, and back up before changing firmware.

## Source and building

- Native firmware: `vendor/etch-v1.88-source/firmware/fm1-native/`
- UI sprite sources: `assets/`
- Tests: `tests/`
- Native JieLi build (with a compatible toolchain):

```bash
python3 vendor/etch-v1.88-source/firmware/fm1-native/build.py --storage --updater --usb-audio-48k --multi-project
```

This project includes a **software-generated and audited** firmware package. Hardware testing of this release, including maximum-load audio stability, storage formatting and an actual V15 recovery transfer, remains necessary.

## Documentation

The [CHANGELOG](CHANGELOG.md) contains the release history. The [cheat sheet](CHEAT-SHEET.html) explains the synthesizer functions. Technical verification reports are in `docs/`. The README is intentionally a user guide, not a release log.

## Credits and license

**RUPTURE by SikkenWorks, Utrecht, the Netherlands.** Built on the FM1 open-firmware community and the included GPL-licensed source. See `LICENSE` and `licenses/` for the applicable licenses and acknowledgments.


## SYSTEM INFO and encoder controls

Open **GLO → SYSTEM INFO** to see firmware version, firmware identity, USB-link status, a coarse battery-voltage band (not an exact charge percentage), storage readiness, used project slots and DSP/DMA peaks. Select **SEL** for detailed audio diagnostics, or **HOME** to go back. The displayed DSP figure is a recorded peak, not a guaranteed live CPU benchmark.

The four knobs retain one unit of change per deliberate detent. Sustained fast turning may advance continuous values more quickly; discrete choices always follow the individual detents. The last adjusted knob gets a short focus marker. None of this changes the audio engine.

The PLAY, ENV, LFO and SOUND SHAPE graphics run on elapsed time, independently of the display refresh policy. Active sprite/shake changes are displayed on the next available UI pass, while idle redraws are suppressed. If a physical FM1 cannot render a full frame in time, visible frame drops are still possible; the audio path remains prioritized.

## Live sequencer recording

On SEQUENCER, **REC alone never starts the sequencer**. If the sequencer is **already playing**, pressing **REC** begins recording immediately, just as in previous releases. If the sequencer is **stopped**, press **REC** to arm without playing. Press **PLAY** next to start a **two-bar count-in** of eight quarter-note metronome clicks at the current BPM. The step grid and SEQ button LED pulse on each beat. The synth can still be played during the count-in, but those notes are **not recorded**. At the next bar boundary the sequencer and recording begin together on step 1, synchronized to the PCM clock. Press PLAY or REC again during the count-in to cancel it. There is no count-in if playback was already running. Notes played during REC are quantized to the **nearest sequencer step**, accounting for tempo, swing and the actual key-down contact time rather than delayed LCD updates. The first note played on the downbeat just after the two-bar count-in is captured on **step 1**; a press arriving in the last 50 ms before that downbeat is also eligible for step 1 when still held. A Note ON stores the quantized trigger, and keeping the key held automatically extends its recorded note duration over the following steps. Releasing it finishes the sustain. Other notes and parameter locks stay intact. If you turn **PRESETS while holding SEQ**, the entire sequence rotates by one step per encoder detent (counterclockwise = earlier, clockwise = later). Steps wrap at the pattern boundary and retain note lengths, explicit HOLD references and parameter locks. Without holding SEQ, PRESETS controls its globally assigned parameter (BPM by default). Long pressing SEQ does not open the library; SAVE on the sequencer opens the eight sequence slots. The same beat-aligned metronome sounds during the two-bar count-in and subsequent live recording at the project BPM, independently of the monophonic CXS/SAW voice. **PRESETS controls its globally assigned parameter**, including during live recording (BPM by default). **While live recording is armed or active**, turn **ALGORITHM** to adjust the metronome volume from 0 (off) to 100. ALGORITHM keeps its normal functions (including sequencer note sustain) when recording is disarmed. If PRESETS is assigned to BPM, turning it changes the BPM for both the sequencer and its click; ALGORITHM changes only the click loudness. The click does not change synth volume, steal mono synth voices or sound during ordinary playback. The metronome volume is a session setting, not a per-project saved property.

With the transport **stopped**, select a sequencer step and press **SEL** to clear the **entire step**, including every note, local HOLD event and parameter lock. Directed HOLD events on later steps that depend on those deleted triggers are unlinked. Other notes and other steps remain untouched. Step clearing is blocked while playing or recording.

The ENV wall now lets airborne dust and falling fragments settle in eight exact 1-bit PNG frames, while a 1-pixel tremble remains throughout Note HOLD. Note OFF retains the original closing artwork.
