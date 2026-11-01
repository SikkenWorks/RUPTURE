#!/usr/bin/env python3
"""RUPTURE V1.41: validate and publish firmware, site and full source."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json, shutil
r=Path(__file__).resolve().parents[1]
out=Path('/mnt/data')
m=json.loads((r/'RELEASE-MANIFEST.json').read_text());assert m['version']=='1.41'
fw=r/m['firmware_file'];assert fw.is_file() and len(fw.read_bytes())==m['firmware_bytes']
assert hashlib.sha256(fw.read_bytes()).hexdigest()==m['firmware_sha256']
import sys
sys.path.insert(0,str(r/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc
assert not fm1_fwsc.verify_fwsc(fw)[1]
new={e['name']:e['data'] for e in fm1_fwsc.parse_ufw(fw.read_bytes())['entries']}
old={e['name']:e['data'] for e in fm1_fwsc.parse_ufw((r/'RUPTURE-V1.40.fwsc').read_bytes())['entries']}
assert new['flash.bin'][:0x4000]==old['flash.bin'][:0x4000] and new['ota.bin']==old['ota.bin']
for src,name in [(fw,fw.name),(r/'index.html','index.html'),(r/'WebInstaller.html','RUPTURE-WebInstaller-V1.41.html'),(r/'docs/TEST-V1.41.md','RUPTURE-TEST-RESULTS-V1.41.md')]:
 shutil.copy2(src,out/name)
website=['index.html','WebInstaller.html','README.md','CHANGELOG.md','CHEAT-SHEET.html','WEBSITE-DEPLOY.md','RELEASE-MANIFEST.json','RUPTURE-V1.41.fwsc','.nojekyll','LICENSE']
website += [str(p.relative_to(r)) for p in (r/'assets').glob('*') if p.is_file()]
website += [str(p.relative_to(r)) for p in (r/'licenses').glob('*') if p.is_file()]
with ZipFile(out/'RUPTURE-V1.41-WEBSITE-GITHUB-PAGES.zip','w',ZIP_DEFLATED,compresslevel=6) as z:
 for rel in sorted(set(website)):
  assert (r/rel).is_file(),rel
  z.write(r/rel,rel)
with ZipFile(out/'RUPTURE-FM1-V1.41-FULL-SOURCE-FWSC.zip','w',ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(r.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(r).as_posix()
  if any(k in p.parts for k in ('__pycache__','.git','.pytest_cache')):continue
  if rel.startswith('vendor/etch-v1.88-source/build/'):continue
  if p.suffix in ('.o','.elf','.pgm','.pyc'):continue
  if rel.startswith('RUPTURE-V1.') and rel.endswith('.fwsc') and rel!=fw.name:continue
  z.write(p,rel)
for name in ('RUPTURE-V1.41-WEBSITE-GITHUB-PAGES.zip','RUPTURE-FM1-V1.41-FULL-SOURCE-FWSC.zip'):
 with ZipFile(out/name) as z:
  assert z.testzip() is None
  print('PASS',name,'entries',len(z.namelist()),'bytes',(out/name).stat().st_size)
print('PASS FM1 V1.41 full package and HTML',m['firmware_sha256'])
