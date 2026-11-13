#!/usr/bin/env python3
"""Publish V1.88 FM1 package, standalone installer, site, full source and 5x8 BEND sprites."""
from pathlib import Path
import base64,hashlib,json,re,shutil,zipfile,sys
R=Path(__file__).resolve().parents[1]
O=Path('/mnt/data')
V='1.88'
fw=(R/f'RUPTURE-V{V}.fwsc').read_bytes()
sha=lambda b:hashlib.sha256(b).hexdigest()
m=json.loads((R/'RELEASE-MANIFEST.json').read_text())
assert m['version']==V
assert m['firmware_sha256']==sha(fw)
assert m['firmware_bytes']==len(fw)
assert 'V1.88' in (R/'CHANGELOG.md').read_text()[:100]
for name in ('WebInstaller.html','index.html'):
 html=(R/name).read_text()
 assert 'Stop at the FIRST NUL' in html and 'read-only identity queries' in html
 assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',html,re.S).group(1))==fw
 assert f"const PKG_HASH='{sha(fw)}'" in html
 assert f'const PKG_LEN={len(fw)}' in html
 assert 'V1.88' in html

output={
 f'RUPTURE-V{V}.fwsc':f'RUPTURE-V{V}.fwsc',
 f'RUPTURE-WebInstaller-V{V}.html':'WebInstaller.html',
 f'RUPTURE-V{V}-INDEX.html':'index.html',
 f'RUPTURE-TEST-RESULTS-V{V}.md':f'RUPTURE-TEST-RESULTS-V{V}.md',
 f'RUPTURE-CHEAT-SHEET-V{V}.html':'CHEAT-SHEET.html',
}
for name,src in output.items():
 shutil.copy2(R/src,O/name)
 assert (O/name).read_bytes()==(R/src).read_bytes()

# GitHub Pages site: self-contained public distribution with the new FWSC and previews.
web=O/f'RUPTURE-V{V}-GITHUB-PAGES.zip'
site=['.nojekyll','index.html','WebInstaller.html','README.md','CHANGELOG.md','CHEAT-SHEET.html','WEBSITE-DEPLOY.md','LICENSE','RELEASE-MANIFEST.json',f'RUPTURE-V{V}.fwsc',f'RUPTURE-TEST-RESULTS-V{V}.md']
site+=sorted(p.relative_to(R).as_posix() for p in (R/'assets').rglob('*') if p.is_file())
with zipfile.ZipFile(web,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 for name in site:
  assert (R/name).is_file(),name
  z.write(R/name,name)

# Full editable source + binary, docs, native firmware and tests.
full=O/f'RUPTURE-FM1-V{V}-FULL-SOURCE-FWSC.zip'
with zipfile.ZipFile(full,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(R.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(R)
  if '__pycache__' in rel.parts or '.pytest_cache' in rel.parts:continue
  if p.suffix in ('.pyc','.log'):continue
  z.write(p,rel.as_posix())

# All five original 8-frame strips + 80 native screen renders, including CRT ON/OFF.
prev=O/f'RUPTURE-V{V}-BEND-ANIMATIONS-SPRITES.zip'
with zipfile.ZipFile(prev,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 a=R/'vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188'
 for p in sorted(a.rglob('*.png')):
  z.write(p, 'BEND_SPRITES_224x30/'+p.relative_to(a).as_posix())
 a=R/'assets/bend-v188-native-ui'
 for p in sorted(a.rglob('*.png')):
  z.write(p, 'NATIVE_UI_240x240/'+p.relative_to(a).as_posix())
 for x in ('RUPTURE-TEST-RESULTS-V1.88.md',):
  z.write(R/x,x)

# Verify final files are linked to the exact same release, ZIPs are intact.
for pkg in (web,full,prev):
 with zipfile.ZipFile(pkg) as z:
  assert z.testzip() is None,pkg
  if pkg!=prev:
   assert z.read(f'RUPTURE-V{V}.fwsc')==fw
   assert z.read('WebInstaller.html')==(R/'WebInstaller.html').read_bytes()
   assert z.read('index.html')==(R/'index.html').read_bytes()
  if pkg==full:
   for x in ['native-build/app.bin','vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_sprites.h','vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_curves.h','tools/package_v187_release.py','tests/test_v186_connection_fix.mjs']:
    assert x in z.namelist(),x
   assert z.read('native-build/app.bin')==(R/'native-build/app.bin').read_bytes()
  if pkg==prev:
   assert len([x for x in z.namelist() if x.startswith('BEND_SPRITES_224x30/') and '/frame_' in x and x.endswith('.png')])==40
  print('VERIFIED',pkg.name,'bytes',pkg.stat().st_size,'files',len(z.namelist()))
for x in [*output,web.name,full.name,prev.name]:
 p=O/x
 assert p.is_file() and p.stat().st_size>0
 print('DOWNLOAD',p.name,p.stat().st_size)
print('FWSC SHA256',sha(fw))
