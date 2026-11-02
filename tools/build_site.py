#!/usr/bin/env python3
"""Build RUPTURE's single-file website from the current release.

Usage:
    python3 tools/build_site.py
    python3 tools/build_site.py --root /path/to/release

Requires only Python's standard library. Hard-fails if the release .fwsc,
manifest and embedded WebInstaller firmware aren't byte-identical.
The Web MIDI OTA engine and installer application scripts are copied
verbatim; no changes to the firmware image are made by this script.
"""
from __future__ import annotations

import argparse
import base64
import hashlib
import html
import json
from pathlib import Path
import re
import sys
from html.parser import HTMLParser


class Cheats(HTMLParser):
    def __init__(self):
        super().__init__()
        self.is_section = False
        self.in_head = False
        self.in_para = False
        self.items = []
        self.title = ""
        self.paras = []
        self.accum = []

    def handle_starttag(self, tag, attrs):
        if tag == 'section':
            self.is_section = True
            self.title, self.paras = '', []
        if self.is_section and tag in ('h2', 'p'):
            self.accum = []
            self.in_head = tag == 'h2'
            self.in_para = tag == 'p'

    def handle_data(self, data):
        if self.in_head or self.in_para:
            self.accum.append(data)

    def handle_endtag(self, tag):
        if tag == 'h2' and self.in_head:
            self.title = ''.join(self.accum).strip()
            self.in_head = False
        elif tag == 'p' and self.in_para:
            self.paras.append(''.join(self.accum).strip())
            self.in_para = False
        elif tag == 'section' and self.is_section:
            if self.title and self.paras:
                self.items.append({'title': self.title, 'body': '\n'.join(self.paras)})
            self.is_section = False


def script_parts(installer: str) -> list[str]:
    scripts = re.findall(r'<script\b[^>]*>.*?</script\s*>', installer, re.S | re.I)
    assert len(scripts) == 3, f'Expected exact 3 original installer scripts, got {len(scripts)}'
    assert scripts[0].startswith('<script id="firmware"')
    assert 'root.ETCHOta=api' in scripts[1]
    assert 'PKG_HASH=' in scripts[2]
    return scripts


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument('--root', type=Path, default=Path(__file__).resolve().parent.parent)
    p.add_argument('--output', type=Path)
    args = p.parse_args()
    root = args.root.resolve()
    manifest = json.loads((root / 'RELEASE-MANIFEST.json').read_text('utf-8'))
    version = str(manifest['version'])
    filename = manifest['firmware_file']
    fw = (root / filename).read_bytes()
    sha = hashlib.sha256(fw).hexdigest()
    assert sha == manifest['firmware_sha256'], 'Release manifest SHA-256 does not match firmware file'
    assert len(fw) == manifest['firmware_bytes'], 'Release manifest firmware size does not match'
    installer = (root / 'WebInstaller.html').read_text('utf-8')
    original = script_parts(installer)
    embed_match = re.search(r'<script id="firmware"[^>]*>(.*?)</script>', original[0], re.S)
    assert embed_match
    embedded_fw = base64.b64decode(embed_match.group(1).strip())
    assert embedded_fw == fw, 'Original WebInstaller and .fwsc bytes differ'
    installer_hash = re.search(r"const PKG_HASH='([0-9a-f]+)'", original[2])
    installer_length = re.search(r'const PKG_LEN=(\d+)', original[2])
    assert installer_hash and installer_hash.group(1) == sha, 'Installer SHA does not match'
    assert installer_length and int(installer_length.group(1)) == len(fw), 'Installer size does not match'
    assert f'V{version}' in installer
    # The installation logic is retained; the package download becomes available
    # only AFTER the original SHA-256 and product-identity check succeeds.
    app = original[2]
    needle = "$('status').textContent='Package verified.';$('connect').disabled=false;"
    insert = "$('status').textContent='Package verified.';const d=$('firmware-download');if(d){d.href=URL.createObjectURL(new Blob([raw],{type:'application/octet-stream'}));d.hidden=false;}$('connect').disabled=false;"
    if app.count(needle) == 1:
        app = app.replace(needle, insert)
    else:
        assert app.count("$('firmware-download')") == 1, 'Corrected installer must expose firmware download link' 
    # Prior installer had an outdated V1.1 reminder despite the V1.20 package.
    app = app.replace('does not prove V1.1 is running. Confirm V1.1', f'does not prove V{version} is running. Confirm V{version}')
    readme = (root / 'README.md').read_bytes()
    changelog = (root / 'CHANGELOG.md').read_bytes()
    assert f'V{version}'.encode() in changelog[:500], 'CHANGELOG does not start with current version'
    cheats = Cheats()
    cheats.feed((root / 'CHEAT-SHEET.html').read_text('utf-8'))
    assert len(cheats.items) >= 12, 'Cheat sheet missing topics'
    def image_data(name: str, mime: str) -> str:
        return f'data:{mime};base64,' + base64.b64encode((root / 'assets' / name).read_bytes()).decode('ascii')
    template = (root / 'tools' / 'site_template.html').read_text('utf-8')
    values = {
        '{{VERSION}}': version,
        '{{BOOT_GIF_DATA}}': image_data('boot-animation.gif','image/gif'),
        '{{BOOT_STILL_DATA}}': image_data('boot-final.png','image/png'),
        '{{README_FALLBACK}}': base64.b64encode(readme).decode('ascii'),
        '{{CHANGELOG_FALLBACK}}': base64.b64encode(changelog).decode('ascii'),
        '{{CHEAT_FALLBACK_JSON}}': json.dumps(cheats.items,ensure_ascii=True).replace('</','<\\/'),
        '{{SITE_META_JSON}}': json.dumps({'version':version,'identity':manifest['firmware_identity'],'sha256':sha,'bytes':len(fw)}),
        '{{FIRMWARE_SCRIPT}}': original[0],
        '{{OTA_ENGINE_SCRIPT}}': original[1],
        '{{INSTALL_APP_SCRIPT}}': app,
    }
    for key, value in values.items():
        if template.count(key) != 1 and key != '{{VERSION}}':
            raise ValueError(f'Missing or duplicated template marker: {key} x {template.count(key)}')
        template = template.replace(key,value)
    if re.search(r'\{\{[A-Z0-9_]+\}\}',template):
        raise ValueError('Unfilled template variables detected')
    output = args.output or root / 'index.html'
    output.parent.mkdir(parents=True,exist_ok=True)
    output.write_text(template,'utf-8')
    assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',template,re.S).group(1)) == fw
    print(f'Built {output} | {len(template):,} chars | RUPTURE V{version} | firmware SHA-256 {sha}')
    print(f'Embedded README {len(readme):,} B, CHANGELOG {len(changelog):,} B, {len(cheats.items)} cheat topics')

if __name__ == '__main__':
    main()
