#!/usr/bin/env python3
"""Regression inspection of read-only ID probe, all write gates, trusted loader."""
from pathlib import Path
import zipfile,hashlib,re,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
with tempfile.TemporaryDirectory() as d:
 exe=Path(d)/'flash-chip'
 subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-I',str(src),str(root/'tests/test_v1852_flash_chip.c'),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
original=Path('/mnt/data/RUPTURE-FM1-V1.85.1-FULL-SOURCE-FWSC.zip')
if original.is_file():
 with zipfile.ZipFile(original) as z:
  a='vendor/etch-v1.88-source/firmware/fm1-native/'
  # These files must remain completely unchanged.
  for p in ['audio_core.c','audio_target.c','native_fx.c','rupture_eq2.c',
            'native_project_codec.c','hal/fm1_audio.h','hal/fm1_isr.S',
            'ota_trusted_loader.h','build.py']:
   assert z.read(a+p)==(src/p).read_bytes(),p
  # Report precisely which native files are modified.
  names=[s for s in z.namelist() if s.startswith(a) and s.endswith(('.c','.h','.S'))]
  differences=sorted(s.removeprefix(a) for s in names if z.read(s)!=(root/s).read_bytes())
  assert differences==sorted(['hal/fm1_flash.h','wave_store_target.c','ota_target.c','ota.c']),differences
  print('PASS: native diff limited to four files + new hal/fm1_flash_id.h; trusted loader unchanged')
store=(src/'wave_store_target.c').read_text()
assert store.count('!native_flash_write_allowed()')==3
assert store.count('FL_FAR(fl_jedec_ram)()')==1
assert store.index('FL_FAR(fl_jedec_ram)()')>store.index('static void native_library_boot(void)')
assert store.index('FL_FAR(fl_jedec_ram)()')<store.index('native_store_status=etch_store_load_project')
ota=(src/'ota_target.c').read_text()
assert ota.count('!native_flash_write_allowed()')==2
reply=(src/'ota.c').read_text()
assert 'id[24] = (uint8_t)(native_flash_jedec_cached >> 16)' in reply
assert 'id[25] = (uint8_t)(native_flash_jedec_cached >> 8)' in reply
assert 'id[26] = (uint8_t)native_flash_jedec_cached' in reply
print('PASS: boot ID probe and storage + OTA write safety gates wired; identity exposes ID without a loader rewrite')
