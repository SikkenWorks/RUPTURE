#!/usr/bin/env python3
"""Produce a buildable, versioned full-project ZIP with source + safe FWSC."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json

root=Path(__file__).resolve().parents[1]
meta=json.loads((root/'RELEASE-MANIFEST.json').read_text())
assert meta['version']=='2.0.13'
fwsc=root/'RUPTURE-V2.0.13.fwsc'
assert fwsc.is_file()
assert fwsc.stat().st_size==meta['firmware_bytes']

output=root.parents[1]/'deliverables'/'RUPTURE-FM1-V2.0.13-GROWL-EXPRESSION-FULL-PROJECT.zip'
output.parent.mkdir(parents=True,exist_ok=True)
prefix='RUPTURE-FM1-V2.0.13'

files=0
with ZipFile(output,'w',ZIP_DEFLATED,compresslevel=6) as z:
 for f in sorted(root.rglob('*')):
  if not f.is_file():continue
  rel=f.relative_to(root)
  if any(x in rel.parts for x in ('__pycache__','.pytest_cache','.git')):continue
  if f.suffix.lower() in ('.pyc','.o','.elf','.pgm'):continue
  if rel.parts[:3]==('vendor','etch-v1.88-source','build'):continue
  if f.parent==root and f.suffix.lower()=='.fwsc' and f!=fwsc:continue
  z.write(f,f'{prefix}/{rel.as_posix()}')
  files+=1
 names=set(z.namelist())
 for needed in (
  f'{prefix}/RUPTURE-V2.0.13.fwsc',
  f'{prefix}/vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h',
  f'{prefix}/vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c',
  f'{prefix}/tools/package_v213.py',
  f'{prefix}/docs/TEST-V2.0.13.md',
  f'{prefix}/tests/test_v213_expression.c',
  f'{prefix}/tests/test_v213_dual_knob_storage.c',
 ):
  assert needed in names,needed
with ZipFile(output) as z:
 assert z.testzip() is None,'ZIP archive CRC check failed'
print('PASS V2.0.13 complete project:',output,'files=',files,'bytes=',output.stat().st_size)
