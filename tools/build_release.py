#!/usr/bin/env python3
"""Rebuild RUPTURE release artifacts from the verified manifest and native app.

The existing updater and protected bootheader are byte-identical to V1.20;
this script only packages the already compiled/validated release and site.
"""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil, sys
root=Path(__file__).resolve().parents[1]
base=root.parent
meta=json.loads((root/'RELEASE-MANIFEST.json').read_text())
ver=meta['version']
assert ver=='1.31', 'This release script is validated for V1.30 only'
fw=root/meta['firmware_file']
assert hashlib.sha256(fw.read_bytes()).hexdigest()==meta['firmware_sha256']
assert len(fw.read_bytes())==meta['firmware_bytes']
appdir=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05'
app=(appdir/'app.bin').read_bytes()
assert hashlib.sha256(app).hexdigest()==meta['app_sha256']
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as ff
new=ff.parse_ufw(fw.read_bytes())['entries']
old=ff.parse_ufw((root/'reference/ETCH-V1.88.fwsc').read_bytes())['entries']
component=lambda entries, name: next(e['data'] for e in entries if e['name']==name)
assert component(new,'flash.bin')[:0x4000]==component(old,'flash.bin')[:0x4000], 'protected bootheader changed'
assert component(new,'ota.bin')==component(old,'ota.bin'), 'trusted OTA loader changed'
assert not ff.verify_fwsc(fw)[1], 'FWSC verification failed'
for name in ['app.bin','app.bin.audit.json','native-build-options.json']:
 target=root/'native-build'/name
 target.parent.mkdir(parents=True,exist_ok=True)
 shutil.copy2(appdir/name,target)
import subprocess
subprocess.run([sys.executable,str(root/'tools/build_site.py')],check=True,cwd=root)
subprocess.run([sys.executable,str(root/'tools/test_site.py')],check=True,cwd=root)
subprocess.run([sys.executable,str(root/'tests/test_website_release.py')],check=True,cwd=root)
# Explicit output files at /mnt/data; no stale-version path inference.
shutil.copy2(fw,base/fw.name)
shutil.copy2(root/'index.html',base/'index.html')
shutil.copy2(root/'index.html',base/f'RUPTURE-V{ver}-INDEX.html')
shutil.copy2(root/'WebInstaller.html',base/f'RUPTURE-WebInstaller-V{ver}.html')
shutil.copy2(root/'CHEAT-SHEET.html',base/f'RUPTURE-CHEAT-SHEET-V{ver}.html')
shutil.copy2(root/'docs/TEST-V1.31.md',base/f'RUPTURE-TEST-RESULTS-V{ver}.md')
files=[
 'index.html','README.md','CHANGELOG.md','CHEAT-SHEET.html','WebInstaller.html',
 fw.name,'RELEASE-MANIFEST.json','WEBSITE-DEPLOY.md','LICENSE',
 'licenses/FELUCCA-LICENSING.txt','assets/boot-animation.gif','assets/boot-final.png','assets/qr-code-original.png',
 'tools/build_site.py','tools/site_template.html','tools/test_site.py','.nojekyll'
]
with ZipFile(base/f'RUPTURE-V{ver}-WEBSITE-GITHUB-PAGES.zip','w',ZIP_DEFLATED,compresslevel=6) as z:
 for name in files:
  p=root/name
  assert p.is_file(),name
  z.write(p,name)
# Export native C screenshots, and preserve the identical boot artwork of V1.20.
for title, images in [
 (f'RUPTURE-V{ver}-53-NATIVE-1BIT-SCREENS.zip',sorted((root/'ui/screens').glob('*.png'))),
 (f'RUPTURE-V{ver}-24-BOOT-FRAMES.zip',sorted((root/'ui/loading-frames').glob('frame-*.png'))+[root/'ui/assets/rupture-splash-final-240.png'])
]:
 assert len(images)>=24
 with ZipFile(base/title,'w',ZIP_DEFLATED,compresslevel=6) as z:
  for p in images: z.write(p,p.relative_to(root).as_posix())
# Complete buildable source and web pages. No old V1.20 firmware binary in this versioned ZIP.
with ZipFile(base/f'RUPTURE-FM1-V{ver}-FULL-SOURCE-FWSC.zip','w',ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(root.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(root).as_posix()
  if any(x in p.parts for x in ('__pycache__','.git','.pytest_cache')):continue
  if rel.startswith('vendor/etch-v1.88-source/build/'):continue
  if any(rel.startswith('RUPTURE-V'+old+'.') or rel.startswith('RUPTURE-V'+old+'-') for old in ('1.20','1.21','1.22','1.23','1.24','1.25','1.26','1.27','1.28','1.29')):continue
  if p.suffix in ('.o','.elf','.pgm','.pyc'):continue
  z.write(p,rel)
print('Validated firmware',fw.name,'sha',meta['firmware_sha256'])
print('Published',base/f'RUPTURE-V{ver}-WEBSITE-GITHUB-PAGES.zip')
print('Published',base/f'RUPTURE-FM1-V{ver}-FULL-SOURCE-FWSC.zip')
