# RUPTURE V1.65 — Physical FM1 Acceptance Matrix

**All checks below are PENDING.** This document is a checklist for testing on an actual M-VAVE FM1, not a record of a completed hardware test.

## Before beginning

- [ ] Confirm the exact hardware is an FM1 supported by the existing installer and identify the currently installed firmware.
- [ ] Back up projects, sequences and the current firmware where possible. Verify you can restore the backup before destructive operations.
- [ ] Confirm a working USB/recovery method. A software OTA simulator is **not** a hardware rescue path.
- [ ] Use a stable power source and USB cable. **Never deliberately interrupt power while the OTA loader writes**.

## Core instrument acceptance

| Area | Test on physical FM1 | Pass criterion | Result |
|---|---|---|---|
| Boot and version | Cold boot and open GLO → SYSTEM INFO | Shows V1.65, no boot hang, correct `FM-1_968` | PENDING |
| First-run wizard | On a safe, backed-up storage state, test NO, NO :(, CRT OFF and skip format | All paths allow CONTINUE; no unwanted erase | PENDING |
| Format result | Only with verified backup, select format and intentionally hold SAVE 3 s | FORMATTING, then correct SUCCESS/FAILED result and SCL acknowledgement | PENDING |
| CXS | Sustain bass notes and sweep PRESSURE 0/10/25/50/75/100 | Continuous timbre response; no stuck notes or DSP stalls | PENDING |
| Oscillators/subs | Switch CXS/SAW, both subs and octaves, use highest/lowest useful notes | Stable volume and tuning without unsafe clicks | PENDING |
| FX/DMA stress | 30 min sustained play with heavy supported FX, EQ and USB audio where available | No audible dropouts; diagnostics show no worsening DMA failure pattern | PENDING |
| Animations | Hold/release notes on PLAY, ENV, LFO and SOUND SHAPE | All loop or shake as designed and return to idle, without audio dropouts | PENDING |
| LCD load response | Test intensive playing while navigating and showing animations | Controls remain usable; dropping animation frames does not mute audio | PENDING |
| Knob UX | Slowly/quickly turn K1–K4, try SCL, HOME+SCL undo | Accurate increments, no label overlap, undo only where documented | PENDING |
| Project persistence | Save, power-cycle, load and compare a backed-up 12-slot project/sequence set | Same parameters, no unexpected overwrite; unsaved `*` works | PENDING |
| Write-failure UX | Use only controlled failure scenarios, not intentional mid-flash power loss | Honest error; user work not falsely marked saved | PENDING |
| MIDI | USB and TRS note, clock and transport tests | No hung notes; device reconnect behaves normally | PENDING |
| Battery/USB info | Compare connected/disconnected and charging states | Accurate coarse state; no invented battery percentage | PENDING |
| CRT | Toggle ON/OFF, revisit after restart | OFF remains the lightweight default where no preference is stored | PENDING |
| Long run | 60 min mixed play/sequence/idle/screensaver | No watchdog reset, unexplained silence or unrecoverable UI | PENDING |

## OTA and recovery acceptance (highest risk)

| Test | Safe prerequisites | Pass criterion | Result |
|---|---|---|---|
| Current OTA update | Full backup and a **known working** rescue method | Loader CHECK→WRITE finishes, FM1 boots V1.65, browser shows clear completion | PENDING |
| Reconnect after reboot | FM1 OTA complete | `RUPTURE` MIDI port returns or installer reports the unverified reconnect truthfully | PENDING |
| Compatible custom firmware | Verify exact destination package and its actual cross-firmware support first | A **planned** switch completes and device boots correctly, project backup available | PENDING |
| Return to official V15 | Exact official V15 and correct FM1 bootheader, verified recovery equipment and backups | Vendor V15 boots; no protected-area mismatch; FM1 remains usable | PENDING |
| Recovery Windows | Approved recovery tool and recovery procedure, only when required | Device can be restored without relying on unverifiable assumptions | PENDING |

**Do not deliberately brick an FM1 or disconnect power during flashing just to satisfy a test.** If an OTA destination's bootheader or identity is not compatible, mark the transition BLOCKED instead of forcing the flash.

## Test record

Date / FM1 unit / board revision / starting firmware / USB host OS & browser / used `.fwsc` SHA-256 / audio device and sample rate / backup location / test operator / observed diagnostic counters / details of failures / pass signatures:

_Pending physical test entry._

**Promotion criterion:** no critical audio, project-data, boot or recovery defects; all required firmware paths actually demonstrated on compatible hardware; test evidence captured and reviewed. Until then, V1.65 remains SOFTWARE RC.
