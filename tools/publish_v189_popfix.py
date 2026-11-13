#!/usr/bin/env python3
"""Publish self-contained V1.89 POPFIX firmware, website, source and ENV sprites."""
import argparse
import base64
import hashlib
import json
from pathlib import Path
import re
import shutil
import zipfile

R = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, default=R.parent)
args = parser.parse_args()
O = args.output.resolve()
O.mkdir(parents=True, exist_ok=True)
V = '1.89'
firmware_name = f'RUPTURE-V{V}.fwsc'
fw = (R / firmware_name).read_bytes()
sha = lambda b: hashlib.sha256(b).hexdigest()
manifest = json.loads((R / 'RELEASE-MANIFEST.json').read_text())
assert manifest['version'] == V
assert manifest['firmware_sha256'] == sha(fw)
assert manifest['firmware_bytes'] == len(fw) == 610086
assert (R / 'CHANGELOG.md').read_text().startswith(f'# RUPTURE V{V}')
for page in ('WebInstaller.html', 'index.html'):
    html = (R / page).read_text()
    assert 'Stop at the FIRST NUL' in html
    assert 'read-only identity queries' in html
    raw = base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>', html, re.S).group(1))
    assert raw == fw
    assert f"const PKG_HASH='{sha(fw)}'" in html
    assert f'const PKG_LEN={len(fw)}' in html

outputs = {
    firmware_name: firmware_name,
    f'RUPTURE-WebInstaller-V{V}.html': 'WebInstaller.html',
    f'RUPTURE-V{V}-INDEX.html': 'index.html',
    f'RUPTURE-TEST-RESULTS-V{V}.md': f'RUPTURE-TEST-RESULTS-V{V}.md',
    f'RUPTURE-V{V}-ENV-20-FRAME-SPRITESHEET.png': 'assets/panel-animations-v1883/ENV-20-FRAMES.png',
    f'RUPTURE-V{V}-ENV-20-FRAME-SPRITESHEET-4X.png': 'assets/panel-animations-v1883/ENV-20-FRAMES-PREVIEW-4X.png',
}
for external, internal in outputs.items():
    shutil.copy2(R / internal, O / external)
    assert (O / external).read_bytes() == (R / internal).read_bytes()

site = O / f'RUPTURE-V{V}-GITHUB-PAGES.zip'
website_files = [
    '.nojekyll', 'index.html', 'WebInstaller.html', firmware_name,
    'README.md', 'CHANGELOG.md', 'CHEAT-SHEET.html', 'BEND-PATCH-NOTES.md',
    'WEBSITE-DEPLOY.md', 'LICENSE', 'RELEASE-MANIFEST.json',
    f'RUPTURE-TEST-RESULTS-V{V}.md',
]
website_files += sorted(p.relative_to(R).as_posix() for p in (R / 'assets').rglob('*') if p.is_file())
with zipfile.ZipFile(site, 'w', zipfile.ZIP_DEFLATED, compresslevel=7) as z:
    for name in website_files:
        assert (R / name).is_file(), name
        z.write(R / name, name)

full = O / f'RUPTURE-FM1-V{V}-POPFIX-FULL-SOURCE.zip'
with zipfile.ZipFile(full, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for p in sorted(R.rglob('*')):
        if not p.is_file():
            continue
        rel = p.relative_to(R)
        if any(i in rel.parts for i in ('__pycache__', '.pytest_cache', '.git')):
            continue
        if p.suffix in ('.pyc', '.log', '.o', '.d'):
            continue
        z.write(p, rel.as_posix())

sprites = O / f'RUPTURE-V{V}-ENV-ANIMATION-UNCHANGED.zip'
with zipfile.ZipFile(sprites, 'w', zipfile.ZIP_DEFLATED, compresslevel=7) as z:
    for folder in ('assets/panel-animations-v1883', 'assets/panel-hold-v1883'):
        for p in sorted((R / folder).rglob('*')):
            if p.is_file():
                z.write(p, p.relative_to(R).as_posix())
    z.write(R / f'RUPTURE-TEST-RESULTS-V{V}.md', f'RUPTURE-TEST-RESULTS-V{V}.md')

for archive in (site, full, sprites):
    with zipfile.ZipFile(archive) as z:
        assert z.testzip() is None
        names = set(z.namelist())
        if archive != sprites:
            assert z.read(firmware_name) == fw
            assert z.read('index.html') == (R / 'index.html').read_bytes()
            assert z.read('WebInstaller.html') == (R / 'WebInstaller.html').read_bytes()
        else:
            assert len([n for n in names if n.startswith('assets/panel-animations-v1883/env/') and n.endswith('.png')]) == 20
            assert len([n for n in names if n.startswith('assets/panel-hold-v1883/env/') and n.endswith('.png')]) == 8
        if archive == full:
            for essential in (
                'native-build/app.bin', 'tools/package_v189_popfix.py',
                'tools/generate_env_complete_v1883.py', 'tests/test_v1883_env_animation.c',
                'tests/test_v189_popfix_ota.py', 'tests/pop_fix/run_pop_tests.sh',
                'vendor/etch-v1.88-source/firmware/fm1-native/bringup.c',
                'vendor/etch-v1.88-source/firmware/fm1-native/hal/fm1_audio.h',
                'vendor/etch-v1.88-source/firmware/fm1-native/rupture_panel_anim.h',
            ):
                assert essential in names, essential
        print('VERIFIED', archive.name, archive.stat().st_size, 'bytes', len(names), 'files')

for item in [*outputs, site.name, full.name, sprites.name]:
    print('DOWNLOAD', item, (O / item).stat().st_size)
print('FIRMWARE SHA256', sha(fw))
