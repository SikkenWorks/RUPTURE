from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,shutil
root=Path(__file__).resolve().parents[1]
out=Path('/mnt/data')
version='1.80'
manifest=json.loads((root/'RELEASE-MANIFEST.json').read_text())
assert manifest['version']==version
assets=['.nojekyll','LICENSE','README.md','CHANGELOG.md','CHEAT-SHEET.html','RELEASE-MANIFEST.json','WEBSITE-DEPLOY.md','index.html','WebInstaller.html','RUPTURE-V1.80.fwsc']
with ZipFile(out/'RUPTURE-V1.80-WEBSITE-GITHUB-PAGES.zip','w',ZIP_DEFLATED,compresslevel=7) as z:
 for n in assets:z.write(root/n,n)
 for dr in ['assets','licenses']:
  for p in sorted((root/dr).rglob('*')):
   if p.is_file():z.write(p,p.relative_to(root))
 for p in sorted((root/'docs').glob('TEST-V1.80.md')):z.write(p,p.relative_to(root))
with ZipFile(out/'RUPTURE-V1.80-REVERB-DEMO-WAV.zip','w',ZIP_DEFLATED,compresslevel=7) as z:
 for p in sorted((root/'demos').glob('*.wav')):z.write(p,p.relative_to(root))
with ZipFile(out/'RUPTURE-FM1-V1.80-FULL-SOURCE-FWSC.zip','w',ZIP_DEFLATED,compresslevel=7) as z:
 for p in sorted(root.rglob('*')):
  if not p.is_file() or '__pycache__' in p.parts or p.suffix in ('.pyc','.log') or 'build/.cache' in str(p):continue
  z.write(p,p.relative_to(root))
for inname,outname in [('RUPTURE-V1.80.fwsc','RUPTURE-V1.80.fwsc'),('WebInstaller.html','RUPTURE-WebInstaller-V1.80.html'),('index.html','RUPTURE-V1.80-INDEX.html'),('CHEAT-SHEET.html','RUPTURE-CHEAT-SHEET-V1.80.html'),('docs/TEST-V1.80.md','RUPTURE-TEST-RESULTS-V1.80.md')]:
 shutil.copy2(root/inname,out/outname)
for p in sorted(out.glob('*V1.80*')):
 if p.is_file():print(p.name,p.stat().st_size,hashlib.sha256(p.read_bytes()).hexdigest()[:12])
for filename in ['RUPTURE-V1.80-WEBSITE-GITHUB-PAGES.zip','RUPTURE-V1.80-REVERB-DEMO-WAV.zip','RUPTURE-FM1-V1.80-FULL-SOURCE-FWSC.zip']:
 with ZipFile(out/filename) as z:
  bad=z.testzip();assert not bad,(filename,bad)
  if 'FULL-SOURCE' in filename:
   names=set(z.namelist())
   for required in ['vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c','native-build/app.bin','RUPTURE-V1.80.fwsc','WebInstaller.html','README.md','CHANGELOG.md','tests/test_v180_reverb_network.c','demos/SPRING-V1.80-100wet.wav','docs/TEST-V1.80.md']:
    assert required in names,(filename,required)
  print('verified',filename,len(z.namelist()),'files')
