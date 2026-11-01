#!/usr/bin/env python3
"""Check static site structure, embedded integrity and documentation fallback."""
from __future__ import annotations
import base64,hashlib,json,re
from html.parser import HTMLParser
from pathlib import Path

ROOT=Path(__file__).resolve().parent.parent
class TagInspect(HTMLParser):
    def __init__(self):
        super().__init__();self.ids=[];self.anchor_refs=[];self.scripts=0
    def handle_starttag(self,tag,attrs):
        a=dict(attrs)
        if 'id' in a:self.ids.append(a['id'])
        if tag=='script':self.scripts+=1
        if tag=='a':self.anchor_refs.append(a.get('href'))

s=(ROOT/'index.html').read_text('utf-8')
m=json.loads((ROOT/'RELEASE-MANIFEST.json').read_text('utf-8'))
fw=(ROOT/m['firmware_file']).read_bytes()
assert m['firmware_sha256']==hashlib.sha256(fw).hexdigest()
assert len(fw)==m['firmware_bytes']
assert f'RUPTURE V{m["version"]}' in s
assert 'id="firmware"' in s
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',s,re.S).group(1))==fw
assert f"const PKG_HASH='{m['firmware_sha256']}'" in s
assert f"const PKG_LEN={len(fw)}" in s
for tag,fname in [('fallback-readme','README.md'),('fallback-changelog','CHANGELOG.md')]:
    found=re.search(r'<script id="'+tag+r'"[^>]*>(.*?)</script>',s,re.S)
    assert found and base64.b64decode(found.group(1)) == (ROOT/fname).read_bytes()
cheats=json.loads(re.search(r'<script id="fallback-cheats" type="application/json">(.*?)</script>',s,re.S).group(1))
assert len(cheats)>=12
for f in ('README.md','CHANGELOG.md','CHEAT-SHEET.html','tools/site_template.html','tools/build_site.py','WebInstaller.html','assets/boot-animation.gif','assets/boot-final.png'):
    assert (ROOT/f).is_file(), f
for part in ('id="readme-body"','id="changelog-body"','id="cheat-search"','id="cheat-grid"','id="install"','id="connect"','id="progress"','id="log"','id="firmware-download"'):
    assert part in s
html=TagInspect();html.feed(s)
from collections import Counter
assert all(count==1 for count in Counter(html.ids).values()),[(k,v) for k,v in Counter(html.ids).items() if v>1]
assert html.scripts==8
print('PASS: release/firmware/embedded WebInstaller byte parity, hash and length')
print('PASS: live docs + fallback snapshots, cheat entries, OTA controls, unique IDs')
print(f'PASS: RUPTURE V{m["version"]}, {len(cheats)} cheat topics, {len(fw):,} firmware bytes, {len(s):,} webpage chars')
