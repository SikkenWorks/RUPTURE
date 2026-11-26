#!/usr/bin/env python3
"""Verify complete current containers, exact app embedding, installer and source preservation."""
from pathlib import Path
import base64,hashlib,json,re,sys
R=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
sha=lambda data:hashlib.sha256(data).hexdigest()
m=json.loads((R/'RELEASE-MANIFEST.json').read_text())
assert m['version']=='2.0.61'
assert not m['hardware_verified'] and not m['release_ready']
assert not m['storage_audio_v260']['poweroff_pop_fixed']
trusted=R/'reference/firmware/RUPTURE-V1.86.1.fwsc'
assets=fw.extract_stock_assets(trusted)
t={e['name']:e['data'] for e in fw.parse_ufw(trusted.read_bytes())['entries']}
packages=[]
for prefix,apppath,optpath,experimental in (
 ('','app.bin','native-build-options.json',False),
 ('experimental_','cpu1-fx-app.bin','cpu1-fx-build-options.json',True),
):
 name=m[prefix+'firmware_file'];pkg=(R/name).read_bytes()
 assert name=='RUPTURE-V2.0.61-'+('CORE2-EXPERIMENTAL' if experimental else 'STABLE')+'.fwsc'
 assert fw.verify_fwsc(R/name)[1]==[]
 assert len(pkg)==m[prefix+'firmware_bytes'] and sha(pkg)==m[prefix+'firmware_sha256']
 app=(R/'native-build'/apppath).read_bytes()
 assert len(app)==m[prefix+'app_bytes'] and sha(app)==m[prefix+'app_sha256']
 opts=json.loads((R/'native-build'/optpath).read_text())
 assert opts['app_sha256']==sha(app) and opts['version']=='RUPTURE V2.0.61'
 assert opts['cpu1_full_fx_live_lab_enabled']==experimental
 assert opts['storage_writes'] and opts['ota_writes'] and not opts['hardware_verified']
 assert not fw.validate_app_binary(app) and b'V2.0.61' in app
 entries={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
 assert entries['ota.bin']==t['ota.bin']
 assert entries['flash.bin'][:0x4000]==t['flash.bin'][:0x4000]
 segment=bytearray(entries['flash.bin'][0x4000:0x4120+len(app)])
 fw.sfc_xor(segment,0,len(segment),0,assets['key'])
 assert segment[0x120:]==app
 packages.append(pkg)
assert packages[0]!=packages[1]
for installer in ('index.html','WebInstaller.html'):
 html=(R/installer).read_text()
 for tag,pkg,prefix in (('firmware',packages[0],''),('firmware-experimental',packages[1],'EXP_')):
  embedded=base64.b64decode(re.search(r'<script id="'+tag+r'"[^>]*>(.*?)</script>',html,re.S).group(1))
  assert embedded==pkg
  assert re.search(r"const "+prefix+r"PKG_HASH='([^']+)'",html).group(1)==sha(pkg)
  assert int(re.search(r'const '+prefix+r'PKG_LEN=(\d+)',html).group(1))==len(pkg)
 assert '<option value="stable" selected>' in html and 'id="experimental-ack"' in html
 if installer=='index.html':
  for tag,path in (('fallback-readme','README.md'),('fallback-changelog','CHANGELOG.md')):
   embedded=base64.b64decode(re.search(r'<script id="'+tag+r'"[^>]*>(.*?)</script>',html,re.S).group(1))
   assert embedded==(R/path).read_bytes()
  meta=json.loads(re.search(r'<script id="site-release-meta"[^>]*>(.*?)</script>',html,re.S).group(1))
  assert meta['version']==m['version'] and meta['sha256']==sha(packages[0])
checks=json.loads((R/'tests/v260_preserved_sources.json').read_text())
for path,wanted in checks.items():assert sha((R/path).read_bytes())==wanted,path
print('PASS V2.0.61: 2 verified FWSCs, exact native apps, trusted OTA/header, exact offline embeds/hashes/docs,',len(checks),'unchanged DSP/storage/HAL sources; physical/power-off claims remain unverified')
