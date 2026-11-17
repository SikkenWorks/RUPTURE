#!/usr/bin/env python3
"""Compare all 32 rendered SUB OSC panels to the exact source PNG pixels."""
import subprocess
import tempfile
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
NATIVE = ROOT / 'vendor/etch-v1.88-source/firmware/fm1-native'
SOURCES = ROOT / 'assets/subosc-v2'
FOLDERS = ('RUPTURE-V2-SUBOSC1-FEAR', 'RUPTURE-V2-SUBOSC2-NORMAL')

with tempfile.TemporaryDirectory() as temp:
    executable = Path(temp) / 'host-export'
    subprocess.run(['gcc','-O2','-Wno-unused-function','-I',str(NATIVE),
                    str(NATIVE / 'rupture_host_export.c'), '-o', str(executable)],check=True)
    for page, folder in zip((29,30), FOLDERS):
        for i in range(16):
            preview = Path(temp) / f'{page}-{i}.ppm'
            subprocess.run([str(executable),str(page),str(preview),f'subframe{i}'],check=True)
            with Image.open(SOURCES / folder / 'frames-224x30' / f'frame-{i + 1:02}.png') as original:
                assert original.mode == '1' and original.size == (224,30)
                pixels = original.tobytes()
            with Image.open(preview) as rendered:
                cropped = rendered.crop((8,50,232,80)).convert('1')
                assert cropped.tobytes() == pixels, f'{folder} frame {i + 1}: altered pixels'

print('PASS V2.0.2: 32 original 224x30 1-bit SUB OSC frames rendered exactly')
