#!/usr/bin/env python3
"""Publish the V1.86 website/WebInstaller connection-only hotfix with unchanged FWSC."""
from pathlib import Path
import hashlib,base64,json,re,zipfile,shutil
root=Path(__file__).resolve().parents[1]
out=Path('/mnt/data')
manifest=json.loads((root/'RELEASE-MANIFEST.json').read_text())
assert manifest['version']=='1.86'
fw=(root/'RUPTURE-V1.86.fwsc').read_bytes()
sha=hashlib.sha256(fw).hexdigest()
assert sha==manifest['firmware_sha256']=='239edee0056b32896f6f293cab22a05e366b94928ed76d2ebd4487047a45ccd8'
files={
 'RUPTURE-V1.86-CONNECTION-FIX-WebInstaller.html':'WebInstaller.html',
 'RUPTURE-V1.86-CONNECTION-FIX-INDEX.html':'index.html',
 'RUPTURE-V1.86-CONNECTION-FIX-REPORT.md':'RUPTURE-V1.86-CONNECTION-FIX-REPORT.md',
}
for published,src in files.items():
 shutil.copy2(root/src,out/published)
 assert (out/published).read_bytes()==(root/src).read_bytes()
website=out/'RUPTURE-V1.86-CONNECTION-FIX-GITHUB-PAGES.zip'
site_files=['.nojekyll','index.html','WebInstaller.html','README.md','CHANGELOG.md','CHEAT-SHEET.html',
 'WEBSITE-DEPLOY.md','LICENSE','RELEASE-MANIFEST.json','RUPTURE-V1.86.fwsc',
 'RUPTURE-V1.86-CONNECTION-FIX-REPORT.md','RUPTURE-TEST-RESULTS-V1.86.md']
site_files += sorted(p.relative_to(root).as_posix() for p in (root/'assets').rglob('*') if p.is_file())
with zipfile.ZipFile(website,'w',zipfile.ZIP_DEFLATED,compresslevel=7) as z:
 for rel in site_files:
  assert (root/rel).is_file(),rel
  z.write(root/rel,rel)
source=out/'RUPTURE-FM1-V1.86-CONNECTION-FIX-FULL-SOURCE.zip'
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(root.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(root)
  if '__pycache__' in rel.parts or '.pytest_cache' in rel.parts:continue
  if p.name.endswith(('.pyc','.log')):continue
  z.write(p,rel.as_posix())
for pkg in (website,source):
 with zipfile.ZipFile(pkg) as z:
  assert z.testzip() is None
  assert z.read('RUPTURE-V1.86.fwsc')==fw
  assert z.read('index.html')==(root/'index.html').read_bytes()
  assert z.read('WebInstaller.html')==(root/'WebInstaller.html').read_bytes()
  assert 'tests/test_v186_connection_fix.mjs' in z.namelist() if pkg==source else True
  print('VERIFIED',pkg.name,len(z.namelist()),'items',pkg.stat().st_size,'bytes')
for src in ('WebInstaller.html','index.html'):
 data=(root/src).read_text()
 assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',data,re.S).group(1))==fw
 assert data.count('Stop at the FIRST NUL')==1
 assert 'checking '+"'" in data
print('VERIFIED FWSC unchanged SHA-256',sha)
for rel in files:
 assert (out/rel).is_file()
 print('VERIFIED',rel,(out/rel).stat().st_size)
