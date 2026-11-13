#!/usr/bin/env python3
"""Independent V1.89 artifact audit: exact POPFIX logic, V1.88.3 ENV, OTA."""
from pathlib import Path
import base64
import hashlib
import json
import re
import sys

R = Path(__file__).resolve().parents[1]
N = R / 'vendor/etch-v1.88-source'
SRC = N / 'firmware/fm1-native'
sha = lambda b: hashlib.sha256(b).hexdigest()
sys.path.insert(0, str(N / 'tools'))
import fm1_fwsc as fw

manifest = json.loads((R / 'RELEASE-MANIFEST.json').read_text())
assert manifest['version'] == '1.89'
assert manifest['firmware_file'] == 'RUPTURE-V1.89.fwsc'
assert not manifest['hardware_verified'] and not manifest['release_ready']
app = (R / 'native-build/app.bin').read_bytes()
assert not fw.validate_app_binary(app)
assert sha(app) == manifest['app_sha256'] and len(app) == manifest['app_bytes']
assert app == (N / 'build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
options = json.loads((R / 'native-build/native-build-options.json').read_text())
assert options['storage_writes'] and options['ota_writes']
assert options['version'] == 'RUPTURE V1.89'
new_fw = (R / manifest['firmware_file']).read_bytes()
old_fw = (R / 'RUPTURE-V1.88.3.fwsc').read_bytes()
assert sha(new_fw) == manifest['firmware_sha256']
assert len(new_fw) == manifest['firmware_bytes'] == len(old_fw) == 610086
assert fw.verify_fwsc(R / manifest['firmware_file'])[1] == []
get_entries = lambda raw: {e['name']: e['data'] for e in fw.parse_ufw(raw)['entries']}
new = get_entries(new_fw)
old = get_entries(old_fw)
assert new['ota.bin'] == old['ota.bin'], 'Trusted OTA loader changed'
assert new['flash.bin'][:0x4000] == old['flash.bin'][:0x4000], 'Protected header changed'
assert len(new['flash.bin']) == len(old['flash.bin']) == 0x93000
key = fw.extract_stock_assets(R / 'RUPTURE-V1.86.1.fwsc')['key']
encrypted_app = bytearray(new['flash.bin'][0x4000:0x4000 + 0x120 + len(app)])
fw.sfc_xor(encrypted_app, 0, len(encrypted_app), 0, key)
assert encrypted_app[0x120:] == app, 'Decoded app does not match native build'

# Semantic hashes of the THREE imported files from the exact user-provided
# RUPTURE-FM1-V1.88.2-POPFIX-SOURCE.zip. Whitespace/comments may document V1.89,
# but statements/preprocessor logic must be byte-for-byte equivalent.
def normalized_cpp(path):
    source = path.read_text()
    without_comments = re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)
    return re.sub(r'\s+', '', without_comments).encode()

popfix_logic = {
    'bringup.c': '5a04b3dc54f0f454bd228625f638ae63bcecdd4458a946b1a73131537334d47c',
    'hal/fm1_audio.h': '9e711bbff254561c06376bbfd2f875d2eadeedc55dd412ed16bedf9b0b71e5cf',
    'native_output_startup.h': '65e5202624054b94f55fd1df058e9156e52e7536aefcd8536ca0659fca9f1e28',
}
for name, expected in popfix_logic.items():
    assert sha(normalized_cpp(SRC / name)) == expected, f'POPFIX source logic changed: {name}'
audio_header = (SRC / 'hal/fm1_audio.h').read_text()
assert '#define FM1_AUDIO_LEGACY_CODEC_ORDER 0' in audio_header
assert 'FM1_ALNK_CON0 |= 0x800u' in audio_header
assert 'fm1_audio_codec_lines_asserted()' in audio_header
assert 'FM1_AUDIO_CODEC_STEP_MS 2u' in audio_header
assert 'FM1_AUDIO_CODEC_SETTLE_MS 5u' in audio_header
assert 'ETCH_OUTPUT_CODEC_PRE_LED_MS 300u' in (SRC / 'native_output_startup.h').read_text()

# Every framebuffer, sprite lookup, feature and audio engine from V1.88.3 is
# unchanged. The UI changed only its ABOUT version string.
inherited = {
    'rupture_panel_sprites.h': 'becac33ca92ce1dea07268118aa6be13364ad3919f4390f82ade0389fa3079d3',
    'rupture_hold_sprites.h': 'f4c46311fd4982a0663069801af3547a3337d4f220bd783b38c4a0b71e32204a',
    'rupture_panel_anim.h': '61a06441a2174a8f1535ee44b31bff442e6abc41c65716a6a99c270fb8f87659',
    'rupture_bend_sprites.h': 'acf8d1a0966567d80ff94fa5767e896db71a8ca94439ebd68781e3dd53922781',
    'audio_core.c': '740b88a5dd9dbb7aad7c25b6410811fcd54098cebb1f489646c3c761d235a1c0',
    'audio_target.c': 'b8dfa2ef0ced4be23a9f7e36a99d7327ec2867afde711f2df132c3ddc6071d2a',
    'ota.c': 'c77217b9a650beda39827a2754e76a8502ee97c8244ce07087e05c87dc9e34e6',
    'native_fx.c': '99e37d0564d5fd1bcb95ede763ed2c5218cd06f0c505d87f21e8b82c01feb645',
    'native_panel_midi.c': '29e5d831f453c723e1999cefb95abeddb69d7760c1f1056519fecc2bef90aa2a',
    'usb.c': '3e484359d55c68e7510b3b588315e1e781ca30d3946f7d0fea4288895cb6dbef',
    'ota_target.c': '1d9a9ceb0489d98211d9757eafcd9ed021b9f7d57efa4e0ef3631824f2a44c5a',
    'wave_store_target.c': '2027c9d68ddc801da4541c6fbfd1339824c51db2699ffbb4ffa7124459131813',
    'hal/fm1_flash_id.h': 'c77ff4febd1e1efc1cea73e0080aedb7846be431d44dc4983862f3e2c34a0aac',
}
for name, expected in inherited.items():
    assert sha((SRC / name).read_bytes()) == expected, f'Unrelated V1.88.3 code changed: {name}'
ui = (SRC / 'rupture_ui.c').read_text()
assert sha(ui.replace('"V1.89"', '"V1.88.3"').encode()) == 'cf8bc3b7eeba0529b87463add4ee3c509a291af1e30fe503d64985786d913dd8'
chip = (SRC / 'hal/fm1_flash_id.h').read_text()
assert '0x856014u' in chip and '0xB36014u' in chip

# Compare both published HTML containers, and ensure the OTA JavaScript is
# untouched after normalized release-version/hash substitution.
for name in ('WebInstaller.html', 'index.html'):
    html = (R / name).read_text()
    firmware_b64 = re.search(r'<script id="firmware"[^>]*>(.*?)</script>', html, re.S).group(1)
    assert base64.b64decode(firmware_b64) == new_fw
    assert f"const PKG_HASH='{sha(new_fw)}'" in html
    assert f'const PKG_LEN={len(new_fw)}' in html
    assert 'Stop at the FIRST NUL' in html and 'read-only identity queries' in html
    assert 'V1.89' in html
scripts = re.findall(r'<script\b[^>]*>([\s\S]*?)</script>', (R / 'WebInstaller.html').read_text())
assert len(scripts) == 3
assert sha(scripts[1].replace('V1.89', 'V1.88.3').encode()) == 'e2199e0a69077652c22281c5aaf67bcf9f143367aaf632bbc8f09041b4ce007e'
old_hash = sha(old_fw)
normal_app = scripts[2].replace('V1.89', 'V1.88.3').replace(sha(new_fw), old_hash)
assert sha(normal_app.encode()) == '83ac6701b7439ce1b3166587dc2a5856ed25788d684bf1db1597d4630f35a007'

print('PASS V1.89 POPFIX source: 3/3 supplied C changes semantically exact')
print('PASS inherited V1.88.3 ENV, 13 DSP/MIDI/BEND/flash source files unchanged')
print('PASS preserved both JEDEC IDs, trusted OTA loader and protected bootheader')
print('PASS firmware decryption parity and both installers with original Web MIDI/OTA JS')
