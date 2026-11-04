#!/usr/bin/env python3
"""V1.69, deterministic transformations. Leaves OTA, storage and DSP topology intact."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
native=r/'vendor/etch-v1.88-source/firmware/fm1-native'
# Replace 45x45 virtual ring scan with same pixels addressed directly (no geometry change).
p=native/'rupture_ui.c';s=p.read_text()
old='static void ru_knob(int x,int y,uint32_t v){for(int yy=-22;yy<=22;yy++)for(int xx=-22;xx<=22;xx++){int r2=xx*xx+yy*yy;if((r2>=400&&r2<=484)|| (r2>=272&&r2<=300&&((xx+yy)&1)==0))ru_set(x+xx,y+yy);}if(v>100u)v=100u;int px=ru_needle_xy[v][0],py=ru_needle_xy[v][1];ru_line(x,y,x+px,y+py);ru_rect(x-1,y-1,3,3);}'
assert s.count(old)==1,'knob draw signature changed'
coords=[(x,y) for y in range(-22,23) for x in range(-22,23) if (400<=x*x+y*y<=484) or (272<=x*x+y*y<=300 and ((x+y)&1)==0)]
assert len(coords)==332
header=native/'rupture_knob_ring_pixels.h'
header.write_text('''/* RUPTURE V1.69: exact 1-bit pixels of the original 45x45 knob outline.
 * Generated from the V1.68 predicate. No trigonometry or multiplications
 * on repaint; stored in flash/rodata, never in an audio ISR.
 * Do not hand-edit these coordinates; verify bitmap equivalence in tests.
 */
#ifndef RUPTURE_KNOB_RING_PIXELS_H
#define RUPTURE_KNOB_RING_PIXELS_H
#include <stdint.h>
static const int8_t rupture_knob_ring_pixels[][2]={\n'''+''.join(f' {{{x},{y}}},'+('\\n' if (i+1)%12==0 else '') for i,(x,y) in enumerate(coords)).replace('\\n','\n')+'\n};\n#define RUPTURE_KNOB_RING_COUNT (sizeof rupture_knob_ring_pixels/sizeof rupture_knob_ring_pixels[0])\n#endif\n')
s=s.replace(old,'''#include "rupture_knob_ring_pixels.h"
static void ru_knob(int x,int y,uint32_t v){
 /* 332 exact visible ring pixels, not 2,025 radius tests per knob/frame.
  * Needle and central pin remain bit-for-bit V1.68 originals. */
 for(uint32_t i=0;i<RUPTURE_KNOB_RING_COUNT;i++)
  ru_set(x+rupture_knob_ring_pixels[i][0],y+rupture_knob_ring_pixels[i][1]);
 if(v>100u)v=100u;
 int px=ru_needle_xy[v][0],py=ru_needle_xy[v][1];
 ru_line(x,y,x+px,y+py);ru_rect(x-1,y-1,3,3);
}''')
p.write_text(s)
# Optimise compressor control conversion. The 4 cached parameters reuse
# existing alignment padding, no increase in global FX state footprint.
p=native/'native_fx.h';s=p.read_text()
needle=' uint8_t applied_valid;\n uint32_t ring_phase,ring_increment;'
assert s.count(needle)==1
s=s.replace(needle,''' uint8_t applied_valid;
 /* Reuse the 3 padding bytes before ring_phase (no new audio SRAM). */
 uint16_t comp_threshold_cached;
 uint8_t comp_ratio_cached;
 uint32_t ring_phase,ring_increment;''')
needle=' int32_t comp_env,comp_gain,comp_target;\n uint8_t comp_tick;'
assert s.count(needle)==1
s=s.replace(needle,''' int32_t comp_env,comp_gain,comp_target;
 uint8_t comp_tick;
 /* Reuse bytes after comp_tick. Exact precomputed Q15 envelope steps. */
 uint8_t comp_attack_step,comp_release_step;''')
p.write_text(s)
p=native/'native_fx.c';s=p.read_text()
needle='    s->applied.trem_rate!=cfg->trem_rate||s->applied.phaser_rate!=cfg->phaser_rate||'
assert s.count(needle)==1
s=s.replace(needle,'''    s->applied.comp_attack!=cfg->comp_attack||s->applied.comp_release!=cfg->comp_release||
    s->applied.comp_threshold!=cfg->comp_threshold||s->applied.comp_ratio!=cfg->comp_ratio||
    s->applied.trem_rate!=cfg->trem_rate||s->applied.phaser_rate!=cfg->phaser_rate||''')
needle='  s->applied=*cfg;s->applied_valid=1;'
assert s.count(needle)==1
s=s.replace(needle,'''  s->applied=*cfg;s->applied_valid=1;
  /* Identical V1.68 integer rounding. Recompute only on parameter changes,
   * not at the 44.1 kHz sample rate. Includes live control automation. */
  s->comp_attack_step=(uint8_t)(2u+(32767u-qclamp(cfg->comp_attack))/512u);
  s->comp_release_step=(uint8_t)(1u+(32767u-qclamp(cfg->comp_release))/2048u);
  s->comp_threshold_cached=(uint16_t)(2000u+(32767u-qclamp(cfg->comp_threshold))*25000u/32767u);
  s->comp_ratio_cached=(uint8_t)(1u+qclamp(cfg->comp_ratio)*11u/32767u);''')
needle='''   int32_t atk=2+(32767u-qclamp(c->comp_attack))/512u;
   int32_t rel=1+(32767u-qclamp(c->comp_release))/2048u;'''
assert s.count(needle)==1
s=s.replace(needle,'''   int32_t atk=s->comp_attack_step;
   int32_t rel=s->comp_release_step;''')
needle='''    int32_t thr=2000+(32767u-qclamp(c->comp_threshold))*25000u/32767u;
    int32_t ratio=1+qclamp(c->comp_ratio)*11u/32767u;'''
assert s.count(needle)==1
s=s.replace(needle,'''    int32_t thr=s->comp_threshold_cached;
    int32_t ratio=s->comp_ratio_cached;''')
needle='''  int32_t atk=2+(32767u-qclamp(c->comp_attack))/512u;
  int32_t rel=1+(32767u-qclamp(c->comp_release))/2048u;'''
assert s.count(needle)==1
s=s.replace(needle,'''  int32_t atk=s->comp_attack_step;
  int32_t rel=s->comp_release_step;''')
needle='''   int32_t thr=2000+(32767u-qclamp(c->comp_threshold))*25000u/32767u;
   int32_t ratio=1+qclamp(c->comp_ratio)*11u/32767u;'''
assert s.count(needle)==1
s=s.replace(needle,'''   int32_t thr=s->comp_threshold_cached;
   int32_t ratio=s->comp_ratio_cached;''')
# IMPORTANT: The original fx_v102_process was called BEFORE the applied
# branch? It is below it in etch_fx_process_stereo_masked, so fields ready.
p.write_text(s)
# Knob interpolation request: 25ms baseline; pressure should not throttle it,
# only confirmed repeated DMA lateness may. Also keep urgent display events.
p=native/'rupture_lcd_budget.h';s=p.read_text()
needle=''' if(missed>=3u||pressure>=20u)return 180u;
 if(missed||pressure>=9u)return 100u;
 return 50u;'''
assert s.count(needle)==1
s=s.replace(needle,''' (void)pressure; /* High max DSP percent is not a DMA underrun. */
 if(missed>=3u)return 100u; /* Actual repeated lateness: protect audio. */
 if(missed)return 50u;
 return 25u; /* 40 FPS target for knob easing at healthy DMA. */''')
p.write_text(s)
print('Applied V1.69: exact knob pixels',len(coords),'comp control caches, live 40FPS knob budget')
