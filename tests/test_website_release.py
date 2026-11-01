#!/usr/bin/env python3
"""Release-aware smoke checks for public index.html, docs and firmware parity."""
import base64, hashlib, json, re
from pathlib import Path
root=Path(__file__).resolve().parent.parent
meta=json.loads((root/'RELEASE-MANIFEST.json').read_text())
index=(root/'index.html').read_text()
fw=(root/meta['firmware_file']).read_bytes()
assert hashlib.sha256(fw).hexdigest()==meta['firmware_sha256']
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',index,re.S).group(1)) == fw
assert f"const PKG_HASH='{meta['firmware_sha256']}'" in index
assert index.count('id="readme-body"')==1 and index.count('id="changelog-body"')==1
for ident,name in [('fallback-readme','README.md'),('fallback-changelog','CHANGELOG.md')]:
 assert base64.b64decode(re.search(fr'<script id="{ident}"[^>]*>(.*?)</script>',index,re.S).group(1)) == (root/name).read_bytes()
assert 'fetch(url+\'?v=\'+Date.now()' in index
assert 'GLO' in (root/'CHEAT-SHEET.html').read_text()
print('PASS: public website installer and documentation stay in sync with V'+meta['version'])
