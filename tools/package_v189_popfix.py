#!/usr/bin/env python3
"""RUPTURE V1.89: package POPFIX app with the original trusted OTA assets."""
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
previous = R / 'RUPTURE-V1.88.3.fwsc'
assert fw.verify_fwsc(trusted)[1] == []
assert fw.verify_fwsc(previous)[1] == []
assert not fw.validate_app_binary(app)
assert b'V1.89' in app
assert b'V1.88.3' not in app, 'Old ABOUT label leaked into native binary'
assets = fw.extract_stock_assets(trusted)
assert assets['product'] == 'FM-1_968'
old = {e['name']: e['data'] for e in fw.parse_ufw(previous.read_bytes())['entries']}
flash = fw.build_flash_image_from_assets(
    app, assets['uboot'], assets['cfg_tool'], assets['eq_cfg'], assets['key'],
    isd_config=assets['isd_config'], flash_header_payload=assets['flash_header_payload'])
assert len(flash) == len(old['flash.bin']) == 0x93000
assert flash[:0x4000] == old['flash.bin'][:0x4000], 'Protected 16 KiB bootheader changed'
assert assets['ota'] == old['ota.bin'], 'Trusted OTA loader changed'
package = fw.build_fwsc(flash, assets['ota'], 'FM-1_968')
target = R / 'RUPTURE-V1.89.fwsc'
target.write_bytes(package)
assert fw.verify_fwsc(target)[1] == []
entries = {e['name']: e['data'] for e in fw.parse_ufw(package)['entries']}
assert entries['ota.bin'] == old['ota.bin']
assert entries['flash.bin'][:0x4000] == old['flash.bin'][:0x4000]
assert entries['flash.bin'] == flash

for name in ('app.bin', 'app.bin.audit.json', 'native-build-options.json'):
    shutil.copy2(APP_DIR / name, R / 'native-build' / name)
options = json.loads((R / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V1.89'
assert options['app_sha256'] == sha(app)
assert options['ota_writes'] and options['storage_writes']

manifest_path = R / 'RELEASE-MANIFEST.json'
manifest = json.loads(manifest_path.read_text())
manifest.update(
    version='1.89', firmware_file=target.name,
    firmware_bytes=len(package), firmware_sha256=sha(package),
    app_bytes=len(app), app_sha256=sha(app),
    physical_fm1_verified=False, hardware_verified=False, release_ready=False,
    release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',
    release_feature='Speaker POPFIX from user-provided V1.88.2 with full V1.88.3 ENV animation',
    release_note='Clock silent I2S DMA before codec PC0/2/1; settle codec for 300 ms before LED '
                 'activation; retain PC6 400 ms LED/USB/clock gating and soft PCM ramp. '
                 'Original V1.88.3 20-frame ENV stays intact. Trusted OTA loader and '
                 'protected header byte-identical to V1.88.3. Hardware acoustic test pending.',
)
manifest['popfix_v189'] = {
    'source': 'user-provided RUPTURE-FM1-V1.88.2-POPFIX-SOURCE.zip',
    'modified_native_sources': [
        'bringup.c', 'hal/fm1_audio.h', 'native_output_startup.h',
    ],
    'clock_before_codec_control': True,
    'codec_line_sequence': ['PC0', 'PC2', 'PC1'],
    'codec_step_ms': 2, 'codec_settle_ms': 5,
    'quiet_codec_to_led_ms': 300, 'pc6_led_quiet_ms': 400,
    'warm_restart_skips_repeated_codec_control_edges': True,
    'legacy_ab_switch_default': 0,
    'original_env_v1883_unchanged': True,
    'trusted_ota_loader_unchanged': True,
    'protected_bootheader_unchanged': True,
    'loader_sha256': sha(entries['ota.bin']),
    'bootheader_sha256': sha(flash[:0x4000]),
    'acoustic_pop_physically_verified': False,
}
manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')

installer_path = R / 'WebInstaller.html'
before = installer_path.read_text()
assert 'V1.88.3' in before and 'V1.89' not in before
assert 'Stop at the FIRST NUL' in before and 'read-only identity queries' in before
before_scripts = re.findall(r'<script\b[^>]*>([\s\S]*?)</script>', before)
assert len(before_scripts) == 3
new = before.replace('V1.88.3', 'V1.89')
new, n = re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',
    lambda m: m.group(1) + base64.b64encode(package).decode('ascii') + m.group(2),
    new, count=1, flags=re.S)
assert n == 1
new, n = re.subn(r"const PKG_HASH='[0-9a-f]{64}'",
    f"const PKG_HASH='{sha(package)}'", new)
assert n == 1
new, n = re.subn(r'const PKG_LEN=\d+', f'const PKG_LEN={len(package)}', new)
assert n == 1
after_scripts = re.findall(r'<script\b[^>]*>([\s\S]*?)</script>', new)
assert after_scripts[1].replace('V1.89', 'V1.88.3') == before_scripts[1], 'OTA engine changed'
before_app = before_scripts[2]
after_app = after_scripts[2].replace('V1.89', 'V1.88.3')
after_app = after_app.replace(sha(package), sha(previous.read_bytes()))
after_app = after_app.replace(f'const PKG_LEN={len(package)}', f'const PKG_LEN={len(previous.read_bytes())}')
assert after_app == before_app, 'Web MIDI installer logic changed'
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',new,re.S).group(1)) == package
installer_path.write_text(new)

print('PASS RUPTURE V1.89 app:', len(app), sha(app))
print('PASS V1.89 FWSC:', len(package), sha(package))
print('PASS original OTA loader unchanged:', sha(entries['ota.bin']))
print('PASS protected bootheader unchanged:', sha(flash[:0x4000]))
print('PASS V1.88.3 Web MIDI connection/OTA JS retained verbatim apart from version/hash/length')
