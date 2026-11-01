from pathlib import Path
import sys,hashlib
r=Path('/mnt/data/rupture-v137-work')
sys.path.insert(0,str(r/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
reference=r/'RUPTURE-V1.36.fwsc'
assets=fw.extract_stock_assets(reference)
old_app=(r/'native-build/app.bin').read_bytes()
flash=fw.build_flash_image_from_assets(old_app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
old_flash=next(e['data'] for e in fw.parse_ufw(reference.read_bytes())['entries'] if e['name']=='flash.bin')
print('flash original vs regenerated',len(flash),len(old_flash),sum(x!=y for x,y in zip(flash,old_flash)))
flash=bytearray(flash);flash[:0x4000]=old_flash[:0x4000]
old_rebuilt=fw.build_fwsc(bytes(flash),assets['ota'],'FM-1_968')
print('old package parity after preserved header',old_rebuilt==reference.read_bytes(),len(old_rebuilt),len(reference.read_bytes()))
print('new app exists', (r/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').stat().st_size)
