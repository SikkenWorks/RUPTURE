#!/usr/bin/env python3
"""Package RUPTURE V1.91 with unchanged trusted FM1 OTA/bootheader."""
from pathlib import Path
import base64
import hashlib
import json
import re
import shutil
import sys

ROOT = Path(__file__).resolve().parents[1]
NATIVE = ROOT / 'vendor/etch-v1.88-source'
APP_DIR = NATIVE / 'build/fm1-native-storage-updater-uac48-p05'
APP = (APP_DIR / 'app.bin').read_bytes()
sha = lambda data: hashlib.sha256(data).hexdigest()
sys.path.insert(0, str(NATIVE / 'tools'))
import fm1_fwsc as fw

trusted = ROOT / 'RUPTURE-V1.86.1.fwsc'
previous = ROOT / 'RUPTURE-V1.90.fwsc'
assert fw.verify_fwsc(trusted)[1] == []
assert fw.verify_fwsc(previous)[1] == []
assert not fw.validate_app_binary(APP)
assert b'V1.91' in APP
assets = fw.extract_stock_assets(trusted)
assert assets['product'] == 'FM-1_968'
prev_entry = {e['name']: e['data'] for e in fw.parse_ufw(previous.read_bytes())['entries']}
flash = fw.build_flash_image_from_assets(
    APP, assets['uboot'], assets['cfg_tool'], assets['eq_cfg'], assets['key'],
    isd_config=assets['isd_config'], flash_header_payload=assets['flash_header_payload'])
assert len(flash) == len(prev_entry['flash.bin']) == 0x93000
assert flash[:0x4000] == prev_entry['flash.bin'][:0x4000], 'Protected FM1 bootheader must not change'
assert assets['ota'] == prev_entry['ota.bin'], 'Trusted OTA loader must not change'
pkg = fw.build_fwsc(flash, assets['ota'], 'FM-1_968')
target = ROOT / 'RUPTURE-V1.91.fwsc'
target.write_bytes(pkg)
assert fw.verify_fwsc(target)[1] == []
entries = {e['name']: e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert entries['ota.bin'] == prev_entry['ota.bin']
assert entries['flash.bin'][:0x4000] == prev_entry['flash.bin'][:0x4000]
assert entries['flash.bin'] == flash

for name in ('app.bin', 'app.bin.audit.json', 'native-build-options.json'):
    shutil.copy2(APP_DIR / name, ROOT / 'native-build' / name)
options = json.loads((ROOT / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V1.91'
assert options['app_sha256'] == sha(APP)
assert options['ota_writes'] and options['storage_writes']

manifest_path = ROOT / 'RELEASE-MANIFEST.json'
manifest = json.loads(manifest_path.read_text())
manifest.update(
    version='1.91', firmware_file=target.name,
    firmware_bytes=len(pkg), firmware_sha256=sha(pkg),
    app_bytes=len(APP), app_sha256=sha(APP),
    physical_fm1_verified=False, hardware_verified=False, release_ready=False,
    release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',
    release_feature='FX-chain real-time efficiency and dedicated K4 bypass / SEL parameters',
    release_note='Real-time ordered DSP now has a fast dry bypass, bounded tail flush, '
                 'division-free stereo MOSFET ROM transfer and a fused original-order '
                 'COMP/TILT/FUZZ/SAND route. K4 selects slot ON/OFF while SEL opens '
                 'parameters. Existing PJT35 GLIDE and eight-slot FX storage unchanged. '
                 'The preserved OTA loader, bootheader and both flash IDs are retained. '
                 'Hardware CPU/deadline and full eight-FX load remain unverified.',
)
manifest['fx_chain_glide_v190'] = {
    'fx_slots': 8,
    'fx_processors': ['REVERB', 'DELAY', 'CHORUS', 'MOSFET', 'COMP', 'TILT', 'FUZZ', 'SAND'],
    'new_project_fx_slots_empty': True,
    'new_fx_enabled_by_default': True,
    'fx_reorder_changes_pcm': True,
    'fx_duplicate_instances_allowed': False,
    'glide_time_max_ms': 2000,
    'glide_modes': ['OFF', 'LEGATO', 'ALWAYS'],
    'glide_curves': ['LINEAR', 'EXP'],
    'step_slide': True,
    'project_codec_version': 35,
    'legacy_routing_preserved_until_edit': True,
    'trusted_loader_sha256': sha(entries['ota.bin']),
    'protected_bootheader_sha256': sha(flash[:0x4000]),
    'physical_fm1_verified': False,
}
manifest['realtime_v191'] = {
    'zero_slot_fast_dry': True,
    'bounded_old_tank_flush': True,
    'mosfet_division_free_runtime': True,
    'mosfet_transfer_rom_points': 257,
    'reuses_v189_four_fx_fastpath_when_safe': True,
    'fx_sel_opens_parameters': True,
    'fx_k4_on_off': True,
    'no_additional_audio_voices': True,
    'hardware_dma_margin_verified': False,
    'eight_fx_not_heavier_than_v189_verified': False,
}
manifest.setdefault('limitations', []).append(
    'V1.91 reduces dry and MOSFET processing, but a maximum eight-effect chain '
    'may still cost more CPU than V1.89. Real FM1 DMA timing, rapid chain edits '
    'and eight-FX live stability require physical verification before performance use.')
manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')

for installer_name in ('WebInstaller.html', 'index.html'):
    installer_path = ROOT / installer_name
    before = installer_path.read_text()
    assert 'Stop at the FIRST NUL' in before and 'read-only identity queries' in before
    assert 'V1.90' in before or 'V1.91' in before
    package_match = re.search(r'(<script id="firmware"[^>]*>)(.*?)(</script>)', before, re.S)
    assert package_match
    old_pkg = base64.b64decode(package_match.group(2))
    # Safe to rerun packaging after a native build fix: an already embedded
    # V1.91 candidate may differ from the final one. Still forbid replacing
    # a payload with an unrelated loader or protected bootheader.
    old_entries = {e['name']: e['data'] for e in fw.parse_ufw(old_pkg)['entries']}
    assert old_entries['ota.bin'] == entries['ota.bin']
    assert old_entries['flash.bin'][:0x4000] == flash[:0x4000]
    old_hash = re.search(r"const PKG_HASH='([0-9a-f]{64})'", before).group(1)
    old_len = re.search(r'const PKG_LEN=(\d+)', before).group(1)
    assert old_hash == sha(old_pkg) and int(old_len) == len(old_pkg)
    after = before.replace('V1.90', 'V1.91')
    after, n = re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',
        lambda m: m.group(1) + base64.b64encode(pkg).decode('ascii') + m.group(2),
        after, count=1, flags=re.S)
    assert n == 1
    after, n = re.subn(r"const PKG_HASH='[0-9a-f]{64}'",
        f"const PKG_HASH='{sha(pkg)}'", after)
    assert n == 1
    after, n = re.subn(r'const PKG_LEN=\d+', f'const PKG_LEN={len(pkg)}', after)
    assert n == 1
    # Remove only the differences a version refresh is allowed to introduce.
    normalized_after = after.replace('V1.91', 'V1.90')
    normalized_after = normalized_after.replace(base64.b64encode(pkg).decode('ascii'), package_match.group(2))
    normalized_after = normalized_after.replace(sha(pkg), old_hash)
    normalized_after = normalized_after.replace(f'const PKG_LEN={len(pkg)}', f'const PKG_LEN={old_len}')
    normalized_before = before.replace('V1.91', 'V1.90')
    assert normalized_after == normalized_before, f'{installer_name}: OTA/connection logic changed'
    assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>', after, re.S).group(1)) == pkg
    installer_path.write_text(after)

print('PASS V1.91 firmware:', len(pkg), sha(pkg))
print('PASS V1.91 app:', len(APP), sha(APP))
print('PASS unchanged OTA loader:', sha(entries['ota.bin']))
print('PASS unchanged protected bootheader:', sha(flash[:0x4000]))
print('PASS both Web MIDI installers retain their original OTA/connection logic')
