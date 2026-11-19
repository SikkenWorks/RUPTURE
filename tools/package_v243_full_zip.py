#!/usr/bin/env python3
"""Deterministic clean RUPTURE project export with exactly two release FWSCs."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib
import json
import re

ROOT=Path(__file__).resolve().parents[1]
META=json.loads((ROOT/'RELEASE-MANIFEST.json').read_text())
assert META['version']=='2.0.43'
RELEASE_FWSC={META['firmware_file'],META['experimental_firmware_file']}
assert RELEASE_FWSC=={
 'RUPTURE-V2.0.43-STABLE.fwsc',
 'RUPTURE-V2.0.43-CORE2-EXPERIMENTAL.fwsc',
}
for label,sha_key,bytes_key in (
 ('firmware_file','firmware_sha256','firmware_bytes'),
 ('experimental_firmware_file','experimental_firmware_sha256','experimental_firmware_bytes'),
):
 path=ROOT/META[label]
 assert path.is_file()
 assert path.stat().st_size==META[bytes_key]
 assert hashlib.sha256(path.read_bytes()).hexdigest()==META[sha_key]

def include(path:Path)->bool:
 rel=path.relative_to(ROOT)
 if any(c in rel.parts for c in ('.git','__pycache__','.pytest_cache','.mypy_cache')):
  return False
 if path.suffix.lower() in ('.pyc','.o','.elf','.pgm','.tmp','.bak'):
  return False
 if rel.parts[:3]==('vendor','etch-v1.88-source','build'):
  return False
 if len(rel.parts)==1 and path.suffix.lower()=='.fwsc':
  return path.name in RELEASE_FWSC
 if len(rel.parts)==2 and rel.parts[0]=='tools':
  if re.fullmatch(r'package_v\d+(?:_full_zip)?\.py',path.name):
   return path.name in ('package_v243.py','package_v243_full_zip.py')
  if path.name in ('generate_v240_intro_logo.py','render_v240_intro_gif.py'):
   return False
  if path.name.startswith('ru157_'):
   return False
 if rel.parts[0]=='tests' and re.fullmatch(r'test_v240_.*\.(?:py|js|c)',path.name):
  return False
 if rel.parts[0]=='native-build' and path.name.startswith('cpu1-live-uac-'):
  return False
 if rel.parts[:2]==('ui','loading-frames') and 'V2.0.40-BOOT-INTRO-' in path.name:
  return False
 if len(rel.parts)>1 and rel.parts[0]=='tools' and rel.parts[1]=='ru157_host_export':
  return False
 if len(rel.parts)==2 and rel.parts[0]=='docs' and re.fullmatch(r'TEST-V2\.0\.\d+\.md',path.name):
  return path.name=='TEST-V2.0.43.md'
 return True

PREFIX='RUPTURE-FM1-V2.0.43'
OUTPUT=ROOT.parents[1]/'deliverables'/'RUPTURE-FM1-V2.0.43-CLEAN-FULL-PROJECT.zip'
OUTPUT.parent.mkdir(parents=True,exist_ok=True)
files=[p for p in sorted(ROOT.rglob('*')) if p.is_file() and include(p)]
names={p.relative_to(ROOT).as_posix() for p in files}

for expected in (
 *RELEASE_FWSC,
 'README.md','CHANGELOG.md','RELEASE-MANIFEST.json',
 'WebInstaller.html','index.html','LICENSE',
 'docs/TEST-V2.0.43.md','docs/RELEASE-LAYOUT.md',
 'tools/package_v243.py','tools/package_v243_full_zip.py',
 'tests/run_tests.sh','tests/test_v210_release_ota.py',
 'tests/test_v243_dual_fwsc.py','tests/test_v243_clean_project.py',
 'tests/test_v243_webinstaller_choice.py',
 'tests/test_v243_webinstaller_choice_runtime.js',
 'tests/test_v243_core2_fx_wiring.py',
 'tests/test_v243_led_intro_wiring.py',
 'tests/test_v243_png_branding.py',
 'tests/test_v243_core2_fx_pcm.c',
 'tests/test_v243_note_release.c',
 'tests/test_v243_note_release_runtime.c',
 'tests/test_v243_note_release_wiring.py',
 'tests/test_v233_production_pcm_parity.py',
 'tests/test_v239_source_parity.py',
 'tests/test_v239_note_fx_lcd_guard.c',
 'tests/test_v239_note_dma_meter.c',
 'tests/test_v239_note_dma_wiring.py',
 'tests/test_v239_note_scope_guard.c',
 'tests/test_v239_scope_wiring.py',
 'tests/test_v243_intro.c',
 'tools/generate_v243_intro_logo.py',
 'tools/render_v243_intro_gif.py',
 'ui/loading-frames/RUPTURE-V2.0.43-BOOT-INTRO-PREVIEW.gif',
 'assets/boot-sikkenworks-photo-v243.png',
 'assets/boot-sikkenworks-photo-v243-display-240.png',
 'vendor/etch-v1.88-source/firmware/fm1-native/rupture_intro_timeline.h',
 'vendor/etch-v1.88-source/firmware/fm1-native/rupture_sikkenworks_boot_logo.h',
 'tests/test_v235_offload_guard.c','tests/test_v235_offload_wiring.py',
 'native-build/app.bin','native-build/native-build-options.json',
 'native-build/cpu1-fx-app.bin',
 'native-build/cpu1-fx-app.bin.audit.json',
 'native-build/cpu1-fx-build-options.json',
 'vendor/etch-v1.88-source/firmware/fm1-native/build.py',
 'vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c',
 'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c',
 'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_dma_meter.h',
 'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_scope_guard.h',
 'vendor/etch-v1.88-source/firmware/fm1-native/rupture_cpu1_lab.h',
 'vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_release_guard.h',
 'reference/firmware/RUPTURE-V1.86.1.fwsc',
 'reference/firmware/RUPTURE-V2.0.9.fwsc',
):
 assert expected in names,expected

root_firmwares={Path(n).name for n in names if '/' not in n and n.endswith('.fwsc')}
assert root_firmwares==RELEASE_FWSC,root_firmwares
assert not any(re.fullmatch(r'tools/package_v\d+(?:_full_zip)?\.py',n) and
 n not in ('tools/package_v243.py','tools/package_v243_full_zip.py') for n in names)
assert not any(n.startswith('vendor/etch-v1.88-source/build/') for n in names)
assert not any(n.startswith('docs/TEST-V2.0.') and n!='docs/TEST-V2.0.43.md' for n in names)

# A "clean" project must never silently omit a test or native source.
for sub in ('tests','vendor/etch-v1.88-source/firmware/fm1-native','src','assets','ui'):
 for p in (ROOT/sub).rglob('*'):
  if p.is_file() and include(p):
   assert p.relative_to(ROOT).as_posix() in names,p

with ZipFile(OUTPUT,'w',ZIP_DEFLATED,compresslevel=6) as archive:
 for p in files:
  archive.write(p,f'{PREFIX}/{p.relative_to(ROOT).as_posix()}')
with ZipFile(OUTPUT) as archive:
 assert archive.testzip() is None,'ZIP CRC failed'
 zipped={p[len(PREFIX)+1:] for p in archive.namelist()}
 assert zipped==names
 assert {Path(p).name for p in zipped if '/' not in p and p.endswith('.fwsc')}==RELEASE_FWSC
print('PASS V2.0.43 CLEAN project:',OUTPUT,'files=',len(files),'bytes=',OUTPUT.stat().st_size)
