# RUPTURE V1.52 — CRT lens correction and native UI audit

- **Native FM1 pi32v2 build:** PASS, 254,112-byte app, 327,452-byte reported headroom.
- **Original CRT defect:** V1.51 deleted alternate pixels from text and only shifted rows horizontally.
- **V1.52 implementation:** persistent CRT ON/OFF unchanged. Raw 1-bit UI buffer remains byte-identical with the CRT ON or OFF (except the on-screen settings label); a 2D, Q8 barrel lens and very light neutral phosphor scanlines are applied only while filling the existing RGB565 LCD staging stripes. No extra full-screen framebuffer or DSP operations. Bilinear nearest-source-pixel interpolation prevents thin glyphs from vanishing during barrel mapping. QR page remains fully unfiltered.
- **CRT host test:** PASS, 7 styled pages and 1 unfiltered QR page; RAW byte parity and neutral monochrome grayscale confirmed.
- **Screensaver forward/hold/reverse/loop test:** PASS, unchanged from V1.45.
- **OTA FWSC verifier:** PASS, exact same 16KiB protected bootheader and OTA loader as V1.51.
- **Identity:** FM-1_968 preserved. Manufacturer and Product unchanged: SIKKENWORKS / RUPTURE.
- **WebInstaller/Website consistency tests:** PASS, embedded firmware byte-identical to .fwsc; browser JavaScript syntax checked.
- **Physical hardware:** NOT TESTED. Actual CRT perceptual quality, screen-refresh CPU cost, USB OTA transition and audio under load remain to be verified on an FM1. CRT ON costs additional UI work but does not add audio-DSP work.
