#!/usr/bin/env python3
"""Preserve the user's actual PNG gray pixels on the FM1 RGB565 LCD."""
from pathlib import Path
from hashlib import sha256
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'assets/boot-sikkenworks-photo-v243.png'
TARGET = ROOT / 'vendor/etch-v1.88-source/firmware/fm1-native/rupture_sikkenworks_boot_logo.h'
PREVIEW = ROOT / 'assets/boot-sikkenworks-photo-v243-display-240.png'
EXPECTED_SHA = '85d690dd543a5fdd1d3e1d0267760a37148b9a33b536b24560ef1a46966c0dff'
assert sha256(SOURCE.read_bytes()).hexdigest() == EXPECTED_SHA
img = Image.open(SOURCE).convert('RGBA')
# The transparent white gutters in the supplied PNG are outside the artwork.
# Preserve the actual portrait/space photograph, including internal stars.
bbox = img.getchannel('A').getbbox()
assert bbox and img.size == (1402, 1122)
img = img.crop(bbox)
size = (240, round(img.height * 240 / img.width))
assert size[1] <= 240
img = img.resize(size, Image.Resampling.NEAREST)
canvas = Image.new('RGBA', (240, 240), (0, 0, 0, 255))
canvas.alpha_composite(img, (0, (240 - size[1]) // 2))
# Source pixels retain their 256 grayscale levels. RGB565 conversion occurs
# only when the firmware draws the LCD stripe. NO Bayer/ordered dithering.
gray = canvas.convert('L')
gray.save(PREVIEW)
pixels = gray.tobytes()
assert len(pixels) == 240 * 240
lines = [
    '/* Native 240x240 SikkenWorks photograph from the supplied PNG.',
    ' * Transparent gutters cropped; aspect-preserved NEAREST resize.',
    ' * Exact 8-bit grayscale source values; no dithering, no antialiasing.',
    ' * The ST7789 RGB565 LCD displays these as actual gray levels.',
    ' * Source SHA256: ' + EXPECTED_SHA + ' */',
    '#ifndef RUPTURE_SIKKENWORKS_BOOT_LOGO_H',
    '#define RUPTURE_SIKKENWORKS_BOOT_LOGO_H',
    '#include <stdint.h>',
    '#define RU_SIKKENWORKS_LOGO_WIDTH 240u',
    '#define RU_SIKKENWORKS_LOGO_HEIGHT 240u',
    '#define RU_SIKKENWORKS_LOGO_BYTES 57600u',
    'static const uint8_t ru_sikkenworks_logo_gray[RU_SIKKENWORKS_LOGO_BYTES]={',
]
for offset in range(0, len(pixels), 32):
    lines.append(' ' + ','.join('0x%02x' % v for v in pixels[offset:offset + 32]) + ',')
lines += ['};', '#endif /* RUPTURE_SIKKENWORKS_BOOT_LOGO_H */', '']
TARGET.write_text('\n'.join(lines))
print(f'Actual uploaded PNG portrait: {len(pixels)} exact gray samples, '
      f'{len(set(pixels))} unique levels, NO dithering')
