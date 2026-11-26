#!/usr/bin/env python3
"""Clean full-source V2.0.60 export; exactly two current root firmwares."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,re
R=Path(__file__).resolve().parents[1]
M=json.loads((R/'RELEASE-MANIFEST.json').read_text())
assert M['version']=='2.0.60'
firmware={M['firmware_file'],M['experimental_firmware_file']}
for prefix in ('','experimental_'):
 p=R/M[prefix+'firmware_file']
 assert len(p.read_bytes())==M[prefix+'firmware_bytes']
 assert hashlib.sha256(p.read_bytes()).hexdigest()==M[prefix+'firmware_sha256']
def include(p):
 rel=p.relative_to(R)
 if any(c in rel.parts for c in ('.git','__pycache__','.pytest_cache','.mypy_cache')):return False
 if p.suffix.lower() in ('.pyc','.o','.elf','.pgm','.tmp','.bak'):return False
 if rel.parts[:3]==('vendor','etch-v1.88-source','build'):return False
 if len(rel.parts)==1 and p.suffix.lower()=='.fwsc':return p.name in firmware
 if rel.parts[0]=='native-build' and p.name.startswith('cpu1-lab-'):return False
 if rel.parts[0]=='tools' and re.fullmatch(r'package_v\d+(?:_full_zip)?\.py',p.name):
  return p.name in ('package_v260.py','package_v260_full_zip.py')
 return True
files=[p for p in sorted(R.rglob('*')) if p.is_file() and include(p)]
names={p.relative_to(R).as_posix() for p in files}
for required in (
 *firmware,'README.md','CHANGELOG.md','RELEASE-MANIFEST.json',
 'WebInstaller.html','index.html','docs/AUDIO-OUTPUT-AUDIT-V2.0.60.md',
 'docs/TEST-V2.0.60.md','docs/VALIDATION-V2.0.60.txt',
 'tests/test_v260_storage_audio.py','tests/test_v260_release.py','tests/run_v260_checks.py',
 'tests/v260_preserved_sources.json','tools/package_v260.py','tools/package_v260_full_zip.py',
 'vendor/etch-v1.88-source/firmware/fm1-native/native_storage_audio.h',
 'native-build/app.bin','native-build/cpu1-fx-app.bin',
):assert required in names,required
assert {n for n in names if '/' not in n and n.endswith('.fwsc')}==firmware
assert all(not n.startswith('vendor/etch-v1.88-source/build/') for n in names)
out=R.parents[1]/'deliverables/RUPTURE-FM1-V2.0.60-CLEAN-FULL-PROJECT.zip'
out.parent.mkdir(exist_ok=True,parents=True)
prefix='RUPTURE-FM1-V2.0.60/'
with ZipFile(out,'w',ZIP_DEFLATED,compresslevel=6) as z:
 for p in files:z.write(p,prefix+p.relative_to(R).as_posix())
with ZipFile(out) as z:
 assert z.testzip() is None
 assert {n[len(prefix):] for n in z.namelist()}==names
print('PASS CLEAN full-source ZIP:',out,'files=',len(files),'bytes=',out.stat().st_size)
