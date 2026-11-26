#!/usr/bin/env python3
"""Package V2.0.62 from audited native builds, preserving trusted boot/OTA."""
from pathlib import Path
import base64, hashlib, json, re, shutil, sys
ROOT=Path(__file__).resolve().parents[1]
V='2.0.62'
N=ROOT/'vendor/etch-v1.88-source'
sys.path.insert(0,str(N/'tools'))
import fm1_fwsc as fw
sha=lambda data:hashlib.sha256(data).hexdigest()
trusted=ROOT/'reference/firmware/RUPTURE-V1.86.1.fwsc'
assert fw.verify_fwsc(trusted)[1]==[]
assets=fw.extract_stock_assets(trusted)
trusted_entries={e['name']:e['data'] for e in fw.parse_ufw(trusted.read_bytes())['entries']}
manifest=json.loads((ROOT/'RELEASE-MANIFEST.json').read_text())
packages=[]
for experimental,subdir,prefix in (
 (False,'fm1-native-storage-updater-uac48-p05',''),
 (True,'fm1-native-storage-updater-uac48-p05-c1fx','cpu1-fx-'),
):
 directory=N/'build'/subdir
 app=(directory/'app.bin').read_bytes()
 opts=json.loads((directory/'native-build-options.json').read_text())
 assert opts['version']==f'RUPTURE V{V}' and opts['app_sha256']==sha(app)
 assert opts['cpu1_full_fx_live_lab_enabled']==experimental
 assert opts['cpu1_audio_offload_enabled']==experimental
 assert opts['storage_writes'] and opts['ota_writes'] and not opts['hardware_verified']
 assert len(app)<=581564 and not fw.validate_app_binary(app)
 assert f'V{V}'.encode() in app
 flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
 assert len(flash)==0x93000
 assert flash[:0x4000]==trusted_entries['flash.bin'][:0x4000]
 pkg=fw.build_fwsc(flash,assets['ota'],'FM-1_968')
 filename=f'RUPTURE-V{V}-'+('CORE2-EXPERIMENTAL' if experimental else 'STABLE')+'.fwsc'
 (ROOT/filename).write_bytes(pkg)
 assert fw.verify_fwsc(ROOT/filename)[1]==[]
 entries={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
 assert entries['ota.bin']==trusted_entries['ota.bin']
 assert entries['flash.bin']==flash
 for source,dest in (
  ('app.bin',prefix+'app.bin'),('app.bin.audit.json',prefix+'app.bin.audit.json'),
  ('native-build-options.json',prefix+'build-options.json' if experimental else 'native-build-options.json'),
 ):
  shutil.copy2(directory/source,ROOT/'native-build'/dest)
 key='experimental_' if experimental else ''
 manifest.update({key+'firmware_file':filename,key+'firmware_sha256':sha(pkg),key+'firmware_bytes':len(pkg),key+'app_sha256':sha(app),key+'app_bytes':len(app)})
 packages.append(pkg)
assert packages[0]!=packages[1]
manifest.update(
 version=V,
 release_feature='New startup animation',
 release_note='Intro1, then Intro2, then RUPTURE or first-run setup.',
 physical_fm1_verified=False,hardware_verified=False,release_ready=False,
 release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',
 boot_intro={'intro1_frames':8,'intro1_frame_ms':100,'intro2_frames':135,'intro2_frame_ms':34,'duration_ms':5390},
)
(ROOT/'RELEASE-MANIFEST.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
for filename in ('WebInstaller.html','index.html'):
 path=ROOT/filename;old=path.read_text();new=old.replace('V2.0.61',f'V{V}')
 # Require already embedded trusted boot/OTA before replacing either payload.
 for tag,pkg,prefix in (('firmware',packages[0],''),('firmware-experimental',packages[1],'EXP_')):
  pattern=r'(<script id="'+tag+r'"[^>]*>)(.*?)(</script>)'
  match=re.search(pattern,old,re.S);assert match
  previous=base64.b64decode(match.group(2))
  previous_entries={e['name']:e['data'] for e in fw.parse_ufw(previous)['entries']}
  assert previous_entries['ota.bin']==assets['ota']
  assert previous_entries['flash.bin'][:0x4000]==trusted_entries['flash.bin'][:0x4000]
  new,count=re.subn(pattern,lambda m:m.group(1)+base64.b64encode(pkg).decode()+m.group(3),new,count=1,flags=re.S);assert count==1
  new,count=re.subn(r"const "+prefix+r"PKG_HASH='[0-9a-f]{64}'",f"const {prefix}PKG_HASH='{sha(pkg)}'",new);assert count==1
  new,count=re.subn(r'const '+prefix+r'PKG_LEN=\d+',f'const {prefix}PKG_LEN={len(pkg)}',new);assert count==1
 if filename=='index.html':
  for tag,source in (('fallback-readme','README.md'),('fallback-changelog','CHANGELOG.md')):
   pattern=r'(<script id="'+tag+r'"[^>]*>)(.*?)(</script>)'
   new,count=re.subn(pattern,lambda m:m.group(1)+base64.b64encode((ROOT/source).read_bytes()).decode()+m.group(3),new,count=1,flags=re.S);assert count==1
  pattern=r'(<script id="site-release-meta"[^>]*>)(.*?)(</script>)'
  def meta(m):
   data=json.loads(m.group(2));data.update(version=V,sha256=sha(packages[0]),bytes=len(packages[0]),experimental_sha256=sha(packages[1]),experimental_bytes=len(packages[1]))
   return m.group(1)+json.dumps(data)+m.group(3)
  new,count=re.subn(pattern,meta,new,count=1,flags=re.S);assert count==1
 # Prove all executable installer JavaScript remains byte-identical except
 # for the package length/hash constants. Inline base64/doc/metadata scripts
 # are compared independently by test_v260_release.py.
 def executable(html):
  scripts=[]
  for attrs,body in re.findall(r'<script\b([^>]*)>(.*?)</script>',html,re.S):
   if re.search(r'type=["\'](?:application/octet-stream|application/json|text/plain)',attrs):continue
   body=re.sub(r"const (EXP_)?PKG_HASH='[0-9a-f]{64}'",r"const \1PKG_HASH='HASH'",body)
   body=re.sub(r'const (EXP_)?PKG_LEN=\d+',r'const \1PKG_LEN=0',body)
   scripts.append((attrs,body.replace('V2.0.61','VERSION').replace('V'+V,'VERSION')))
  return scripts
 assert executable(old)==executable(new),filename+' executable installer logic changed'
 path.write_text(new)
print('PASS V'+V+': both native apps/FWSCs, protected boot/OTA and unchanged executable installer logic')
