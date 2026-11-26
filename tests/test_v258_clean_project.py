#!/usr/bin/env python3
"""Do not accidentally reintroduce stale installers/tooling into future ZIPs."""
from pathlib import Path
from zipfile import ZipFile
import re

root=Path(__file__).resolve().parents[1]
pack=(root/'tools/package_v258_full_zip.py').read_text()
assert "RELEASE_FWSC" in pack and 'root_firmwares==RELEASE_FWSC' in pack
assert "re.fullmatch(r'package_v" in pack
assert "'vendor','etch-v1.88-source','build'" in pack
assert "'docs' and re.fullmatch" in pack
output=root.parents[1]/'deliverables/RUPTURE-FM1-V2.0.58-CLEAN-FULL-PROJECT.zip'
if output.exists():
 with ZipFile(output) as z:
  assert z.testzip() is None
  ns=z.namelist()
  prefix='RUPTURE-FM1-V2.0.58/'
  assert all(n.startswith(prefix) for n in ns)
  rs=[n[len(prefix):] for n in ns]
  assert {n for n in rs if '/' not in n and n.endswith('.fwsc')}=={
   'RUPTURE-V2.0.58-STABLE.fwsc',
   'RUPTURE-V2.0.58-CORE2-EXPERIMENTAL.fwsc',
  }
  assert not any(re.fullmatch(r'tools/package_v\d+(?:_full_zip)?\.py',n) and
   n not in ('tools/package_v258.py','tools/package_v258_full_zip.py') for n in rs)
  assert not any(n.startswith('vendor/etch-v1.88-source/build/') for n in rs)
  assert not any(n.startswith('docs/TEST-V2.0.') and n!='docs/TEST-V2.0.58.md' for n in rs)
  for needed in ('tests/run_tests.sh','src/cxs.c','README.md',
    'tests/test_v239_note_fx_lcd_guard.c',
    'tests/test_v239_note_dma_meter.c',
    'tests/test_v239_note_dma_wiring.py',
    'tests/test_v239_source_parity.py',
    'tests/test_v239_note_scope_guard.c',
    'tests/test_v239_scope_wiring.py',
    'tests/test_v258_intro.c',
    'tests/test_v258_note_release.c',
    'tests/test_v258_note_release_runtime.c',
    'tests/test_v258_note_release_wiring.py',
    'tests/test_v258_key_pressure.c',
    'tests/test_v258_profiles.c',
    'tests/test_v258_profile_persistence.c',
    'tests/test_v258_touch_persistence.c',
    'tests/test_v258_pressure_wiring.py',
    'tests/test_v258_pressure_ui_wiring.py',
    'tests/test_v258_midi_expression.c',
    'tests/test_v258_midi_expression_wiring.py',
    'tests/test_v258_core2_fx_guard.c',
    'tests/test_v258_core2_fx_stream.c',
    'tests/test_v258_gain_fastpath.c',
    'tests/test_v258_gain_fastpath_wiring.py',
    'tests/test_v258_core2_fx_wait_ema.c',
    'tests/test_v258_core2_fx_wait_ema_wiring.py',
    'tests/test_v258_welcome_dsp_wiring.py',
    'tests/test_v258_core2_stability_wiring.py',
    'tests/test_v258_poll_backoff_wiring.py',
    'tests/test_v258_core2_pressure_wiring.py',
    'tests/test_v258_core2_pressure_guard.c',
    'tools/generate_v258_intro_logo.py',
    'tools/render_v258_intro_gif.py',
    'tools/render_v258_expression_settings.c',
    'ui/loading-frames/RUPTURE-V2.0.58-KEY-PRESSURE-CXS.png',
    'ui/loading-frames/RUPTURE-V2.0.58-KEY-PRESSURE-GROWL.png',
    'ui/loading-frames/RUPTURE-V2.0.58-REVERB-LIVE-MIX.png',
    'ui/loading-frames/RUPTURE-V2.0.58-MIDI-VELOCITY.png',
    'ui/loading-frames/RUPTURE-V2.0.58-BOOT-INTRO-PREVIEW.gif',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_intro_timeline.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_scope_guard.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_dma_meter.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_cpu1_lab.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_core2_fx_guard.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_core2_fx_stream.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_core2_fx_wait_ema.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_core2_pressure_guard.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_release_guard.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_key_touch.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_midi_expression.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_touch_profile.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/rupture_touch_adc.h',
    'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c',
    'docs/RELEASE-LAYOUT.md'):
   assert needed in rs,needed
 print('PASS V2.0.58: clean ZIP preserves current source and exactly two root FWSCs')
else:
 print('SKIP ZIP extraction audit: package not generated yet; source filter contract passed')
