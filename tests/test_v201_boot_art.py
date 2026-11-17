#!/usr/bin/env python3
"""Pixel-for-pixel comparison of the actual rendered FM1 loading sequence."""
from pathlib import Path
from PIL import Image
import sys

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root))
from tools.compile_v2_signal_breach import midpoint
supplied = root / 'assets/boot-v2-signal-breach/frames'
rendered = root / 'ui/loading-frames'
sources = []
renders = []
for frame in range(1, 13):
    with Image.open(supplied / f'frame-{frame:02}.png') as source:
        assert source.size == (240, 240) and source.mode == '1'
        sources.append(source.tobytes())
for frame in range(1, 24):
    with Image.open(rendered / f'frame-{frame:02}.png') as result:
        assert result.size == (240, 240)
        renders.append(result.convert('1').tobytes())
for i, source in enumerate(sources):
    assert renders[i * 2] == source, f'Keyframe {i + 1} changed'
    if i + 1 < len(sources):
        assert renders[i * 2 + 1] == midpoint(source, sources[i + 1]), f'In-between {i + 1} changed'
assert len(list(rendered.glob('frame-*.png'))) == 23, 'Stale loading frames still present'
print('PASS V2.0.1: 12 exact 1-bit source frames and 11 reproducible intermediate frames')
