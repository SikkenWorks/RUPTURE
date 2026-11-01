from pathlib import Path
s=Path(__file__).resolve().parents[1].joinpath("vendor/etch-v1.88-source/firmware/fm1-native/bringup.c").read_text()
start=s.index("if(pressed&(1u<<6)){\n  /* EDIT owns Channel EQ navigation")
end=s.index("if(pressed&(1u<<10))",start)
block=s[start:end]
assert "if(ru_page==RU_PAGE_EQ)" in block
assert "ru_eq_tab=(uint8_t)((ru_eq_tab+1u)%3u)" in block
assert "else{ru_nav_to(RU_PAGE_EQ);ru_eq_tab=0u;}" in block
handler=s[s.index(" else if(ru_page==RU_PAGE_EQ){\n  /* SEL"):s.index(" else if(ru_page==RU_PAGE_SEQ){",s.index(" else if(ru_page==RU_PAGE_EQ){\n  /* SEL"))]
assert "if(pressed&(1u<<3))" not in handler
assert "if(alg)" in handler
u=Path(__file__).resolve().parents[1].joinpath("vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c").read_text()
assert 'ru_footer("EDIT NEXT / ALG LEGACY")' in u
print("PASS V1.35: EDIT-only EQ page changes; SEL no page switch; ALGORITHM retained")
