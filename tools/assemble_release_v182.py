from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,shutil,json,sys
root=Path(__file__).resolve().parents[1];base=Path('/mnt/data')
version='1.82';manifest=json.loads((root/'RELEASE-MANIFEST.json').read_text());assert manifest['version']==version
fw=root/f'RUPTURE-V{version}.fwsc';assert fw.exists()
assert hashlib.sha256(fw.read_bytes()).hexdigest()==manifest['firmware_sha256']
appdir=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05'
assert hashlib.sha256((appdir/'app.bin').read_bytes()).hexdigest()==manifest['app_sha256']
# Physically pinned boot/loader from original V1.80 package.
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as f
v180={e['name']:e['data'] for e in f.parse_ufw((root/'RUPTURE-V1.81.fwsc').read_bytes())['entries']}
v182={e['name']:e['data'] for e in f.parse_ufw(fw.read_bytes())['entries']}
assert v180['flash.bin'][:0x4000]==v182['flash.bin'][:0x4000]
assert v180['ota.bin']==v182['ota.bin']
assert not f.verify_fwsc(fw)[1]
for name in ['app.bin','app.bin.audit.json','native-build-options.json']:
 shutil.copy2(appdir/name,root/'native-build'/name)
# Files promoted for direct download.
for src,dest in [(fw,fw.name),(root/'WebInstaller.html',f'RUPTURE-WebInstaller-V{version}.html'),(root/'index.html',f'RUPTURE-V{version}-INDEX.html'),(root/'CHEAT-SHEET.html',f'RUPTURE-CHEAT-SHEET-V{version}.html'),(root/f'docs/TEST-V{version}.md',f'RUPTURE-TEST-RESULTS-V{version}.md')]:shutil.copy2(src,base/dest)
# GitHub Pages package with native website, installer and supporting docs.
with ZipFile(base/f'RUPTURE-V{version}-WEBSITE-GITHUB-PAGES.zip','w',ZIP_DEFLATED,compresslevel=6) as z:
 for n in ['.nojekyll','LICENSE','README.md','CHANGELOG.md','CHEAT-SHEET.html','RELEASE-MANIFEST.json','WEBSITE-DEPLOY.md','index.html','WebInstaller.html',fw.name]:z.write(root/n,n)
 for directory in ['assets','licenses']:
  for p in sorted((root/directory).rglob('*')):
   if p.is_file():z.write(p,p.relative_to(root))
 z.write(root/f'docs/TEST-V{version}.md',f'docs/TEST-V{version}.md')
# Full project, but not build intermediates or redundant nested old releases.
with ZipFile(base/f'RUPTURE-FM1-V{version}-FULL-SOURCE-FWSC.zip','w',ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(root.rglob('*')):
  if not p.is_file() or any(x in p.parts for x in ('__pycache__','.git','.pytest_cache')):continue
  rel=p.relative_to(root).as_posix()
  if rel.startswith('vendor/etch-v1.88-source/build/'):continue
  if p.suffix in ('.o','.elf','.pyc','.pgm','.log'):continue
  z.write(p,rel)
for name in [f'RUPTURE-FM1-V{version}-FULL-SOURCE-FWSC.zip',f'RUPTURE-V{version}-WEBSITE-GITHUB-PAGES.zip']:
 p=base/name
 with ZipFile(p) as z:
  assert not z.testzip();names=set(z.namelist());assert fw.name in names
  assert f'docs/TEST-V{version}.md' in names
  assert 'WebInstaller.html' in names
  if 'FULL-SOURCE' in name:
   for required in ['vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c','vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_paint_budget.h','native-build/app.bin','tests/test_v181_full_audio_parity.py']:
    assert required in names,(name,required)
 print('VERIFIED',p.name,p.stat().st_size,'bytes',len(names),'files',hashlib.sha256(p.read_bytes()).hexdigest())
print('FWSC',fw.name,len(fw.read_bytes()),manifest['firmware_sha256'])
