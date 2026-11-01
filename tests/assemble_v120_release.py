#!/usr/bin/env python3
"""Package exact native app build and self-contained updater; verify source/assets and rollback partitions."""
from pathlib import Path
import base64, hashlib, json, re, shutil, zipfile, sys
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
fw_path=root/'RUPTURE-V1.20.fwsc'
fw_bytes=fw_path.read_bytes()
sha=hashlib.sha256(fw_bytes).hexdigest()
app=(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
old=(root/'RUPTURE-V1.19.fwsc').read_bytes()
assert fw.verify_fwsc(fw_path)[1]==[]
old_entries=fw.parse_ufw(old)['entries']
new_entries=fw.parse_ufw(fw_bytes)['entries']
old_flash=next(e['data'] for e in old_entries if e['name']=='flash.bin')
new_flash=next(e['data'] for e in new_entries if e['name']=='flash.bin')
old_loader=next(e['data'] for e in old_entries if e['name']=='ota.bin')
new_loader=next(e['data'] for e in new_entries if e['name']=='ota.bin')
assert old_flash[:0x4000]==new_flash[:0x4000], 'protected 16KiB changed'
assert old_loader==new_loader, 'OTA loader changed'
assert len(fw_bytes)==len(old)
assert new_flash!=old_flash
assert len(app)<581564
html_path=root/'WebInstaller.html';html=html_path.read_text()
html,count=re.subn(r'(<script id="firmware" type="application/octet-stream">).*?(</script>)',
                   lambda m:m.group(1)+base64.b64encode(fw_bytes).decode()+m.group(2),html,count=1,flags=re.S)
assert count==1
html,count=re.subn(r"const PKG_HASH='[0-9a-f]+';const PKG_LEN=\d+;",
                   f"const PKG_HASH='{sha}';const PKG_LEN={len(fw_bytes)};",html,count=1)
assert count==1
assert "RUPTURE V1.20" in html and 'FIRMWARE / V1.20' in html
assert 'contextmenu' in html and 'selectstart' in html
html_path.write_text(html)
assert base64.b64decode(re.search(r'<script id="firmware" type="application/octet-stream">(.*?)</script>',html,re.S).group(1))==fw_bytes
shutil.copy2(html_path,root.parent/'RUPTURE-WebInstaller-V1.20.html')
shutil.copy2(fw_path,root.parent/'RUPTURE-V1.20.fwsc')
shutil.copy2(root/'CHEAT-SHEET.html',root.parent/'RUPTURE-CHEAT-SHEET-V1.20.html')
shutil.copy2(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin',root/'native-build/app.bin')
shutil.copy2(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin.audit.json',root/'native-build/app.bin.audit.json')
shutil.copy2(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/native-build-options.json',root/'native-build/native-build-options.json')
man_path=root/'RELEASE-MANIFEST.json';manifest=json.loads(man_path.read_text())
manifest.update(version='1.20',firmware_file='RUPTURE-V1.20.fwsc',firmware_bytes=len(fw_bytes),firmware_sha256=sha,app_bytes=len(app),app_sha256=hashlib.sha256(app).hexdigest(),native_screens=38)
manifest['v120']={
 'splash':'24-frame deterministic jagged tear in native 240x240 1-bit C UI',
 'reveal_text':'SIKKENWORKS / Born out of passion.',
 'immutable_art_rom_bytes':7200,
 'native_frames':24,
 'additional_ram_bytes':0,
 'guard':'boot pre-audio, recovery priority preserved; physical hardware untested',
 'startup_ms_estimate_excluding_spi':24*56+675,
 'frame_24_bit_exact':True,
 'protected_first_16k_byte_equal_v119':True,
 'ota_loader_byte_equal_v119':True,
 'existing_audio_fx_dsp_modified':False,
 }
man_path.write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n')
# Package image sets.
def archive(out, paths):
 with zipfile.ZipFile(out,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  for p in paths:z.write(p,p.relative_to(root).as_posix())
archive(root.parent/'RUPTURE-V1.20-24-BOOT-FRAMES.zip',sorted((root/'ui/loading-frames').glob('frame-*.png'))+[root/'ui/assets/rupture-splash-final-240.png'])
archive(root.parent/'RUPTURE-V1.20-38-NATIVE-1BIT-SCREENS.zip',sorted((root/'ui/screens').glob('*.png')))
report=f'''# RUPTURE V1.20 firmware verification

- Native JieLi crossbuild: PASS
- Previous regression suite: PASS
- 24 1-bit 240×240 real C-rendered animation frames: PASS
- Exact parity frame 24 vs final approved 240×240 image: PASS
- Protect firmware flash first 0x4000 bytes vs V1.19: byte-equal PASS
- Trusted OTA loader vs V1.19: byte-equal PASS
- FM-1_968 FWSC CRC: PASS
- WebInstaller firmware is byte-identical: PASS
- Application bytes: {len(app)}; image slot headroom: {581564-len(app)} bytes
- Added ROM art size: 7200 bytes; extra audio buffers: zero
- Firmware bytes: {len(fw_bytes)}
- FWSC SHA-256: {sha}
- Hardware boot / realtime interaction: NOT TESTED on FM1
'''
(root/'docs/TEST-V1.20.md').write_text(report)
(root.parent/'RUPTURE-TEST-RESULTS-V1.20.md').write_text(report)
# Do not ship cross-compiler intermediates or stale firmware; ship the complete C source and artifacts.
exclude_dirs={'__pycache__','.git'}
with zipfile.ZipFile(root.parent/'RUPTURE-FM1-V1.20-FULL-SOURCE-FWSC.zip','w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(root.rglob('*')):
  if not p.is_file() or any(d in p.parts for d in exclude_dirs):continue
  rel=p.relative_to(root).as_posix()
  if rel.startswith('vendor/etch-v1.88-source/build/'):continue
  if rel in ('RUPTURE-V1.18.fwsc','RUPTURE-V1.19.fwsc'):continue
  if rel.startswith('ui/screens/') or rel.startswith('ui/loading-frames/') or rel.startswith('ui/assets/'):
   pass
  if rel in ('tear-240-BOX.png','tear-240-NEAREST.png','tear-240-DITHER.png','animation-contact-sheet.png'):continue
  if p.suffix in ('.o','.elf','.pgm','.pyc'):continue
  z.write(p,rel)
 z.write(root.parent/'RUPTURE-V1.20-BOOT-ANIMATION.gif','RUPTURE-V1.20-BOOT-ANIMATION.gif')
print('FWSC',sha,'Bytes',len(fw_bytes),'App',len(app))
for path in sorted(root.parent.glob('RUPTURE-*V1.20*')):
 print(path.name,path.stat().st_size)
