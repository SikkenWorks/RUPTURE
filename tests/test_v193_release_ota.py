#!/usr/bin/env python3
"""RUPTURE V1.93: installer, bootheader, flash IDs, POPFIX and native app audit."""
from pathlib import Path
import base64
import hashlib
import json
import re
import sys

R = Path(__file__).resolve().parents[1]
N = R / 'vendor/etch-v1.88-source'
SRC = N / 'firmware/fm1-native'
sys.path.insert(0, str(N / 'tools'))
import fm1_fwsc as fw

sha = lambda data: hashlib.sha256(data).hexdigest()
manifest = json.loads((R / 'RELEASE-MANIFEST.json').read_text())
assert manifest['version'] == '1.93'
assert manifest['firmware_file'] == 'RUPTURE-V1.93.fwsc'
assert manifest['fx_expansion_v192']['available_types'] == 10
assert manifest['fx_expansion_v192']['simultaneous_slots'] == 8
assert manifest['fx_repeat_v193']['duplicate_type_in_slots_allowed']
assert manifest['fx_repeat_v193']['new_pcm_buffers'] == 0
assert not manifest['release_ready'] and not manifest['physical_fm1_verified']
app = (R / 'native-build/app.bin').read_bytes()
assert not fw.validate_app_binary(app)
assert sha(app) == manifest['app_sha256'] and len(app) == manifest['app_bytes']
assert app == (N / 'build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
assert b'V1.93' in app
options = json.loads((R / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V1.93'
assert options['storage_writes'] and options['ota_writes']
new_fw = (R / manifest['firmware_file']).read_bytes()
old_fw = (R / 'RUPTURE-V1.92.fwsc').read_bytes()
assert sha(new_fw) == manifest['firmware_sha256']
assert len(new_fw) == manifest['firmware_bytes'] == len(old_fw) == 610086
assert fw.verify_fwsc(R / manifest['firmware_file'])[1] == []
entries = lambda data: {e['name']: e['data'] for e in fw.parse_ufw(data)['entries']}
new, old = entries(new_fw), entries(old_fw)
assert new['ota.bin'] == old['ota.bin']
assert new['flash.bin'][:0x4000] == old['flash.bin'][:0x4000]
assert len(new['flash.bin']) == len(old['flash.bin']) == 0x93000
key = fw.extract_stock_assets(R / 'RUPTURE-V1.86.1.fwsc')['key']
encrypted = bytearray(new['flash.bin'][0x4000:0x4000+0x120+len(app)])
fw.sfc_xor(encrypted, 0, len(encrypted), 0, key)
assert encrypted[0x120:] == app

for name, expected in {
    'hal/fm1_audio.h': '9e711bbff254561c06376bbfd2f875d2eadeedc55dd412ed16bedf9b0b71e5cf',
    'native_output_startup.h': '65e5202624054b94f55fd1df058e9156e52e7536aefcd8536ca0659fca9f1e28',
}.items():
    source = (SRC/name).read_text()
    normalized = re.sub(r'\s+', '', re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)).encode()
    assert sha(normalized) == expected, f'POPFIX source changed: {name}'

for name, expected in {
    'rupture_panel_sprites.h': 'becac33ca92ce1dea07268118aa6be13364ad3919f4390f82ade0389fa3079d3',
    'rupture_hold_sprites.h': 'f4c46311fd4982a0663069801af3547a3337d4f220bd783b38c4a0b71e32204a',
    'rupture_panel_anim.h': '61a06441a2174a8f1535ee44b31bff442e6abc41c65716a6a99c270fb8f87659',
    'rupture_bend_sprites.h': 'acf8d1a0966567d80ff94fa5767e896db71a8ca94439ebd68781e3dd53922781',
    'audio_target.c': 'b8dfa2ef0ced4be23a9f7e36a99d7327ec2867afde711f2df132c3ddc6071d2a',
    'ota.c': 'c77217b9a650beda39827a2754e76a8502ee97c8244ce07087e05c87dc9e34e6',
    'native_panel_midi.c': '29e5d831f453c723e1999cefb95abeddb69d7760c1f1056519fecc2bef90aa2a',
    'usb.c': '3e484359d55c68e7510b3b588315e1e781ca30d3946f7d0fea4288895cb6dbef',
    'ota_target.c': '1d9a9ceb0489d98211d9757eafcd9ed021b9f7d57efa4e0ef3631824f2a44c5a',
    'wave_store_target.c': '2027c9d68ddc801da4541c6fbfd1339824c51db2699ffbb4ffa7124459131813',
    'hal/fm1_flash_id.h': 'c77ff4febd1e1efc1cea73e0080aedb7846be431d44dc4983862f3e2c34a0aac',
}.items():
    # OTA and wave-store source hashes may be refreshed by targeted source tests,
    # but native startup artwork, runtime identity and flash IDs must stay intact.
    assert sha((SRC/name).read_bytes()) == expected, f'Unrelated source changed: {name}'

flash_id = (SRC/'hal/fm1_flash_id.h').read_text()
assert '0x856014u' in flash_id and '0xB36014u' in flash_id
bringup = (SRC/'bringup.c').read_text()
assert 'ETCH_OUTPUT_CODEC_PRE_LED_MS' in (SRC/'native_output_startup.h').read_text()
assert 'ru_fx_sync_chain' in bringup and 'ru_fx_migrate_legacy' in bringup
assert 'RU_PAGE_GLIDE' in bringup and 'RU_PAGE_FX_PICK' in bringup
assert 'ru_fx_page_for_selection(native_project_cfg.v190.chain.type[ru_fx_selected])' in bringup
assert 'control==3u' in bringup and 'chain->enabled|=flag' in bringup
atomic_chain = bringup.split('static void ru_fx_sync_chain(void){', 1)[1].split('static void ru_fx_apply_extra(void){', 1)[0]
assert 'fm1_irq_off();' in atomic_chain and 'fm1_irq_enable_all();' in atomic_chain
assert 'native_controls.fx_chain=*c;' in atomic_chain
assert 'ru_mosfet_shaper' in (SRC/'native_fx.c').read_text()
assert 'RU_FX_DISTORTION' in (SRC/'native_fx.c').read_text()
assert 'RU_FX_WAH' in (SRC/'native_fx.c').read_text()
assert 'RU_PAGE_DISTORTION' in bringup and 'RU_PAGE_WAH' in bringup
assert 'RU_FX_REMOVE' in bringup
assert 'rupture_chain_fade[RU_FX_SLOTS]' in (SRC/'native_fx.h').read_text()
assert 'uint16_t shared_fx_done=0u;' in (SRC/'native_fx.c').read_text()
assert 'SAME FX MAY FILL MANY SLOTS' in (SRC/'rupture_tutorial.h').read_text()
ui = (SRC/'rupture_ui.c').read_text()
assert 'ru_center("PORTAMENTO",120,54,1)' in ui
assert 'MONO PORTAMENTO' not in ui and 'PITCH ONLY / NO EXTRA VOICE' not in ui

for name in ('WebInstaller.html', 'index.html'):
    html = (R/name).read_text()
    firmware_b64 = re.search(r'<script id="firmware"[^>]*>(.*?)</script>', html, re.S).group(1)
    assert base64.b64decode(firmware_b64) == new_fw
    assert f"const PKG_HASH='{sha(new_fw)}'" in html
    assert f'const PKG_LEN={len(new_fw)}' in html
    assert 'V1.93' in html
    assert 'Stop at the FIRST NUL' in html and 'read-only identity queries' in html

print('PASS V1.93 FWSC/flash decryption and both web installer payloads')
print('PASS trusted OTA, protected bootheader, POPFIX and both flash IDs')
print('PASS untouched animation sprites and unaffected native subsystems')
