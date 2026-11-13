#!/usr/bin/env python3
"""V1.88.3 release contract, FM1 OTA safety and unaffected audio source."""
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

sha = lambda b: hashlib.sha256(b).hexdigest()
manifest = json.loads((R / 'RELEASE-MANIFEST.json').read_text())
assert manifest['version'] == '1.88.3'
assert manifest['firmware_file'] == 'RUPTURE-V1.88.3.fwsc'
assert not manifest['hardware_verified'] and not manifest['release_ready']
app = (R / 'native-build/app.bin').read_bytes()
assert not fw.validate_app_binary(app)
assert sha(app) == manifest['app_sha256']
assert len(app) == manifest['app_bytes']
assert app == (N / 'build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
new_fw = (R / manifest['firmware_file']).read_bytes()
old_fw = (R / 'RUPTURE-V1.88.2.fwsc').read_bytes()
assert sha(new_fw) == manifest['firmware_sha256']
assert len(new_fw) == manifest['firmware_bytes'] == len(old_fw) == 610086
assert fw.verify_fwsc(R / manifest['firmware_file'])[1] == []
get_entries = lambda raw: {e['name']: e['data'] for e in fw.parse_ufw(raw)['entries']}
new = get_entries(new_fw)
old = get_entries(old_fw)
assert new['ota.bin'] == old['ota.bin'], 'OTA loader changed'
assert new['flash.bin'][:0x4000] == old['flash.bin'][:0x4000], 'Protected header changed'
assert len(new['flash.bin']) == len(old['flash.bin']) == 0x93000
# The JLFS app area is SFC-XOR encrypted on disk; decode it before parity.
key = fw.extract_stock_assets(R / 'RUPTURE-V1.86.1.fwsc')['key']
area = bytearray(new['flash.bin'][0x4000:0x4000 + 0x120 + len(app)])
fw.sfc_xor(area, 0, len(area), 0, key)
assert area[0x120:] == app

# Source checks against archived byte fingerprints of V1.88.2. All native
# audio/USB/MIDI/OTA/flash/codecs/BEND logic stays byte-identical.
critical = {
    'audio_core.c': '740b88a5dd9dbb7aad7c25b6410811fcd54098cebb1f489646c3c761d235a1c0',
    'audio_core.h': 'fda98bfb3c263bfec0b6db2839ee0bfc44a3f899dafae7cb9a4d069e6b2ec7c5',
    'native_fx.c': '99e37d0564d5fd1bcb95ede763ed2c5218cd06f0c505d87f21e8b82c01feb645',
    'native_fx.h': '68156f459ce3b1e525c937f69c91967ea164177fb70fa9cb765d2634f92226f0',
    'native_cxs_q15.c': '8625829401e27711263b2a66dd18cd6180b2b590752f49214b9d036389a851c1',
    'audio_target.c': 'b8dfa2ef0ced4be23a9f7e36a99d7327ec2867afde711f2df132c3ddc6071d2a',
    'native_panel_midi.c': '29e5d831f453c723e1999cefb95abeddb69d7760c1f1056519fecc2bef90aa2a',
    'usb.c': '3e484359d55c68e7510b3b588315e1e781ca30d3946f7d0fea4288895cb6dbef',
    'ota.c': 'c77217b9a650beda39827a2754e76a8502ee97c8244ce07087e05c87dc9e34e6',
    'ota_target.c': '1d9a9ceb0489d98211d9757eafcd9ed021b9f7d57efa4e0ef3631824f2a44c5a',
    'wave_store_target.c': '2027c9d68ddc801da4541c6fbfd1339824c51db2699ffbb4ffa7124459131813',
    'native_project_codec.c': '12c4e43c006fe8fbb0c9386461b4488fe7cd40814e52ad52c7f1f2822ddcc30d',
    'native_flash_probe.h': '880f9c770c9593667ea8df89fa16d2e0cc7dea943579558308626199f64b95d2',
    'hal/fm1_flash_id.h': 'c77ff4febd1e1efc1cea73e0080aedb7846be431d44dc4983862f3e2c34a0aac',
    'rupture_bend_sprites.h': 'acf8d1a0966567d80ff94fa5767e896db71a8ca94439ebd68781e3dd53922781',
    'rupture_bend_storyboard.h': '43af3e4f4f307b86e21f7d34746f383fd6873a5593601916bf17a91cbef6efc1',
    'rupture_bend_curves.h': '48ce00ca1765cce650dc8fae04beb76ebeb7019f7400ea5cef59ff7465483793',
    'hal/fm1_isr.S': '2aad84e24a51c79b187ef3faf9d1bae108b1389bf78659f6b02dbbd95d8a7d7b',
    'bringup.c': 'b73cb97d6546dc13280d86241fd8a354ab8710708c96c69032b33fba5613864b',
}
for name, expected in critical.items():
    assert sha((SRC / name).read_bytes()) == expected, f'Unintended native change: {name}'
flash_id = (SRC / 'hal/fm1_flash_id.h').read_text()
assert '0x856014u' in flash_id and '0xB36014u' in flash_id

# WebInstaller and GitHub Pages contain identical current packages and retain
# the corrected read-only identity query and NUL-terminated response parser.
for name in ('WebInstaller.html', 'index.html'):
    h = (R / name).read_text()
    encoded = re.search(r'<script id="firmware"[^>]*>(.*?)</script>', h, re.S).group(1)
    assert base64.b64decode(encoded) == new_fw
    assert f"const PKG_HASH='{sha(new_fw)}'" in h
    assert f'const PKG_LEN={len(new_fw)}' in h
    assert 'Stop at the FIRST NUL' in h
    assert 'read-only identity queries' in h
    assert 'V1.88.3' in h
print('PASS V1.88.3 OTA: new app, original loader, 16 KiB header unchanged, both JEDEC IDs')
print('PASS V1.88.3 isolation: 19 native DSP/MIDI/OTA/flash/BEND source files byte-identical to V1.88.2')
print('PASS V1.88.3 website: both installers embed matching FWSC with verified OTA connection fix')
