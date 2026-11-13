#!/usr/bin/env python3
"""Publish checked V1.86 firmware, source, GitHub Pages and BEND image previews."""
from pathlib import Path
import zipfile,hashlib,json,base64,re,shutil
root=Path(__file__).resolve().parents[1]
out=Path('/mnt/data')
manifest=json.loads((root/'RELEASE-MANIFEST.json').read_text())
assert manifest['version']=='1.86'
fw=(root/'RUPTURE-V1.86.fwsc').read_bytes()
sha=hashlib.sha256(fw).hexdigest()
assert sha==manifest['firmware_sha256']

# Explicit, independently linked artifacts, all copied from current built source.
standalone={
 'RUPTURE-V1.86.fwsc':'RUPTURE-V1.86.fwsc',
 'RUPTURE-WebInstaller-V1.86.html':'WebInstaller.html',
 'RUPTURE-V1.86-INDEX.html':'index.html',
 'RUPTURE-TEST-RESULTS-V1.86.md':'RUPTURE-TEST-RESULTS-V1.86.md',
 'RUPTURE-CHEAT-SHEET-V1.86.html':'CHEAT-SHEET.html',
}
for public,source in standalone.items():
 shutil.copy2(root/source,out/public)
 assert (out/public).read_bytes()==(root/source).read_bytes()

# Publish site exactly as it is locally verified, including assets, instructions,
# hidden .nojekyll and downloadable .fwsc.
web_path=out/'RUPTURE-V1.86-WEBSITE-GITHUB-PAGES.zip'
site_files=['.nojekyll','index.html','WebInstaller.html','README.md','CHANGELOG.md',
            'CHEAT-SHEET.html','WEBSITE-DEPLOY.md','LICENSE','RELEASE-MANIFEST.json',
            'RUPTURE-TEST-RESULTS-V1.86.md','RUPTURE-V1.86.fwsc']
site_files += sorted(p.relative_to(root).as_posix() for p in (root/'assets').rglob('*') if p.is_file())
with zipfile.ZipFile(web_path,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 for name in site_files:
  assert (root/name).is_file(),name
  z.write(root/name,name)

# Full buildable project (no cross-compiler, which the user provides separately).
source_path=out/'RUPTURE-FM1-V1.86-FULL-SOURCE-FWSC.zip'
with zipfile.ZipFile(source_path,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(root.rglob('*')):
  if not p.is_file():continue
  relative=p.relative_to(root)
  if '__pycache__' in relative.parts or '.pytest_cache' in relative.parts:continue
  if p.name.endswith(('.pyc','.log')):continue
  z.write(p,relative.as_posix())

# Pixel-perfect original renders and 8 raw sprite source PNGs.
previews=out/'RUPTURE-V1.86-BEND-UI-CRT-ON-OFF.zip'
with zipfile.ZipFile(previews,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 art=root/'assets/bend-ui-v186'
 for p in sorted(art.glob('*.png')):z.write(p,'240x240_UI/'+p.name)
 frame_dir=root/'vendor/etch-v1.88-source/firmware/fm1-native/bend_frames'
 for p in sorted(frame_dir.glob('bend_frame_*.png')):z.write(p,'224x30_SPRITE_FRAMES/'+p.name)

# Validate exact paths, ZIP checksums and embedded release bits.
for path in (web_path,source_path,previews):
 with zipfile.ZipFile(path) as z:
  assert z.testzip() is None,path
  if path!=previews:
   assert z.read('RUPTURE-V1.86.fwsc')==fw
   assert z.read('index.html')==(root/'index.html').read_bytes()
  if path==source_path:
   assert z.read('vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_sprites.h')
   assert z.read('native-build/app.bin')==(root/'native-build/app.bin').read_bytes()
   assert z.read('vendor/etch-v1.88-source/firmware/fm1-native/hal/fm1_flash_id.h')
  if path==previews:
   assert len([x for x in z.namelist() if x.startswith('240x240_UI/')])==18
   assert len([x for x in z.namelist() if x.startswith('224x30_SPRITE_FRAMES/')])==8
 print('VERIFIED',path.name,path.stat().st_size,len(zipfile.ZipFile(path).namelist()),'files')

installer=(out/'RUPTURE-WebInstaller-V1.86.html').read_text()
site=(out/'RUPTURE-V1.86-INDEX.html').read_text()
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',installer,re.S).group(1))==fw
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',site,re.S).group(1))==fw
assert (out/'RUPTURE-V1.86.fwsc').read_bytes()==fw
print('All download files exist and match V1.86; firmware SHA256',sha)
