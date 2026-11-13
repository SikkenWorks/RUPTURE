#!/usr/bin/env python3
"""Publish verified V1.86.1 artifacts and a complete editable source archive."""
from pathlib import Path
import base64,hashlib,json,re,shutil,zipfile
R=Path(__file__).resolve().parents[1]
O=Path('/mnt/data')
V='1.86.1';fwr=R/f'RUPTURE-V{V}.fwsc'
fw=fwr.read_bytes();sha=lambda b:hashlib.sha256(b).hexdigest()
manifest=json.loads((R/'RELEASE-MANIFEST.json').read_text())
assert manifest['version']==V
assert manifest['firmware_sha256']==sha(fw)
assert manifest['firmware_bytes']==len(fw)
for html in ('WebInstaller.html','index.html'):
 src=(R/html).read_text()
 assert 'Stop at the FIRST NUL' in src and 'read-only identity queries' in src
 assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',src,re.S).group(1))==fw
 assert f"const PKG_HASH='{sha(fw)}'" in src
 assert f'const PKG_LEN={len(fw)}' in src
output={
 f'RUPTURE-V{V}.fwsc': f'RUPTURE-V{V}.fwsc',
 f'RUPTURE-WebInstaller-V{V}.html':'WebInstaller.html',
 f'RUPTURE-V{V}-INDEX.html':'index.html',
 f'RUPTURE-TEST-RESULTS-V{V}.md':f'RUPTURE-TEST-RESULTS-V{V}.md',
}
for target,src in output.items():
 shutil.copy2(R/src,O/target)
 assert (O/target).read_bytes()==(R/src).read_bytes()
web=O/f'RUPTURE-V{V}-GITHUB-PAGES.zip'
main=['.nojekyll','index.html','WebInstaller.html','README.md','CHANGELOG.md','CHEAT-SHEET.html','WEBSITE-DEPLOY.md','LICENSE','RELEASE-MANIFEST.json',f'RUPTURE-V{V}.fwsc',f'RUPTURE-TEST-RESULTS-V{V}.md']
main+=sorted(p.relative_to(R).as_posix() for p in (R/'assets').rglob('*') if p.is_file())
with zipfile.ZipFile(web,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 for rel in main:
  assert (R/rel).is_file(),rel
  z.write(R/rel,rel)
preview=O/f'RUPTURE-V{V}-BEND-K3-PREVIEWS.zip'
with zipfile.ZipFile(preview,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 for p in sorted((R/'assets/bend-v1861').glob('*.png')):z.write(p,p.name)
 full=R/'vendor/etch-v1.88-source/firmware/fm1-native/bend_frames'
 for p in sorted(full.glob('*.png')):z.write(p,'mountain-original/'+p.name)
source=O/f'RUPTURE-FM1-V{V}-K3-CURVES-FULL-SOURCE.zip'
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(R.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(R)
  if '__pycache__' in rel.parts or '.pytest_cache' in rel.parts:continue
  if p.suffix in ('.pyc','.log'):continue
  z.write(p,rel.as_posix())
# Cross-check complete ZIP members and component hashes.
for pkg in (web,source,preview):
 with zipfile.ZipFile(pkg) as z:
  assert z.testzip() is None,pkg
  if pkg!=preview:
   assert z.read(f'RUPTURE-V{V}.fwsc')==fw
   assert z.read('WebInstaller.html')==(R/'WebInstaller.html').read_bytes()
   assert z.read('index.html')==(R/'index.html').read_bytes()
  if pkg==source:
   for required in ('vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_curves.h','vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_knob_pixels.h','tests/test_v186_connection_fix.mjs','tests/test_v1861_curve_pcm.c','tests/test_v1861_release.py','reference/RUPTURE-V1.85.2.fwsc'):
    assert required in z.namelist(),required
  print('VERIFIED',pkg.name,len(z.namelist()),'members',pkg.stat().st_size,'bytes')
for n in (*output,web.name,source.name,preview.name):
 path=O/n;assert path.is_file() and path.stat().st_size>0
 print('DOWNLOAD',path.name,path.stat().st_size)
print('FWSC SHA',sha(fw))
