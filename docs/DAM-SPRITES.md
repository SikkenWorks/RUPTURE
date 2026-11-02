# RUPTURE waveform bar: DAM SPRITES

These 12 original user-supplied 1-bit PNGs are displayed on the PLAY waveform bar, exactly 224x30 at screen x=8,y=50. Files are preserved unchanged under `assets/dam-frames/`. `rupture_dam_sprites.h` embeds their **pixel bits**, not PNG compression or a redraw: 12 x 840 = 10080 bytes static read-only data.

## Timeline

- Idle: frame 01 (closed).
- When any note is gated: frames 02-05 advance every 65ms.
- While keys remain held: frames 06-08 repeat. MIDI/ARP/sequence gates count too.
- After the last gated note: frames 09-12 (or later closing frames if the crack barely opened), then idle. Closing stages advance every 75ms. A new note during closing restarts opening.
- Leak frame step durations by displayed OCT offset -2/-1/0/+1/+2/+3/+4/+5: **270/220/165/115/75/55/45/35ms**.
- The 25ms input tick and LCD refresh yield display timing granularity. Under heavy audio load, the display may be delayed; audio is prioritized.
- No per-pixel geometry generation or additional framebuffers. Copies the packed bytes into the existing native 1-bit canvas. Rendering and sprite timing happen only on the main/UI thread. Existing band hashing suppresses unchanged LCD blocks.
- No project/patch storage version changes; boot, OTA and audio code unchanged beyond a read-only view of voice gates from the UI thread.

## Source PNG SHA256

- `01-closed-idle.png`: `b87c10e3837b15c9c16d6d05f3d95d9fd1d62e259a553f47be6480902ed9f554`
- `02-hairline.png`: `603c5bc40266d135452cd7d6f0be44d9a6c366d1969327caaba3bd11d6399ab5`
- `03-crack-growing.png`: `6b019eb581b061af878aa3fa70153ffe23ed89b39c857aa02716223ac04e9356`
- `04-dam-splitting.png`: `29008d39cfb6a9a6e9589bbcf4e8c57b191e2137b5c09af8f34bfcb4857c19f1`
- `05-first-leak.png`: `bbde9e879a43b6224762e64658d3abff231c872d5253f28f979774083905e7c9`
- `06-leak-loop-A.png`: `077e61310095ddc00701b807b1f622fc1a7926ae95509e70fe0abbe3ac6a5020`
- `07-leak-loop-B.png`: `95a40b09d538a4d4df076b10578fddc32d719c4c7ba86e3e55285441b641da94`
- `08-leak-loop-C.png`: `1cbc57d19bc78d55fdf00dbba19a4e4aa3d165797e7a8df14c8635eec1563902`
- `09-leak-stopping.png`: `8f90c8f34f1b65e554b0513acac2df1f3a1ddd681dd99b5ca0c31c3ddfe53551`
- `10-crack-closing.png`: `29008d39cfb6a9a6e9589bbcf4e8c57b191e2137b5c09af8f34bfcb4857c19f1`
- `11-hairline-closing.png`: `603c5bc40266d135452cd7d6f0be44d9a6c366d1969327caaba3bd11d6399ab5`
- `12-closed-restored.png`: `b87c10e3837b15c9c16d6d05f3d95d9fd1d62e259a553f47be6480902ed9f554`

## Validation

Use `python tests/test_dam_sprites.py` to compare every frame with `rupture_host_export` output at 1:1 pixels, including the exact unchanged header/footer outside the waveform bar. Use `gcc -std=c11 -O2 tests/test_dam_anim.c -o /tmp/ru_dam_test && /tmp/ru_dam_test` for state transitions and speed. Native build target: `vendor/etch-v1.88-source/firmware/fm1-native/build.py --storage --updater --usb-audio-48k`.
