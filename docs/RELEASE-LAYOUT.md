# RUPTURE release convention (V2.0.36 onward)

Each numbered RUPTURE iteration ships **exactly two** `.fwsc` installers
and one clean full-source project ZIP:

1. `RUPTURE-VX.Y.Z-STABLE.fwsc` retains CPU0-only audio.
2. `RUPTURE-VX.Y.Z-CORE2-EXPERIMENTAL.fwsc` runs the specifically
   isolated real CPU1 audio workload. It must be marked unqualified until
   physically tested on FM1 silicon.
3. `RUPTURE-FM1-VX.Y.Z-CLEAN-FULL-PROJECT.zip` contains both FWSCs,
   complete source, UI assets, host test suite, current release scripts,
   reference loader/firmware files and manuals.

Never copy old root FWSC files or previous one-shot release packagers into
the clean ZIP. Exclude `vendor/*/build/`, `.git`, Python caches and object/
ELF/PGM outputs. Preserve the exact source of CXS, SUB, GROWL and FX,
and run the production PCM parity and full FX/USB regression tests before
publishing. Keep historical release notes in the main changelog.

Packaging assertions check the presence of the two current FWSCs, their
distinct app payloads, common trusted OTA and protected bootheader, and the
absence of old firmware files and packagers. Both Web MIDI installers embed
both matching release FWSCs, default to STABLE, and block CORE2 installation
until the user explicitly accepts the extra hardware/recovery warning. No
auto-flashing or device changes occur when the ZIP is built.
