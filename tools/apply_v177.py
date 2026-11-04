#!/usr/bin/env python3
from pathlib import Path
r=Path(__file__).resolve().parents[1]
base=r/'vendor/etch-v1.88-source/firmware/fm1-native'

def patch(path,old,new):
 p=r/path;txt=p.read_text();n=txt.count(old);assert n==1,(str(path),n);p.write_text(txt.replace(old,new))
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/bringup.c'),
' else if(select){ru_focus=(uint8_t)ui_bound((int32_t)ru_focus+(select>0?1:-1),0,3);first_frame=1;}',
''' else if(select && ru_page!=RU_PAGE_SAFE && ru_page!=RU_PAGE_UPDATE &&
         ru_page!=RU_PAGE_LOADING && ru_page!=RU_PAGE_WELCOME &&
         ru_page!=RU_PAGE_FOREIGN && ru_page!=RU_PAGE_BUG_REPORT){
  /* V1.77: The former focus variable was never consumed by the UI.
   * On pages with NO SELECT mapping, open the exact PLAY screen list.
   * Earlier page handlers own SELECT for SEQ, FX, menus and editors.
   * Never preempt OTA, recovery or first-run dialogs. */
  ru_nav_to(RU_PAGE_MENU);
  ru_menu=(uint8_t)ui_bound((int32_t)ru_menu+(select>0?1:-1),0,10);
 }''')
# Warm, genuine low harmonics from the same bandlimited SAW: change existing high-pass blend
# and soften original clipping stages, no extra oscillator, table, or filter state.
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/rupture_saw_tone.h'),
' int32_t wet=low+(((raw-low)*((int32_t)bright*5/2))>>8);',
''' /* V1.77: most of the BRIGHT range now covers warm, low-harmonic
  * bass instead of feeding a fixed ~57% unfiltered saw at its default.
  * 0..60: genuinely rounded; 61..100: progressively open and brilliant.
  * One existing multiply/shift replaces the old expression, no DSP state. */
 int32_t bright_q8=bright<=60u?(12+(int32_t)bright):(72+((int32_t)bright-60)*4);
 int32_t wet=low+(((raw-low)*bright_q8)>>8);''')
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/rupture_saw_tone.h'),
' int32_t drive=256+((int32_t)edge-50)*3;',
' int32_t drive=256+((int32_t)edge-50)*2; /* more usable drive travel */')
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/rupture_saw_tone.h'),
'  wet=(wet*(256+((int32_t)shape_edge-50)*2))>>8;',
'  wet=(wet*(256+((int32_t)shape_edge-50)))>>8;')
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/rupture_saw_tone.h'),
'  wet+=(low*(int32_t)shape_weight)>>7;',
'  wet+=(low*(int32_t)shape_weight)>>8; /* preserves low-end headroom */')
# Reverb: make existing tank sound wider and noticeably present at MIX=100.
# Do NOT introduce extra taps, buffers, division or multiplications in audio.
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c'),
'  uint32_t rsize=8192u+(uint32_t)qclamp(cfg->reverb_size)*24575u/32767u;\n  s->tap1=1559u*rsize/32767u;s->tap2=2707u*rsize/32767u;s->tap3=4019u*rsize/32767u;',
'''  /* V1.77: wider tap span at the three existing TYPE presets. All
   * mapping divisions occur only when controls change, NEVER per sample.
   * SPRING short/bright, ROOM intermediate, HALL longer/diffuse. */
  uint32_t rsize=4096u+(uint32_t)qclamp(cfg->reverb_size)*28671u/32767u;
  s->tap1=763u*rsize/32767u;s->tap2=2207u*rsize/32767u;s->tap3=4019u*rsize/32767u;''')
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c'),
' s->reverb[s->reverb_pos]=clip16(diffused+(((s->space_damp_state*(int32_t)decay)>>15)*27)/32);',
''' /* Same one feedback multiply as before; less abrupt decay and a
  * clearly longer hall tail without increasing realtime work. */
 s->reverb[s->reverb_pos]=clip16(diffused+(((s->space_damp_state*(int32_t)decay)>>15)*31)/32);''')
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c'),
''' int32_t wet_left=clip16((early_a*5+early_b*3+r1*4+r2*3+r3*2)/12);
 int32_t wet_right=clip16((early_b*5+early_a*2+r2*4+r3*3+r1*2)/12);''',
''' /* V1.77: previous wet return was severely under-levelled. Rebalance
  * the SAME five reads and five weight terms, no extra DSP operations.
  * Full wet remains wet-only but now has usable amplitude; clip16 caps peaks. */
 int32_t wet_left=clip16((early_a*7+early_b*5+r1*6+r2*4+r3*3)/6);
 int32_t wet_right=clip16((early_b*7+early_a*4+r2*6+r3*4+r1*3)/6);''')
# Version in native display and native build metadata
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c'),
'"V1.76","FM-1_968"','"V1.77","FM-1_968"')
patch(Path('vendor/etch-v1.88-source/firmware/fm1-native/build.py'),
"'version':'RUPTURE V1.76'","'version':'RUPTURE V1.77'")
print('V1.77 native changes applied')
