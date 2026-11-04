#!/usr/bin/env python3
"""Bind the current manifest, native app and unchanged WebMIDI installer to one FWSC.

Replaces only the embedded firmware payload, SHA-256, size and release label.
Does not edit the OTA CHECK/WRITE JavaScript implementation.
"""
from pathlib import Path
import base64,hashlib,json,re,sys
root=Path(__file__).resolve().parents[1]
mp=root/'RELEASE-MANIFEST.json';m=json.loads(mp.read_text())
fw=(root/m['firmware_file']).read_bytes()
app=(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
sha=hashlib.sha256(fw).hexdigest()
assert m['firmware_identity']=='FM-1_968' and len(fw)>500000
m.update(firmware_bytes=len(fw),firmware_sha256=sha,app_bytes=len(app),app_sha256=hashlib.sha256(app).hexdigest())
mp.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n')
p=root/'WebInstaller.html';s=p.read_text()
s,count=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda x:x[1]+base64.b64encode(fw).decode()+x[2],s,count=1,flags=re.S)
assert count==1
s,count=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha}'",s)
assert count==1
s,count=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(fw)}',s)
assert count==1
assert f'V{m["version"]}' in s
p.write_text(s)
print('Synced installer',m['version'],sha,len(fw))
