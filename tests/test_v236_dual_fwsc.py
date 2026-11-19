#!/usr/bin/env python3
"""Release contract: two independently installable, distinguishable FWSCs."""
from pathlib import Path
import base64
import hashlib
import json
import re
import sys

root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw

manifest=json.loads((root/'RELEASE-MANIFEST.json').read_text())
sha=lambda b:hashlib.sha256(b).hexdigest()
assert manifest['version']=='2.0.36'
apps=[]
entries=[]
for filename,appname,buildoptions in (
 ('RUPTURE-V2.0.36-STABLE.fwsc','app.bin','native-build-options.json'),
 ('RUPTURE-V2.0.36-CORE2-EXPERIMENTAL.fwsc','cpu1-live-uac-app.bin','cpu1-live-uac-build-options.json'),
):
 path=root/filename
 assert path.is_file()
 assert fw.verify_fwsc(path)[1]==[]
 fwsc=fw.parse_ufw(path.read_bytes())
 image={e['name']:e['data'] for e in fwsc['entries']}
 entries.append(image)
 opts=json.loads((root/'native-build'/buildoptions).read_text())
 assert opts['storage_writes'] and opts['ota_writes']
 assert opts['version']=='RUPTURE V2.0.36'
 app=(root/'native-build'/appname).read_bytes()
 assert sha(app)==opts['app_sha256']
 assert not fw.validate_app_binary(app)
 apps.append(app)

assert apps[0]!=apps[1]
assert not json.loads((root/'native-build/native-build-options.json').read_text())['cpu1_audio_offload_enabled']
assert json.loads((root/'native-build/cpu1-live-uac-build-options.json').read_text())['cpu1_audio_offload_enabled']
assert entries[0]['ota.bin']==entries[1]['ota.bin']
assert entries[0]['flash.bin'][:0x4000]==entries[1]['flash.bin'][:0x4000]
key=fw.extract_stock_assets(root/'reference/firmware/RUPTURE-V1.86.1.fwsc')['key']
for app,entry in zip(apps,entries):
 segment=bytearray(entry['flash.bin'][0x4000:0x4000+0x120+len(app)])
 fw.sfc_xor(segment,0,len(segment),0,key)
 assert segment[0x120:]==app
for installer in ('index.html','WebInstaller.html'):
 html=(root/installer).read_text()
 m=re.search(r'<script id="firmware"[^>]*>(.*?)</script>',html,re.S)
 assert m and base64.b64decode(m.group(1))==(root/'RUPTURE-V2.0.36-STABLE.fwsc').read_bytes()
 e=re.search(r'<script id="firmware-experimental"[^>]*>(.*?)</script>',html,re.S)
 assert e and base64.b64decode(e.group(1))==(root/'RUPTURE-V2.0.36-CORE2-EXPERIMENTAL.fwsc').read_bytes()
 assert '<option value="stable" selected>' in html and '<option value="core2">' in html

print('PASS V2.0.36: two distinct FWSCs, exact apps, original OTA/boot and 2-option installers')
