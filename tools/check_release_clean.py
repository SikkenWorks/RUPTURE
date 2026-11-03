#!/usr/bin/env python3
"""Fail when a new RUPTURE release carries stale root binaries or README release notes."""
from pathlib import Path
import json,re,hashlib,sys
from zipfile import ZipFile
root=Path(__file__).resolve().parent.parent
manifest=json.loads((root/'RELEASE-MANIFEST.json').read_text())
fw=root/manifest['firmware_file']
assert fw.is_file() and fw.suffix=='.fwsc'
assert [p.name for p in root.glob('*.fwsc')]==[fw.name], 'More than one active firmware at project root'
assert hashlib.sha256(fw.read_bytes()).hexdigest()==manifest['firmware_sha256']
assert manifest['app_sha256']==hashlib.sha256((root/'native-build/app.bin').read_bytes()).hexdigest()
readme=(root/'README.md').read_text()
assert not re.search(r'^#{1,4}\s*(?:RUPTURE\s*)?V\d+\.\d+\b',readme,re.M|re.I), 'Release notes belong in CHANGELOG, not README'
assert (root/'CHANGELOG.md').read_text().startswith(f'# RUPTURE V{manifest["version"]}')
assert not any(root.glob('*.log')), 'Do not ship old build logs at release root'
assert (root/f'docs/TEST-V{manifest["version"]}.md').is_file()
print('PASS: current-only firmware, clean README, consistent manifest and project layout')
