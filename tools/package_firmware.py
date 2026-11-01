#!/usr/bin/env python3
"""Rebuild RUPTURE firmware using the archived byte-identical OTA loader and boothead."""
from pathlib import Path
import sys,hashlib,json
root=Path(__file__).resolve().parents[1]
source=root/'vendor/etch-v1.88-source'
sys.path.insert(0,str(source/'tools'))
import fm1_fwsc as fw
app=(source/'build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
reference=root/'reference/ETCH-V1.88.fwsc'
report,errors=fw.verify_fwsc(reference)
assert not errors,(report,errors)
assets=fw.extract_stock_assets(reference)
entries=fw.parse_ufw(reference.read_bytes())['entries']
old_flash=next(e['data'] for e in entries if e['name']=='flash.bin')
loader=next(e['data'] for e in entries if e['name']=='ota.bin')
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],key=assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
flash=old_flash[:0x4000]+flash[0x4000:]
package=fw.build_fwsc(flash,loader,'FM-1_968')
version=json.loads((root/'RELEASE-MANIFEST.json').read_text())['version']
p=root/f'RUPTURE-V{version}.fwsc';p.write_bytes(package)
result,errors=fw.verify_fwsc(p)
assert not errors and result['product']=='FM-1_968' and result['flash']['app_crc_ok'],(result,errors)
assert fw.parse_ufw(package)['entries'][0]['data'][:0x4000]==old_flash[:0x4000]
print('Validated',len(package),'B',hashlib.sha256(package).hexdigest())
