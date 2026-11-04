from pathlib import Path
p=Path(__file__).resolve().parents[1]
s=(p/'vendor/etch-v1.88-source/firmware/fm1-native/bringup.c').read_text()
ui=(p/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c').read_text()
assert 'else if(ru_page==RU_PAGE_PLAY && select)' in s
assert 'else if(ru_page==RU_PAGE_MENU)' in s
assert 'else if(select && ru_page!=RU_PAGE_SAFE' in s
assert 'ru_nav_to(RU_PAGE_MENU);' in s
assert 'ru_focus=(uint8_t)ui_bound(' not in s
for x in ['RU_PAGE_SEQ','RU_PAGE_FX','RU_PAGE_SETTINGS','RU_PAGE_HELP','RU_PAGE_PROJECTS','RU_PAGE_SEQ_SLOTS','RU_PAGE_NAME']:
 assert f'else if(ru_page=={x})' in s or f'if(ru_page=={x})' in s, x
assert 'SELECT TO SCROLL' in ui and 'static const char *const options[]={' in ui
assert 'ru_page!=RU_PAGE_UPDATE' in s and 'ru_page!=RU_PAGE_WELCOME' in s
print('PASS V1.77 navigation: same PLAY menu, specialized SELECT routes preserved, protected pages excluded')
