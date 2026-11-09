#!/usr/bin/env python3
"""Produce checked, complete RUPTURE V1.85.2 artifacts."""
from pathlib import Path
import zipfile,hashlib,re,base64,json
src=Path(__file__).resolve().parents[1]
out=Path('/mnt/data')
fw=(src/'RUPTURE-V1.85.2.fwsc').read_bytes()
sh=hashlib.sha256(fw).hexdigest()
files={
 'RUPTURE-V1.85.2.fwsc':src/'RUPTURE-V1.85.2.fwsc',
 'RUPTURE-WebInstaller-V1.85.2.html':src/'WebInstaller.html',
 'RUPTURE-V1.85.2-INDEX.html':src/'index.html',
 'RUPTURE-TEST-RESULTS-V1.85.2.md':src/'RUPTURE-TEST-RESULTS-V1.85.2.md',
 'RUPTURE-CHEAT-SHEET-V1.85.2.html':src/'CHEAT-SHEET.html'
}
for name,p in files.items():
 assert p.is_file(),p
 (out/name).write_bytes(p.read_bytes())

zipweb=out/'RUPTURE-V1.85.2-WEBSITE-GITHUB-PAGES.zip'
base=out/'RUPTURE-V1.85.1-WEBSITE-GITHUB-PAGES.zip'
assert base.is_file()
with zipfile.ZipFile(base) as b, zipfile.ZipFile(zipweb,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=7) as z:
  written=set()
  replace={'index.html','WebInstaller.html','README.md','CHANGELOG.md','CHEAT-SHEET.html','WEBSITE-DEPLOY.md'}
  exclude={'RUPTURE-V1.85.1.fwsc','RUPTURE-TEST-RESULTS-V1.85.1.md'}
  for fname in b.namelist():
   if fname in exclude:continue
   if fname in replace:data=(src/fname).read_bytes()
   else:data=b.read(fname)
   z.writestr(fname,data);written.add(fname)
  for fname,p in [('RUPTURE-V1.85.2.fwsc',src/'RUPTURE-V1.85.2.fwsc'),('RUPTURE-TEST-RESULTS-V1.85.2.md',src/'RUPTURE-TEST-RESULTS-V1.85.2.md')]:
   assert fname not in written
   z.write(p,fname)

sourcezip=out/'RUPTURE-FM1-V1.85.2-FULL-SOURCE-FWSC.zip'
ignore={'native-build-v1852.log','native-build-v1852-repeat.log'}
count=0
with zipfile.ZipFile(sourcezip,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(src.rglob('*')):
  if not p.is_file() or p.name in ignore or '__pycache__' in p.parts:continue
  n=p.relative_to(src).as_posix()
  z.write(p,n);count+=1

for zpath in [zipweb,sourcezip]:
 with zipfile.ZipFile(zpath) as z:
  assert z.testzip() is None,zpath
  assert z.read('RUPTURE-V1.85.2.fwsc')==fw
  assert 'index.html' in z.namelist()
  assert 'vendor/etch-v1.88-source/firmware/fm1-native/hal/fm1_flash_id.h' in z.namelist() if zpath==sourcezip else True
  print('PASS',zpath.name,'entries',len(z.namelist()),'bytes',zpath.stat().st_size)
assert hashlib.sha256((out/'RUPTURE-V1.85.2.fwsc').read_bytes()).hexdigest()==sh
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',(out/'RUPTURE-WebInstaller-V1.85.2.html').read_text(),re.S).group(1))==fw
print('SHA256',sh,'source files',count)
