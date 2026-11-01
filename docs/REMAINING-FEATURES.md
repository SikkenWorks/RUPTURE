# RUPTURE V1.5: native feature audit and remaining gates

## Implemented in V1.2

- TRS MIDI IN (UART1/PH8 RX DMA, 31250 baud) now feeds the same audio MIDI input queue as USB. Portable parser and FM1 target compilation verified, physical TRS input not tested.
- CXS Pressure/Balance/Memory/Density are saved with RUPTURE-tagged native patch records and are restored when a project/preset is loaded. No change to the 48-byte patch record size.
- Native 240x240 strict 1-bit UI, black/white dithering and screenshots exported from its C renderer.
- Six-note mono memory, last-note default, baseline octave 0 (= original low F1 register), SEQ editing and 8 pattern snapshots within each physical project.
- Existing Filter, Drive, Reverb, Chorus, Delay, EQ, LFO, Arp, Guide and Keylock integrated, with the known DSP limitations below.
- GLO long-press PANIC, GLO Master Volume 0-200% (runtime), onboard HELP.

## Still incomplete or requiring independent hardware verification

1. **12 flash project slots (V1.5 host-tested, hardware open):** Twelve selectable catalog addresses are implemented inside the existing two 80 KiB owned project banks. V11/V12 four-slot records remain readable and are migrated on first successful save. Capacity is bounded: large projects can return `STORAGE FULL`, so twelve arbitrary max-size records are not guaranteed. The new V13 catalog is not readable by older firmware. Device flash/power-loss tests, upgrade and downgrade rehearsal are still required.
2. **Note memory additional controls:** V1.6 implements LAST/HIGH/LOW, LEGATO, GLIDE 0–400 ms and approximate BEND 0–12 natively; controls are runtime-only, physical behavior and full MIDI bend calibration remain unverified.
3. **Charging indicator:** the lightning bolt shows an active USB device connection, not verified battery charge current. There is no grounded charge-current sensor measurement in the native code.
4. **Early black screen / loader LCD progress:** mask-ROM/power-on interval is outside the application. The trusted OTA-loader is kept byte-identical; continuous WebInstaller progress exists, but it cannot force a custom FM1 LCD progressbar during separate loader execution.
5. **Reverb types:** Spring/Hall/Room are different settings of one DSP architecture, not three independent room/spring algorithms.
6. **Channel EQ:** 3 gain bands plus high-cut, not a fully parametric Logic Pro-style EQ with movable frequencies/Q.
7. **MOSFET overdrive:** DSP saturation modeled in software, not a verified component-level MOSFET circuit model.
8. **TRS MIDI OUT:** not implemented. Native UART1 RX is MIDI IN only; USB MIDI IN/OUT retains existing ETCH bridge.
9. **Global Master Volume / per-effect storage:** V1.13 implements a device-global flash journal and hosttests. Actual save/reboot validation on real FM1 and comprehensive independent per-effect patch persistence remain open.
10. **Hardware:** input scanning, TRS electrical wiring/type, 6-note retrigger and glide, ADC battery calibration, USB charging and cross-firmware OTA recovery must be validated with a real FM1.

No passing host build or CRC check is proof of a successful flash or recovery. Prefer confirmed recovery and a backup of current user data.

## V1.5 progress

- Full 12-slot native catalog, 16-bit occupancy and legacy 4-slot read/migration implemented and tested with a bounded 1-MiB simulated NOR flash. No flash sector ownership expansion; preserve previous committed bank on write failure.

## V1.4 progress

- Per-preset CXS SOUND SHAPE memory implemented with an explicit tag inside existing 33-byte PJT32 DSP records; no extra flash sectors. Host roundtrip tests pass. Hardware loading/flash persistence still requires a physical test.

## V1.11
- Dual SUB1/SUB2 settings are now per-preset persistent in PJT32 with a separate tag and strict codec validation; real FM1 save/reboot/OTA tests still needed.
- Note Memory and Master Volume are still runtime-only; they are not claimed as saved here.

## V1.13 verified in host environment
- Device-global Master Volume 0–200 stored in a CRC-checked append-only log within the already-owned project A/B bank. No flash sector ownership change. Project swaps carry the latest volume forward; full-log compaction uses a verified A/B swap.
- Native PJ52 296-byte manifest programming now respects 256-byte maximum page size and page boundaries. Host NOR mock explicitly rejects invalid program lengths and simulates torn writes.
- On actual hardware, verify cold boot, 12-slot saves, fallback after power interruption, and independent global volume. USB/TRS/OTA hardware confirmation remains outstanding.

### V1.17 FX data

- New FX parameter/bypass memory is now **project-scoped PJT33**, with 21 bytes per saved project (not per preset). Previous PJT32 projects remain readable, but older firmware cannot read PJT33. Hardware validation is still outstanding.
