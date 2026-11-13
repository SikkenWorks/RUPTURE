#!/usr/bin/env python3
"""Package native-built RUPTURE V1.86 using untouched trusted V1.85.2 OTA/boot assets.

Invoke after: JIELI_TOOLCHAIN=... python3 firmware/fm1-native/build.py --storage --updater --usb-audio-48k
"""
from pathlib import Path
import base64, hashlib, json, re, shutil, sys

root = Path(__file__).resolve().parents[1]
base = root / 'reference/RUPTURE-V1.85.2.fwsc'
app_path = root / 'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin'
assert base.is_file(), f'Missing old verified OTA reference: {base}'
assert app_path.is_file(), f'Build the FM1 app before packaging: {app_path}'
app = app_path.read_bytes()
sha = lambda b: hashlib.sha256(b).hexdigest()
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
assert fw.verify_fwsc(base)[1] == [], 'Original V1.85.2 FWSC invalid'
assert not fw.validate_app_binary(app), 'New app failed FM1 format validation'
assets = fw.extract_stock_assets(base)
assert assets['product'] == 'FM-1_968'
old = {e['name']:e['data'] for e in fw.parse_ufw(base.read_bytes())['entries']}
flash = fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],
    isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert len(flash) == len(old['flash.bin']) == 0x93000
assert flash[:0x4000] == old['flash.bin'][:0x4000], 'Protected boot/header changed unexpectedly'
pack = fw.build_fwsc(flash, assets['ota'], 'FM-1_968')
release_file = root/'RUPTURE-V1.86.fwsc'
release_file.write_bytes(pack)
assert fw.verify_fwsc(release_file)[1] == [], 'New FWSC failed verification'
new = {e['name']:e['data'] for e in fw.parse_ufw(pack)['entries']}
assert new['ota.bin'] == old['ota.bin'], 'Trusted OTA loader changed unexpectedly'
assert new['flash.bin'][:0x4000] == old['flash.bin'][:0x4000]
assert new['flash.bin'] == flash

# Update both application build copies and sidecar provenance, not only a stale ZIP filename.
appdir = app_path.parent
for name in ('app.bin','app.bin.audit.json','native-build-options.json'):
    shutil.copy2(appdir/name, root/'native-build'/name)
metadata = json.loads((root/'native-build/native-build-options.json').read_text('utf-8'))
assert metadata['version']=='RUPTURE V1.86'
assert metadata['app_sha256']==sha(app)
manifest_path = root/'RELEASE-MANIFEST.json'
manifest = json.loads(manifest_path.read_text('utf-8'))
manifest.update(
    version='1.86', firmware_file=release_file.name, firmware_bytes=len(pack), firmware_sha256=sha(pack),
    app_bytes=len(app), app_sha256=sha(app), physical_fm1_verified=False, hardware_verified=False,
    release_ready=False, release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',
    native_screens=126,
    release_feature='BEND play-parameter screen: FX long press / K1 momentary string bend / K2 5 range targets',
    release_note='V1.86: BEND Play Parameter on FX long press; K1 transient positive pitch bend (max 2 semitones), K2 range selector, 8 offline-packed 1-bit mountain frames, live status. Bend at oscillator before all FX. Retains V1.85.2 flash-chip compatibility, existing reverb/SEQ, OTA writer, trusted loader and boot header.'
)
manifest['bend_v186'] = {
    'entry':'FX hold >=700 ms', 'exit':'HOME or normal page navigation', 'k1':'live upward bend with auto release',
    'k2_targets':['1/4','1/2','FULL','1 1/2','DOUBLE'],
    'max_semitones':2.0, 'sprite_frames':8, 'sprite_pixels':[224,30],
    'lcd_pixels':[240,240], 'crt_on_off':True, 'fx_position':'pre-FX oscillator pitch',
    'store_persistent':False, 'hardware_verified':False,
}
warning = ('V1.86: BEND is cross-built and host-tested, but physical FM1 FX-hold timing, K1/K2 response, '
           'actual bend return feel, rapid notes with DMA margin and post-USB-C speaker pop remain unverified. '
           'Back up all projects and keep the tested recovery route available before flashing.')
if warning not in manifest.setdefault('limitations',[]):manifest['limitations'].append(warning)
manifest_path.write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n', 'utf-8')

installer = root/'WebInstaller.html'
s = installer.read_text('utf-8')
assert 'V1.85.2' in s, 'Unexpected prior installer version'
s = s.replace('V1.85.2','V1.86')
s,n = re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)', lambda m: m.group(1)+base64.b64encode(pack).decode('ascii')+m.group(2),s,count=1,flags=re.S)
assert n==1
s,n = re.subn(r"const PKG_HASH='[0-9a-f]{64}'", f"const PKG_HASH='{sha(pack)}'",s)
assert n==1
s,n = re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pack)}',s)
assert n==1
installer.write_text(s,'utf-8')
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',s,re.S).group(1))==pack
assert s.count('V1.86')>2
print('PASS: boot header unchanged SHA256',sha(flash[:0x4000]))
print('PASS: OTA loader unchanged SHA256',sha(new['ota.bin']))
print('PASS: V1.86 firmware size',len(pack),'SHA256',sha(pack))
print('PASS: application bytes',len(app),'SHA256',sha(app),'free',581564-len(app))
print('PASS: WebInstaller embedded package matches .fwsc')
