"""Verify 60 original 1-bit PNG files are byte-for-byte equal to firmware ROM arrays.
V1.58+ shake moves intact images in the UI. The older on-screen tests that
assume zero shake are intentionally superseded by test_v165_sprite_mapping.c.
"""
from pathlib import Path
from PIL import Image
import re
root=Path(__file__).resolve().parents[1]
code=root/'vendor/etch-v1.88-source/firmware/fm1-native'
def bytes_of(name, header):
    s=(code/header).read_text()
    match=re.search(r'static const uint8_t '+name+r'\[20\]\[30\]\[28\]\s*=\s*\{(.*?)\};',s,re.S)
    assert match,name
    data=bytes(int(n,16) for n in re.findall(r'0x([0-9a-fA-F]{2})',match.group(1)))
    assert len(data)==20*30*28,(name,len(data))
    return data
for name,header,folder in [
 ('ru_lfo_sprites','rupture_panel_sprites.h','assets/panel-animations-v156/lfo'),
 ('ru_env_sprites','rupture_panel_sprites.h','assets/panel-animations-v156/env'),
 ('ru_volcano_sprites','rupture_volcano_sprites.h','assets/panel-animations-v157/volcano')]:
    data=bytes_of(name,header)
    for i in range(20):
        path=root/folder/f'{i+1:02d}.png'
        im=Image.open(path).convert('1')
        assert im.size==(224,30),(path,im.size)
        actual=im.tobytes()
        expected=data[i*840:(i+1)*840]
        assert actual==expected, f'{name} frame {i+1} differs from source PNG'
print('PASS V1.65: 60/60 source PNGs bit-identical to ROM sprite arrays (no rescaling or redraw)')
