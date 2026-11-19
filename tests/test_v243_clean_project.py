#!/usr/bin/env python3
"""Do not accidentally reintroduce stale installers/tooling into future ZIPs."""
from pathlib import Path
from zipfile import ZipFile
import re

root=Path(__file__).resolve().parents[1]
pack=(root/'tools/package_v243_full_zip.py').read_text()
assert "RELEASE_FWSC" in pack and 'root_firmwares==RELEASE_FWSC' in pack
assert "re.fullmatch(r'package_v" in pack
assert "'vendor','etch-v1.88-source','build'" in pack
assert "'docs' and re.fullmatch" in pack
output=root.parents[1]/'deliverables/RUPTURE-FM1-V2.0.43-CLEAN-FULL-PROJECT.zip'
if output.exists():
 with ZipFile(output) as z:
  assert z.testzip() is None
  ns=z.namelist()
  prefix='RUPTURE-FM1-V2.0.43/'
  assert all(n.startswith(prefix) for n in ns)
  rs=[n[len(prefix):] for n in ns]
  assert {n for n in rs if '/' not in n and n.endswith('.fwsc')}=={
   'RUPTURE-V2.0.43-STABLE.fwsc',
   'RUPTURE-V2.0.43-CORE2-EXPERIMENTAL.fwsc',
  }
  assert not any(re.fullmatch(r'tools/package_v\d+(?:_full_zip)?\.py',n) and
   n not in ('tools/package_v243.py','tools/package_v243_full_zip.py') for n in rs)
  assert not any(n.startswith('vendor/etch-v1.88-source/build/') for n in rs)
  assert not any(n.startswith('docs/TEST-V2.0.') and n!='docs/TEST-V2.0.43.md' for n in rs)
  for needed in ('tests/run_tests.sh','src/cxs.c','README.md',
    'tests/test_v239_note_fx_lcd_guard.c',
    'tests/test_v239_note_dma_meter.c',
    'tests/test_v239_note_dma_wiring.py',
    'tests/test_v239_source_parity.py',
    'tests/test_v239_note_scope_guard.c',
    'tests/test_v239_scope_wiring.py',
    'tests/test_v243_intro.c',
    'tests/test_v243_note_release.c',
    'tests/test_v243_note_release_runtime.c',
    'tests/test_v243_note_release_wiring.py',
    'tools/generate_v243_intro_logo.py',
    'tools/render_v243_intro_gif.py',
    'ui/loading-frames/RUPTURE-V2.0.43-BOOT-INTRO-PREVIEW.gif',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_intro_timeline.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_scope_guard.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_dma_meter.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_cpu1_lab.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_release_guard.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c',
    'docs/RELEASE-LAYOUT.md'):
   assert needed in rs,needed
 print('PASS V2.0.43: clean ZIP preserves current source and exactly two root FWSCs')
else:
 print('SKIP ZIP extraction audit: package not generated yet; source filter contract passed')
