# RUPTURE V1.56: LFO / ENV native panel art

- `lfo/`: the FIRST author-approved artwork: overhead street and sinkhole.
- `env/`: the SECOND author-approved artwork: wall cracking and collapse.
- Each folder has 20 indexed 224×30 native **1-bit** PNGs, a spritesheet, magnified contact-sheet and a GIF preview.
- `SOURCE-CONTACT-SHEET.png` archives the 1448×1086 source rendering used for the offline conversion. The native PNGs are a one-time, pixel-quantized downscale from the larger contact sheet; they cannot be literally lossless relative to the higher-resolution contact-sheet image. In firmware, the native PNG bitmaps are packed *losslessly* with no runtime resampling.
- All sprites are embedded as static const read-only bytes in `rupture_panel_sprites.h`. No audio callbacks, sample buffers, extra framebuffers, dynamic allocations or runtime procedural artwork.
- ENV displays 1–13 during note-on / hold and 14–20 for release; LFO uses 1–13 during note-on / hold, then reverses *those same frames* from 13 to 1 on release. All 20 exported LFO frames remain in the project for future editing and art review.
- The existing PLAY DAM V3 uses its separate 20 sprites; these sets do not replace or alter PLAY.
