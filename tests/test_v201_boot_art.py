#!/usr/bin/env python3
"""Legacy Signal Breach ROM preserved; new SikkenWorks frames are native."""
from pathlib import Path
from PIL import Image
from hashlib import sha256
import sys

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root))
from tools.compile_v2_signal_breach import midpoint
supplied = root / 'assets/boot-v2-signal-breach/frames'
rendered = root / 'ui/loading-frames'
sources = []
assert sha256((root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_v2_signal_breach_sprites.h').read_bytes()).hexdigest()==\
       '81737c2834659d70f04fc95857ed3e5f70e81019538308adae4e836e21d03cbd'
for frame in range(1, 13):
    with Image.open(supplied / f'frame-{frame:02}.png') as source:
        assert source.size == (240, 240) and source.mode == '1'
        sources.append(source.tobytes())
for i, source in enumerate(sources[:-1]):
    assert len(midpoint(source, sources[i + 1])) == 7200
# The old animation has been intentionally replaced, so displayed samples
# now depict the 86-frame PNG/English-text intro. Keep 23 phase checkpoints.
renders = []
for frame in range(1, 24):
    with Image.open(rendered / f'frame-{frame:02}.png') as result:
        assert result.size == (240, 240)
        renders.append(result.convert('RGB').tobytes())
with Image.open(root/'assets/boot-sikkenworks-photo-v243-display-240.png') as logo:
    expected=bytearray()
    for gray in logo.tobytes():
        expected += bytes(((gray>>3)*255//31,(gray>>2)*255//63,(gray>>3)*255//31))
    assert renders[2] == bytes(expected), 'Uploaded PNG grayscale or RGB565 conversion changed'
assert 0 < sum(renders[0]) < sum(renders[2])
assert not any(renders[7]), 'Logo must finish fading to black'
assert any(renders[10]), 'Text must be visible'
assert not any(renders[22]), 'Final text must fade completely to black'
assert len(list(rendered.glob('frame-*.png'))) == 23, 'Stale loading frames still present'
print('PASS V2.0.43: legacy Signal Breach ROM preserved; 23 native intro samples, uploaded PNG pixels exact')
