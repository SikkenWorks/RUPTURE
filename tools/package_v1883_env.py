#!/usr/bin/env python3
"""Build V1.88.3 using the proven RUPTURE OTA loader and protected header.

Only the native app, ENV graphics/sequence and release labels change.
"""
from pathlib import Path
import base64
import hashlib
import json
import re
import shutil
import sys

R = Path(__file__).resolve().parents[1]
N = R / 'vendor/etch-v1.88-source'
APP_DIR = N / 'build/fm1-native-storage-updater-uac48-p05'
app = (APP_DIR / 'app.bin').read_bytes()
sha = lambda b: hashlib.sha256(b).hexdigest()
sys.path.insert(0, str(N / 'tools'))
import fm1_fwsc as fw

trusted = R / 'RUPTURE-V1.86.1.fwsc'
previous = R / 'RUPTURE-V1.88.2.fwsc'
assert fw.verify_fwsc(trusted)[1] == []
assert fw.verify_fwsc(previous)[1] == []
assert not fw.validate_app_binary(app)
assert b'V1.88.3' in app
assets = fw.extract_stock_assets(trusted)
assert assets['product'] == 'FM-1_968'
old = {e['name']: e['data'] for e in fw.parse_ufw(previous.read_bytes())['entries']}
flash = fw.build_flash_image_from_assets(
    app, assets['uboot'], assets['cfg_tool'], assets['eq_cfg'], assets['key'],
    isd_config=assets['isd_config'], flash_header_payload=assets['flash_header_payload'])
assert len(flash) == len(old['flash.bin']) == 0x93000
assert flash[:0x4000] == old['flash.bin'][:0x4000], 'Protected bootheader changed'
assert assets['ota'] == old['ota.bin'], 'Trusted OTA loader changed'
package = fw.build_fwsc(flash, assets['ota'], 'FM-1_968')
target = R / 'RUPTURE-V1.88.3.fwsc'
target.write_bytes(package)
assert fw.verify_fwsc(target)[1] == []
entries = {e['name']: e['data'] for e in fw.parse_ufw(package)['entries']}
assert entries['ota.bin'] == old['ota.bin']
assert entries['flash.bin'][:0x4000] == old['flash.bin'][:0x4000]
assert entries['flash.bin'] == flash

for name in ('app.bin', 'app.bin.audit.json', 'native-build-options.json'):
    shutil.copy2(APP_DIR / name, R / 'native-build' / name)
options = json.loads((R / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V1.88.3'
assert options['app_sha256'] == sha(app)

manifest_file = R / 'RELEASE-MANIFEST.json'
manifest = json.loads(manifest_file.read_text())
manifest.update(
    version='1.88.3', firmware_file=target.name,
    firmware_bytes=len(package), firmware_sha256=sha(package),
    app_bytes=len(app), app_sha256=sha(app),
    physical_fm1_verified=False, hardware_verified=False, release_ready=False,
    release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',
    release_feature='ENV 20-frame eruption + 8 settling frames with permanent hole',
    release_note='ENV plays native frames 1..20 while pressed, holds the remaining hole '
                 'without a frozen smoke plume, and reverses the exact frames on note release. '
                 'BEND/LFO/audio/FX/storage/MIDI unchanged. Original OTA loader and protected '
                 'bootheader byte-identical to V1.88.2; physical FM1 test pending.',
)
manifest['env_v1883'] = {
    'full_frames': 20, 'settling_hold_frames': 8,
    'resolution': [224, 30], 'color_depth_bits': 1,
    'pressed': '1->20, final open hole remains',
    'released': 'current frame->1 in exact reverse',
    'lfo_sprites_changed': False, 'bend_sprites_changed': False,
    'protected_bootheader_unchanged': True, 'ota_loader_unchanged': True,
    'ota_loader_sha256': sha(entries['ota.bin']),
    'protected_bootheader_sha256': sha(flash[:0x4000]),
    'physical_fm1_verified': False,
}
manifest_file.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')

installer_file = R / 'WebInstaller.html'
before = installer_file.read_text()
assert 'V1.88.2' in before and 'V1.88.3' not in before
assert 'Stop at the FIRST NUL' in before and 'read-only identity queries' in before
old_scripts = re.findall(r'<script\b[^>]*>([\s\S]*?)</script>', before)
assert len(old_scripts) == 3
new = before.replace('V1.88.2', 'V1.88.3')
new, n = re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',
    lambda m: m.group(1) + base64.b64encode(package).decode('ascii') + m.group(2),
    new, count=1, flags=re.S)
assert n == 1
new, n = re.subn(r"const PKG_HASH='[0-9a-f]{64}'",
    f"const PKG_HASH='{sha(package)}'", new)
assert n == 1
new, n = re.subn(r'const PKG_LEN=\d+', f'const PKG_LEN={len(package)}', new)
assert n == 1
new_scripts = re.findall(r'<script\b[^>]*>([\s\S]*?)</script>', new)
assert new_scripts[1].replace('V1.88.3', 'V1.88.2') == old_scripts[1], 'OTA engine was modified'
normalized = new_scripts[2].replace('V1.88.3', 'V1.88.2')
normalized = normalized.replace(sha(package), sha(previous.read_bytes()))
normalized = normalized.replace(f'const PKG_LEN={len(package)}',
    f'const PKG_LEN={len(previous.read_bytes())}')
assert normalized == old_scripts[2], 'Installer application logic was modified'
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',new,re.S).group(1)) == package
installer_file.write_text(new)

print('PASS native app:', len(app), 'bytes, sha256', sha(app))
print('PASS FWSC package:', len(package), 'bytes, sha256', sha(package))
print('PASS OTA loader byte-identical:', sha(entries['ota.bin']))
print('PASS protected 16 KiB bootheader byte-identical:', sha(flash[:0x4000]))
print('PASS WebInstaller OTA + connection logic unchanged, embedded firmware updated')
