#!/usr/bin/env python3
"""RUPTURE 1.65: reproducible SOFTWARE-only release candidate gate.
Never claims a physical FM1 recovery, stock V15 flashing or audio soak test.
"""
from pathlib import Path
from subprocess import run
from tempfile import TemporaryDirectory
import hashlib, json, sys

root=Path(__file__).resolve().parents[1]
m=json.loads((root/'RELEASE-MANIFEST.json').read_text())
assert m['version']=='1.65' and m['firmware_identity']=='FM-1_968'
fw=(root/m['firmware_file']).read_bytes()
assert len(fw)==m['firmware_bytes'] and hashlib.sha256(fw).hexdigest()==m['firmware_sha256']
app=(root/'native-build/app.bin').read_bytes()
assert hashlib.sha256(app).hexdigest()==m['app_sha256']
pins=json.loads((root/'docs/V1.64-RC-PARITY-PINS.json').read_text())
for rel,sha in pins['unchanged_native_source_sha256'].items():
    p=root/rel
    assert p.is_file() and hashlib.sha256(p.read_bytes()).hexdigest()==sha, f'unexpected source drift: {rel}'
previous=(root/'reference/RUPTURE-V1.64-app-baseline.bin').read_bytes()
assert hashlib.sha256(previous).hexdigest()==pins['baseline_app_sha256']
assert len(app)==len(previous)
diffs=[i for i,(a,b) in enumerate(zip(previous,app)) if a!=b]
assert len(diffs)==1 and previous[diffs[0]]==ord('4') and app[diffs[0]]==ord('5'), f'app differences: {diffs[:20]}'
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as ff
report,errors=ff.verify_fwsc(root/m["firmware_file"])
assert not errors and report['product']=='FM-1_968' and report['flash']['app_crc_ok'], errors
ref=(root/'reference/RUPTURE-V1.64-baseline.fwsc').read_bytes()
assert hashlib.sha256(ref).hexdigest()==pins['baseline_firmware_sha256']
a={e['name']:e['data'] for e in ff.parse_ufw(ref)['entries']}
b={e['name']:e['data'] for e in ff.parse_ufw(fw)['entries']}
assert a.keys()==b.keys()=={'flash.bin','ota.bin'}
assert a['ota.bin']==b['ota.bin'], 'OTA loader changed'
assert a['flash.bin'][:0x4000]==b['flash.bin'][:0x4000], 'protected 16KiB changed'
flash_diffs=[i for i,(v,w) in enumerate(zip(a['flash.bin'],b['flash.bin'])) if v!=w]
assert len(flash_diffs)==9 and flash_diffs[:8]==[0x4000,0x4001,0x4002,0x4003,0x4020,0x4021,0x4022,0x4023],flash_diffs
assert flash_diffs[8]==0x4000+0x120+diffs[0],flash_diffs
print(f'PASS pinned parity: {len(pins["unchanged_native_source_sha256"])} native source files, 1 app byte, 9 flash bytes (8 app metadata/CRC), OTA loader and boothead identical')

test_cases=[
 ('LCD scheduling','tests/test_v164_lcd_budget.c',[]),
 ('System status','tests/test_v164_system_info.c',[]),
 ('Undo/save safety','tests/test_v163_edit_safety.c',[]),
 ('SCL/knob feedback','tests/test_v162_ui_labels.c',[]),
 ('Encoder feel','tests/test_v161_native_controls.c',[]),
 ('First-run/format','tests/test_v160_welcome.c',[]),
 ('LFO/ENV shake','tests/test_v158_panel_shake.c',[]),
 ('Volcano/close','tests/test_v157_volcano_anim.c',[]),
 ('Panel timing','tests/test_v156_panel_anim.c',['-I'+str(root/'vendor/etch-v1.88-source/firmware/fm1-native')]),
 ('CRT and QR','tests/test_v151_crt.c',[]),
 ('Screensaver','tests/test_v145_osc_saver.c',[str(root/'vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c')]),
 ('CXS pressure','tests/test_v155_cxs_pressure.c',[str(root/'vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c')]),
 ('Pixel-correct moving sprites','tests/test_v165_sprite_mapping.c',[]),
 ('Native OTA valid/bad data','tests/test_v143_ota_reclaim.c',[str(root/'vendor/etch-v1.88-source/firmware/fm1-native/native_digest.c')]),
]
with TemporaryDirectory() as d:
  d=Path(d)
  for i,(name,source,flags) in enumerate(test_cases):
    exe=d/f'test_{i}'
    run(['gcc','-std=c11','-O2','-Wno-unused-function','-w',str(root/source),*flags,'-lm','-o',str(exe)],check=True,timeout=30)
    args=[str(exe)]
    if 'OTA' in name:args.append(str(root/m['firmware_file']))
    r=run(args,capture_output=True,text=True,timeout=40)
    if r.returncode:
      print(r.stdout,r.stderr,file=sys.stderr)
      raise AssertionError(f'failed {name}: {r.returncode}')
    print('PASS',name)
  for file in ('test_v165_png_parity.py','test_v156_panel_sprites.py','test_v157_volcano_sprites.py'):
    run([sys.executable,str(root/'tests'/file)],check=True,stdout=sys.stdout,timeout=40)
print('PASS V1.65 software RC gate; on-device tests remain PENDING')
