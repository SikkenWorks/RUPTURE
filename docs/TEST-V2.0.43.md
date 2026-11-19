# RUPTURE V2.0.43: hangende noten, intro en FM1-audiotest

## Eerst de Note OFF-regressie controleren (STABLE)

1. Maak een backup. Installeer `RUPTURE-V2.0.43-STABLE.fwsc`.
   Test op laag volume met de audio-uitgang aangesloten.
2. Druk op iedere fysieke toets en laat deze los. Controleer dat elk
   losgelaten contact zijn noot loslaat. Houd één fysieke toets langer
   dan 30 seconden vast: **die mag niet automatisch afgekapt worden**.
3. Sluit USB-MIDI aan. Houd vanuit de controller één noot vast, ga op de
   FM1 naar GROUPS MIDI-learn, en laat de MIDI-noot los. De eerder
   klinkende noot moet nu stoppen. Probeer zowel Note OFF (`0x80`) als
   Note ON met velocity 0 (`0x90`) en controleer ook kanaalwissels.
4. Controleer TRS-MIDI en USB-MIDI met sustainpedaal. Een noot mag met
   CC64 ingedrukt doorklinken, maar moet na de pedaal-release stoppen.
   Stuur eventueel MIDI CC123 (All Notes Off) en CC120 (All Sound Off).
5. Schakel tussen een fysieke toets, de sequencer, een nieuwe preset en
   GROUPS-leermodus. Herhaal met MOSFET, DISTORTION en REVERB samen,
   daarna met SUB1 en USB-C-opname via GarageBand iOS.
6. Voer dezelfde stappen apart uit op `CORE2-EXPERIMENTAL` **alleen als
   de FM1-recoveryprocedure bewezen werkt**. Noteer iedere DMA LATE,
   AUDIO STOP, stuck note en eventuele core2-timeout.

Een note-off kan niet gegarandeerd aankomen als de externe controller
of kabel hem nooit verzendt. De hardware is voor deze versie niet
geverifieerd. Een fysiek hoorbare hang vereist nog steeds een opname,
variantnaam, inputtype en herhaalstappen om de oorzaak vast te pinnen.

## Nieuwe intro controleren

1. Maak een backup en installeer eerst de **STABLE** FWSC.
2. Zet de FM1 uit en opnieuw aan. De door jou aangeleverde PNG met het
   zwart-witte portret en de ruimteachtergrond moet verschijnen, met
   nearest-neighbor-verkleining en **zonder zichtbare ditherpunten**.
   Grijze schaduwen in gezicht, kleding en ruimteachtergrond moeten echte
   grijstinten zijn, geen zwart-wit stippenpatroon. Het logo fadet uit.
3. Controleer dat deze tekst gecentreerd verschijnt, met originele
   hoofd-/kleine letters en alinea-indeling:

   > And lo, the heavens fell silent, and the earth trembled beneath mankind.
   >
   > And then the Rupture began.

4. Alleen het woord `Rupture` mag trillen en flikkeren naar `RUPTURE`.
   `began.` en alle andere woorden blijven op hun pixelpositie.
5. Na een korte pauze verdwijnt de volledige tekst en begint de normale
   RUPTURE-interface. De complete intro duurt nu **5,16 seconden**.
   Logo in/uit 420 ms, tekst in/uit 480 ms.
   Controleer ook dat **de volledige LED-golf over alle 41 klaviertoetsen**
   bij het opstarten nog steeds wordt afgespeeld, direct na de splash.
6. Controleer dat de bestaande recovery-/safe-bootpaden nog starten zonder
   verplicht deze intro te tonen. Dat is bewust een veiligheidsfunctie.

**De intro draait vóór de audio-engine.** Een geslaagde opstarttest bewijst
dus niet dat DMA LATES onder zware FX zijn verdwenen. Test dat apart.

## Bestaande V2.0.39 DSP-/DMA-tests

## Werkelijke V2.0.38-foto's van de FM1 (19 november 2026)

Bij de getoonde FX-keten waren MOSFET en REVERB actief, de andere
zichtbare slots leeg. Op verschillende momenten lieten de read-only
diagnoseschermen zien: WALL MAX 94%, DSP MAX 88%, RENDER 94%, PUBLISH 2%,
MIN SLACK 5%, LATE 9, GUARD 3; NOTE ON 310 / NOTE OFF 311, ON LATES 1,
OFF LATES 0, ON DSP MAX 94%, OFF DSP MAX 89%. Een ander scherm vermeldde
POST LATE 7 en CTRL MAX 1%, NESTED MAX 6%. **Dit zijn cumulatieve pieken
en schermen die niet gelijktijdig zijn gefotografeerd**; ze bewijzen niet
dat uitsluitend deze twee FX de 94% hebben veroorzaakt. Ze bewijzen wel
dat ALNK0 soms vrijwel zijn complete deadline benut en al LATES vertoonde.

Voor beoordeling van V2.0.43: koude boot, kies uitsluitend MOSFET+REVERB,
speel 20 note-on/off-cycli en maak opnieuw foto's van alle vier de
diagnosticapagina's. Vergelijk daarna dezelfde test met een derde effect
en daarna SUB1, CXS/GROWL en USB-C GarageBand. Noteer ook of scope tijdelijk
bevroren lijkt. Een lagere SCOPE-belasting kan RENDER MAX niet op zichzelf
verlagen omdat het optionele scopewerk in de PUBLISH-fase zat.

**Bestaand sinds V2.0.39:** de niet-audible waveform/oscilloscope-capture wordt
maximaal 24 DMA-blokken (~64-70 ms) gepauzeerd na iedere note-on/off met
3+ actieve FX, en bij ieder effectaantal zodra 20% of minder DMA-tijd resteert in het
vorige blok. Controleer de bestaande `SCOPE DEFERRED`-teller; deze kan nu
stijgen zonder een DMA LATE. De scope mag kort bevriezen, maar er mogen
geen audiomonsters, MIDI, USB-frames of FX-tails verdwijnen.

**Bestaande V2.0.38-meting:** open `AUDIO DIAGNOSTICS`, druk drie keer op SEL voor
`READ ONLY / NOTE EDGES`. Noteer NOTE ON, NOTE OFF, ON LATES, OFF LATES,
ON WALL MAX, OFF WALL MAX, ON DSP MAX en OFF DSP MAX. Elke waarde is
cumulatief sinds de FM1 opstart. Voor pieken reset je door uit/aan te zetten;
maak eerst een backup. De twee LATES-tellers tellen alleen het DMA-blok met
een herkende note-edge, niet alle opeenvolgende DSP-blokken of iedere
latere mogelijke USB-uitval. Interpretatie is daarom indicatief.

Vergelijk na 20 losse aanslag/loslaat-cycli de note-tellers met de totale
`DMA LATES`/`GUARD`-tellers op de eerste diagnosticapagina. Reproduceer ook
met een noot 3 seconden aangehouden en drie zware FX, zodat je de 120 ms
cosmetische LCD-begrenzing kunt controleren zonder toonverandering.

## First: reproduce the three-effect note-on/note-off issue

1. Install STABLE, back up presets, then set MOSFET + DISTORTION + REVERB
   (or the three effects that actually trigger the problem on your unit).
2. Reset/read `AUDIO DIAGNOSTICS > DMA LATES / GUARD / WALL`.
3. Play one note, release it, wait 0.2 s and repeat 20 times. Repeat with
   both SUB oscillators, with saved/reloaded presets and during USB recording.
4. Note the increase in DMA LATES and GUARD for presses versus releases.
   Verify CXS, GROWL and reverb tails still have their original sound.
5. Repeat with two effects, then with four and eight, and with rapid MIDI
   note-on/off. Confirm the screen catches up after the 64 ms quiet window.
6. Track `SCOPE DEFERRED` as well as LATES/GUARD. The new guard should have
   no effect with 0-2 FX and should stop nonessential scope capture with 3+.

The improvement deliberately reduces LCD/SPI memory-bus contention, *not*
the count or quality of DSP effects. This is not a measured guarantee of
zero DMA LATES until actual FM1 testing verifies it.

**Two real installable firmware files are included.**

The WebInstaller (`WebInstaller.html` and the matching offline `index.html`)
now has a firmware selection menu. It starts on **STABLE**, and verifies the
exact selected image by SHA-256 before allowing CONNECT/INSTALL. Selecting
**CORE2 EXPERIMENTAL** exposes a risk warning, requires a separate checked
confirmation of tested RP2040 recovery, and prompts again before writing.

- `RUPTURE-V2.0.43-STABLE.fwsc`: CPU0 processes CXS/GROWL/SUB and all FX.
  Safe reference with new PNG intro; no DSP offload.
- `RUPTURE-V2.0.43-CORE2-EXPERIMENTAL.fwsc`: core 1 (CPU0) synthesizes
  CXS/GROWL/SUB. Core 2 (CPU1) runs **all eight ordered FX slots**, including
  MOSFET, SANDPAPER, DISTORTION and REVERB, in 64-sample chunks. The first
  chunk overlaps CPU0's next 64 synth samples. 48 kHz USB FIR stays on CPU0
  to avoid two competing core2 jobs. FX handoff starts only after 128
  integrity probes and at least one qualified FIR probe have succeeded.

## Risk before flashing

The experimental FWSC is **not hardware-qualified**. Some FM1 batches have
shown CPU1 shared-memory read issues. The new FX handoff publishes completed
samples and times out after approximately 1.08 ms per chunk. CPU0 **halts**
core2 before continuing unfinished FX work, but a failure halfway through
one DSP sample could already have modified effect state. This is NOT the old
USB-FIR full-output parity probation and it cannot guarantee glitch-free
sound or prevent a brick. An already missed DMA deadline cannot be repaired.
Never flash the experimental build without
working recovery hardware and a known-good transporter/restore procedure.

## Test sequence on a spare FM1

1. Back up projects/presets. Verify your FM1 Recovery software, RP2040
   transporter and stable restore route work before testing.
2. Install the **STABLE** FWSC first. Confirm cold boot, preset reload,
   CXS, GROWL, SUB1/SUB2, all FX and 44.1/48 kHz USB audio.
3. Install the explicitly named **CORE2-EXPERIMENTAL** FWSC on the test
   device. Cold boot, open `SYSTEM > AUDIO DIAGNOSTIC`; give core2 several
   seconds for integrity/FIR qualification. Then play sustained notes with
   at least one active FX so the new core2 FX lane engages.
4. On `SETTINGS > USB-C AUDIO`, select **48.0 kHz** and reconnect USB.
   Record via GarageBand iOS for several minutes. Start without FX, then
   CXS + SUB1 + REVERB, then MOSFET + DISTORTION + REVERB, then GROWL
   + SUB1 + SANDPAPER and other effect combinations.
5. Note DMA LATES, DMA guard, USB UND/OVER/MISS/STALL en de nieuwe
   diagnostiek: `FX JOBS` (per 64 effectmonsters, twee per volledig
   audioblok), `FB` (CPU0-fallback), `FX MAX US` en `FX TIMEOUT`. De
   voormalige USB `LIVE FIR`-teller is in dit experiment niet van toepassing.
   On any audio
   stop, crackle or abnormal value, stop the experiment and use STABLE.
6. Repeat at 44.1 kHz as a direct-path control. Compare captured PCM with
   a stable reference, looking for missing frames, clipping and clicks.
7. Fully power-cycle after any core2 retirement. Reinstall STABLE for
   normal use until physical test results validate the experiment.

Software validation only proves that the packages parse, decrypt to their
respective exact app binaries, share trusted loader/bootheader, and pass
host audio parity tests (100 ordered FX pairs + 6 heavy 4-FX chains,
81,408 stereo frames exactly matching CPU0). It does **not** prove no DMA
LATES or safe shared-memory access on real FM1 silicon.
