from pathlib import Path
root=Path(__file__).resolve().parents[1]
n=root/'vendor/etch-v1.88-source/firmware/fm1-native'
bring=(n/'bringup.c').read_text();ui=(n/'rupture_ui.c').read_text();usb=(n/'usb.c').read_text();fx=(n/'native_fx.c').read_text();store=(n/'wave_store.c').read_text();mem=(n/'rupture_fx_memory.h').read_text()
assert "static const uint8_t STR1[] = {24, 3, 'S'" in usb
assert "static const uint8_t STR2[] = {16, 3, 'R'" in usb
assert 'if(entry_page==RU_PAGE_NAME && (pressed&(1u<<9))){if(ru_manager_name)ru_manager_commit_name();else ru_commit_name();}' in bring
assert 'ru_footer("SAVE TO STORE")' in ui
assert 'native_store_status!=ETCH_STORE_EMPTY' in bring
assert '((i/2)?181:105)' in ui
assert 'ru_storage_popup(ru_storage_kind' in bring
assert 'ru_storage_begin(1u)' in bring and 'ru_storage_begin(2u)' in bring
assert 'ru_storage_begin(3u)' in bring and 'ru_storage_begin(4u)' in bring
for phase in ('1u','2u','3u','4u'): assert f'ru_store_progress({phase}' in store
assert 'c->eq_hpf<=16384u' in fx
assert '0xc0u' in mem and '0x30u|' in bring
assert 'FM-1_968' not in usb # USB string uses a branded descriptor; OTA identity is in ota.c
print('PASS V1.25: USB RUPTURE branding; SEL no-op; SAVE-only commit; blank-bank SAVE; real storage stages; spaced knobs; signed EQ')
