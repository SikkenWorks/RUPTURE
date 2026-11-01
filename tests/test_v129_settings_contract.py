"""Verify exact native input branches and absence of an AUTO SAVE modal."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=r/'vendor/etch-v1.88-source/firmware/fm1-native'
b=(p/'bringup.c').read_text();ui=(p/'rupture_ui.c').read_text()
settings=b[b.index(' else if(ru_page==RU_PAGE_SETTINGS){\n  if(ru_settings_open==0u){'):b.index(' else if(ru_page==RU_PAGE_ABOUT){',b.index(' else if(ru_page==RU_PAGE_SETTINGS){\n  if(ru_settings_open==0u){'))]
assert 'if(ru_settings_selected==0u)' in settings
assert 'ru_settings_open=1u;' in settings
assert 'else if(ru_settings_selected==1u)' in settings
assert 'ru_autosave_enabled^=1u;' in settings
assert 'ru_global_gain_dirty=1u;' in settings
assert 'ru_settings_open=2u' not in b
assert 'ru_auto_choice' not in b
assert 'ru_auto_choice' not in ui[ui.index(' if(page==RU_PAGE_SETTINGS){'):]
assert '(ru_settings_open==1u || (ru_settings_open==0u && ru_settings_selected==0u))' in b
assert 'if(select)ru_adjust(0,select);' in settings
assert 'ru_screensaver_enabled^=1u;' in b
assert '"AUDIO DIAGNOSTICS","ABOUT"' in ui
assert '"AUDIO DIAG","ABOUT"' not in ui
assert '"SEL TOGGLE AUTO SAVE"' in ui
assert '"K1 VOLUME  SEL OPEN"' in ui
readme=(r/'README.md').read_text()
assert 'K1 immediately adjusts the percentage' in readme
assert 'directly with SEL' in readme
assert 'AUDIO DIAGNOSTICS' in readme
assert not any(z in readme for z in ('# V1.28', '# V1.29', '## CHANGELOG'))
print('PASS V1.29: native page routing and nonmodal toggles / volume K1 policy / manual scope')
