#!/usr/bin/env python3
"""Render every boot frame using the *actual* FM1 UI C renderer."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
from PIL import Image, ImageSequence

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'vendor/etch-v1.88-source/firmware/fm1-native'
TARGET = ROOT / 'ui/loading-frames/RUPTURE-V2.0.58-BOOT-INTRO-PREVIEW.gif'
FRAMES = 86  # RU_INTRO_FRAMES; 60ms each, total 5.16s
WELCOME_MS = 1100
PLAY_MS = 800

with TemporaryDirectory(prefix='rupture_v258_boot_') as work:
    folder = Path(work)
    binary = folder / 'native_export'
    subprocess.run(['cc', '-O2', '-Wno-unused-function', '-I', str(SRC),
                    str(SRC / 'rupture_host_export.c'), '-o', str(binary)], check=True)
    frames = []
    for index in range(1, FRAMES + 1):
        frame_path = folder / f'{index:03}.ppm'
        subprocess.run([str(binary), '24', str(frame_path), str(index)],
                       check=True, stdout=subprocess.DEVNULL)
        # Preserve full RGB565 grayscale shades, not a 1-bit fallback.
        # Quantize GIF colors WITHOUT dithering; the photo is already pixel art.
        frames.append(Image.open(frame_path).convert('RGB').quantize(
            colors=256, dither=Image.Dither.NONE))
    # The returning user's 1.1s greeting was missing in V2.0.51. Export its
    # REAL pixel-perfect FM1 frame, then the ready-to-play RUPTURE screen.
    for mode,flag in ((49,'welcome-back'),(0,'crt-off')):
        frame_path=folder/f'after_{mode}.ppm'
        subprocess.run([str(binary),str(mode),str(frame_path),flag],
                       check=True,stdout=subprocess.DEVNULL)
        frames.append(Image.open(frame_path).convert('RGB').quantize(
            colors=256,dither=Image.Dither.NONE))
    TARGET.parent.mkdir(exist_ok=True, parents=True)
    frames[0].save(TARGET, format='GIF', save_all=True,
                   append_images=frames[1:],
                   duration=[60]*FRAMES+[WELCOME_MS,PLAY_MS],
                   loop=0, optimize=True, disposal=2)

with Image.open(TARGET) as preview:
    assert preview.size == (240, 240)
    assert sum(int(frame.info.get('duration', 0))
               for frame in ImageSequence.Iterator(preview)) == FRAMES * 60+WELCOME_MS+PLAY_MS
    print('PASS native 240x240 SikkenWorks intro GIF:', TARGET,
          'bytes=', TARGET.stat().st_size, 'frames=', preview.n_frames)
