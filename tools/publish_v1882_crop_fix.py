#!/usr/bin/env python3
"""Publish complete V1.88.2 crop-only RUPTURE release, verify every output."""
from pathlib import Path
import base64,hashlib,json,re,shutil,zipfile
R=Path(__file__).resolve().parents[1]
O=Path('/mnt/data')
V='1.88.2'
fwname=f'RUPTURE-V{V}.fwsc'
fw=(R/fwname).read_bytes()
sha=lambda v:hashlib.sha256(v).hexdigest()
m=json.loads((R/'RELEASE-MANIFEST.json').read_text())
assert m['version']==V and m['firmware_sha256']==sha(fw) and m['firmware_bytes']==len(fw)==610086
assert (R/'CHANGELOG.md').read_text().startswith(f'# RUPTURE V{V}')
for name in ('WebInstaller.html','index.html'):
 h=(R/name).read_text()
 assert 'Stop at the FIRST NUL' in h and 'read-only identity queries' in h
 assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',h,re.S).group(1))==fw
 assert f"const PKG_HASH='{sha(fw)}'" in h
 assert f'const PKG_LEN={len(fw)}' in h
 assert f'V{V}' in h
outputs={
 fwname:fwname,
 f'RUPTURE-WebInstaller-V{V}.html':'WebInstaller.html',
 f'RUPTURE-V{V}-INDEX.html':'index.html',
 f'RUPTURE-TEST-RESULTS-V{V}.md':f'RUPTURE-TEST-RESULTS-V{V}.md',
 f'RUPTURE-CHEAT-SHEET-V{V}.html':'CHEAT-SHEET.html',
 f'RUPTURE-V{V}-ORIGINAL-CROPS-VS-FM1.png':'assets/bend-v1882-crop-ui/BEND-V1.88.2-ORIGINAL-1TO1-CROPS-VS-FM1-40FRAMES.png',
 f'RUPTURE-V{V}-BEND-CRT-ON-OFF.png':'assets/bend-v1882-crop-ui/BEND-V1.88.2-CROPPED-FIVE-CURVES-CRT-ON-OFF.png',
 f'RUPTURE-V{V}-ALL-40-CROPPED-SPRITES.png':'vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188/BEND-V1.88.2-CROPPED-40-FRAMES-3X.png',
}
for out,rel in outputs.items():
 shutil.copy2(R/rel,O/out)
 assert (O/out).read_bytes()==(R/rel).read_bytes()

site_zip=O/f'RUPTURE-V{V}-GITHUB-PAGES.zip'
site=['.nojekyll','index.html','WebInstaller.html','README.md','CHANGELOG.md','CHEAT-SHEET.html','WEBSITE-DEPLOY.md','LICENSE','RELEASE-MANIFEST.json',fwname,f'RUPTURE-TEST-RESULTS-V{V}.md']
site+=sorted(p.relative_to(R).as_posix() for p in (R/'assets').rglob('*') if p.is_file() and 'bend-v188-native-ui' not in p.parts)
with zipfile.ZipFile(site_zip,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 for name in site:
  assert (R/name).is_file(),name
  z.write(R/name,name)

full=O/f'RUPTURE-FM1-V{V}-CROP-FIX-FULL-SOURCE.zip'
with zipfile.ZipFile(full,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(R.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(R)
  if '__pycache__' in rel.parts or '.pytest_cache' in rel.parts:continue
  if p.suffix in ('.pyc','.log'):continue
  z.write(p,rel.as_posix())

preview=O/f'RUPTURE-V{V}-CROPPED-SPRITES-PREVIEWS.zip'
with zipfile.ZipFile(preview,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 for rel in ['assets/bend-v1881-approved-poster/approved-poster.jpg',
             'assets/bend-v1882-crop-ui/BEND-V1.88.2-ORIGINAL-1TO1-CROPS-VS-FM1-40FRAMES.png',
             f'RUPTURE-TEST-RESULTS-V{V}.md']:
  z.write(R/rel,rel)
 for folder,label in [(R/'vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188','BEND_SPRITES_CROPPED_1TO1'),
                      (R/'assets/bend-v1882-crop-ui','NATIVE_240x240_UI')]:
  for p in sorted(folder.rglob('*.png')):
   z.write(p,label+'/'+p.relative_to(folder).as_posix())
 p=R/'vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188/POSTER-SOURCE-EXTRACTION.json'
 z.write(p,'BEND_SPRITES_CROPPED_1TO1/'+p.name)

for p in (site_zip,full,preview):
 with zipfile.ZipFile(p) as z:
  assert z.testzip() is None,p
  if p!=preview:
   assert z.read(fwname)==fw
   assert z.read('WebInstaller.html')==(R/'WebInstaller.html').read_bytes()
   assert z.read('index.html')==(R/'index.html').read_bytes()
  if p==full:
   for name in ['tools/extract_approved_poster_frames_v1881.py','tools/package_v1882_crop_fix.py',
                'tools/publish_v1882_crop_fix.py',
                'vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_sprites.h',
                'vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_storyboard.h',
                'native-build/app.bin','tests/test_v1882_crop_fix.py',
                'assets/bend-v1881-approved-poster/approved-poster.jpg']:
    assert name in z.namelist(),name
  else:
   pass
  if p==preview:
   names=z.namelist()
   assert len([s for s in names if re.search(r'/frame_\d\d.png$',s) and s.startswith('BEND_SPRITES')])==40
   assert len([s for s in names if '/poster_crop_' in s and s.endswith('.png')])==40
   assert len([s for s in names if '/source_camera_crop_' in s and s.endswith('.png')])==40
  print('VERIFIED',p.name,'bytes',p.stat().st_size,'files',len(z.namelist()))
for name in [*outputs,site_zip.name,full.name,preview.name]:
 p=O/name
 assert p.is_file() and p.stat().st_size>0,p
 print('DOWNLOAD',p.name,p.stat().st_size)
print('FIRMWARE SHA256',sha(fw))
