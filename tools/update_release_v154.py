#!/usr/bin/env python3
"""Update firmware embedding from the byte-verified .fwsc without changing the WebMIDI OTA implementation."""
import base64, hashlib, json, re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
manifest=root/'RELEASE-MANIFEST.json'
m=json.loads(manifest.read_text())
fw=(root/'RUPTURE-V1.54.fwsc').read_bytes()
app=(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
m.update({'version':'1.54','firmware_file':'RUPTURE-V1.54.fwsc','firmware_bytes':len(fw),'firmware_sha256':hashlib.sha256(fw).hexdigest(),'app_bytes':len(app),'app_sha256':hashlib.sha256(app).hexdigest()})
manifest.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n')
installer=root/'WebInstaller.html'
s=installer.read_text()
pattern=r'(<script id="firmware"[^>]*>).*?(</script>)'
s,n=re.subn(pattern,lambda x:x.group(1)+base64.b64encode(fw).decode()+x.group(2),s,count=1,flags=re.S)
assert n==1
assert s.count('V1.53')>=2
s=s.replace('V1.53','V1.54')
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{m['firmware_sha256']}'",s)
assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f"const PKG_LEN={len(fw)}",s)
assert n==1
installer.write_text(s)
# Update template, not just generated output, to persist the previously approved nav links.
p=root/'tools/site_template.html'
s=p.read_text()
old='<a href="#readme">README</a><a href="#changelog">CHANGELOG</a>'
new='<a href="#readme">README/CHANGELOG</a><a href="https://fm1.sikkenworks.com/FM1-Recovery-by-SikkenWorks.exe">FM1 RECOVERY WINDOWS</a>'
assert s.count(old)==1
s=s.replace(old,new)
p.write_text(s)
print('V1.54 manifest, trusted WebInstaller firmware embedding and persistent nav updated')
