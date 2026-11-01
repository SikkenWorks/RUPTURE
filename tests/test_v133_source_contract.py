from pathlib import Path
root=Path(__file__).resolve().parents[1]
b=(root/'vendor/etch-v1.88-source/firmware/fm1-native/bringup.c').read_text()
u=(root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c').read_text()
w=(root/'vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c').read_text()
assert 'if(fresh)ru_seq_confirm=1u' in b
assert 'ru_name_new_slot' in b
assert 'named->valid|=0x40u' in b
assert 'native_tempo=saved->tempo' in b
assert 'native_seq_length=len' in b
assert 'native_parameter_publish()' in b
assert 'v.active_project=' in b and 'v.active_seq_slot=' in b
assert '"ACTIVE",175,y' in u
assert 'ru_center(a,180,133,1)' in u and 'ru_center(hold,180,145,1)' in u
assert "name[4]==' '?5u:4u" in b and "name[4]==' '?5u:4u" in w
print('PASS V1.33 exact native SAVE/LOAD and UI contract')
