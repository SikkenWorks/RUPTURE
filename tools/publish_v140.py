#!/usr/bin/env python3
"""Export full RUPTURE V1.40 source, FWSC, and GitHub Pages site."""
from pathlib import Path
import zipfile,shutil,hashlib,json
r=Path(__file__).resolve().parents[1]; dest=Path('/mnt/data')
meta=json.loads((r/'RELEASE-MANIFEST.json').read_text());assert meta['version']=='1.40'
fw=r/meta['firmware_file'];assert fw.is_file() and hashlib.sha256(fw.read_bytes()).hexdigest()==meta['firmware_sha256']
for src,out in [(fw,fw.name),(r/'index.html','index.html'),(r/'WebInstaller.html','RUPTURE-WebInstaller-V1.40.html'),(r/'docs/TEST-V1.40.md','RUPTURE-TEST-RESULTS-V1.40.md')]:
 shutil.copy2(src,dest/out)
site_names=['index.html','WebInstaller.html','README.md','CHANGELOG.md','CHEAT-SHEET.html','WEBSITE-DEPLOY.md','RELEASE-MANIFEST.json',fw.name,'.nojekyll','LICENSE']
site_names += [str(p.relative_to(r)) for p in (r/'assets').glob('*') if p.is_file()]
site_names += [str(p.relative_to(r)) for p in (r/'licenses').glob('*') if p.is_file()]
with zipfile.ZipFile(dest/'RUPTURE-V1.40-WEBSITE-GITHUB-PAGES.zip','w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for name in sorted(set(site_names)):
  p=r/name;assert p.is_file(),name;z.write(p,name)
with zipfile.ZipFile(dest/'RUPTURE-FM1-V1.40-FULL-SOURCE-FWSC.zip','w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(r.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(r).as_posix()
  if any(c in p.parts for c in ('__pycache__','.git','.pytest_cache')):continue
  if rel.startswith('vendor/etch-v1.88-source/build/'):continue
  if p.suffix in ('.o','.elf','.pgm','.pyc'):continue
  if rel.startswith('RUPTURE-V1.3') and rel.endswith('.fwsc'):continue
  z.write(p,rel)
for fn in [dest/'RUPTURE-V1.40-WEBSITE-GITHUB-PAGES.zip',dest/'RUPTURE-FM1-V1.40-FULL-SOURCE-FWSC.zip']:
 with zipfile.ZipFile(fn) as z:
  assert z.testzip() is None
  print(fn,fn.stat().st_size,len(z.infolist()))
print('V1.40',fw.name,'sha256',meta['firmware_sha256'])
