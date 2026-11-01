from pathlib import Path
root=Path(__file__).resolve().parents[1]
d=root/'vendor/etch-v1.88-source/firmware/fm1-native'
def change(name,pairs):
 p=d/name;s=p.read_text()
 for a,b in pairs:
  if a not in s: raise RuntimeError('Missing anchor '+name+': '+a[:100])
  s=s.replace(a,b,1)
 p.write_text(s)
# Store new main mode using pre-existing reserved bit 4; bit 5 stays invalid.
(d/'rupture_main_state.h').write_text('''/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.45: main oscillator selector in unused DSP bit 4.
 * No new flash records. All old projects restore CXS.
 */
#ifndef RU_MAIN_STATE_H
#define RU_MAIN_STATE_H
#include <stdint.h>
#include "native_project_codec.h"
#define RU_MAIN_SAW_TAG 0x10u
static inline void ru_main_state_store(struct etch_preset_dsp *v,uint8_t wave){
 if(wave)v->spectral_wave|=RU_MAIN_SAW_TAG;
 else v->spectral_wave&=(uint8_t)~RU_MAIN_SAW_TAG;
}
static inline uint8_t ru_main_state_restore(const struct etch_preset_dsp *v){
 return (uint8_t)!!(v->spectral_wave&RU_MAIN_SAW_TAG);
}
#endif
''')
change('native_project_codec.c', [
 ('#include "rupture_sub_state.h"','#include "rupture_sub_state.h"\n#include "rupture_main_state.h"'),
 ('#define ETCH_SPECTRAL_HI_INVALID(v) (((v)&0x30u)!=0u)', '#define ETCH_SPECTRAL_HI_INVALID(v) (((v)&0x20u)!=0u) /* bit4 = RUPTURE SAW; bit5 reserved */')
])
change('rupture_sub_state.h',[
 ('v->matrix.route[0].amount<=2u', 'v->matrix.route[0].amount<=4u'),
 ('v->living.rate<=2u', 'v->living.rate<=4u'),
 ('type[0]<=2u?type[0]:0u','type[0]<=4u?type[0]:0u'),
 ('type[1]<=2u?type[1]:0u','type[1]<=4u?type[1]:0u')
])
change('rupture_subosc.h',[
 ('#include "native_cxs_q15.h"', '#include "native_cxs_q15.h"\n#include "rupture_bandwaves.h"'),
 ('#define RU_SUB_CLASSIC 2u', '#define RU_SUB_CLASSIC 2u\n#define RU_SUB_SQUARE RU_WAVE_SQUARE\n#define RU_SUB_SAW RU_WAVE_SAW'),
 ('if(type>2u)type=0u;', 'if(type>4u)type=0u;'),
 ('if(s->running_type==RU_SUB_CLASSIC)return ru_sub_classic_read(s->phase,increment);', 'if(s->running_type==RU_SUB_CLASSIC)return ru_sub_classic_read(s->phase,increment);\n if(s->running_type==RU_SUB_SQUARE||s->running_type==RU_SUB_SAW)\n  return ru_bandwave_read(s->running_type,s->phase,increment);')
])
change('audio_core.h',[
 ('uint32_t cxs_lfo_phase;int32_t cxs_body_low;', 'uint32_t cxs_lfo_phase;int32_t cxs_body_low;uint16_t main_saw_mix;'),
 ('uint8_t cxs_pressure,cxs_balance,cxs_memory,cxs_density;', 'uint8_t cxs_pressure,cxs_balance,cxs_memory,cxs_density,main_wave;')
])
change('audio_core.c',[
 ('rupture_q15_init(&s->cxs,target);\n ru_sub_init', 'rupture_q15_init(&s->cxs,target);\n /* On a new note begin in the selected engine; no extra transition CPU. */\n s->main_saw_mix=0u;\n ru_sub_init'),
 ('''   int32_t sample=rupture_q15_sample(&s->cxs);
   sample=rupture_shape_sample(&s->cxs_body_low,sample,c->cxs_weight,c->cxs_bite,
                  c->cxs_punch,s->cxs.cycles);''','''   /* A single carrier: CXS or bandlimited SAW. During live mode switching,
    * run both for a short 64-sample fade; in steady SAW only one constant-time
    * lookup is used, while CXS phase continues without CXS computation. */
   uint16_t target_mix=c->main_wave?32768u:0u;
   if(s->main_saw_mix<target_mix){
    uint32_t n=(uint32_t)s->main_saw_mix+512u;
    s->main_saw_mix=(uint16_t)(n>target_mix?target_mix:n);
   }else if(s->main_saw_mix>target_mix){
    s->main_saw_mix=s->main_saw_mix>512u?(uint16_t)(s->main_saw_mix-512u):0u;
   }
   uint16_t blend=s->main_saw_mix;
   int32_t sample=0;
   if(blend!=32768u){
    sample=rupture_q15_sample(&s->cxs);
    sample=rupture_shape_sample(&s->cxs_body_low,sample,c->cxs_weight,c->cxs_bite,
                    c->cxs_punch,s->cxs.cycles);
   }else{
    uint32_t old=s->cxs.phase;
    s->cxs.phase+=pitch_inc;
    if(s->cxs.phase<old)s->cxs.cycles++;
   }
   if(blend){
    int32_t saw=ru_bandwave_read(RU_WAVE_SAW,s->cxs.phase,pitch_inc);
    /* Fixed-point Q15 crossfade, products <= 2^30 each; no divide. */
    sample=(int32_t)(((sample*(int32_t)(32768u-blend))+(saw*(int32_t)blend))>>15);
   }''')
])
change('bringup.c',[
 ('#include "rupture_sub_state.h"','#include "rupture_sub_state.h"\n#include "rupture_main_state.h"') if '#include "rupture_sub_state.h"' in (d/'bringup.c').read_text() else ('#include "rupture_shape_state.h"','#include "rupture_shape_state.h"\n#include "rupture_main_state.h"'),
 ('static uint8_t ru_saver_frame=0u,ru_saver_active=0u;', 'static uint8_t ru_saver_frame=0u,ru_saver_active=0u,ru_saver_reverse=0u;\nstatic uint32_t ru_saver_hold_until_ms=0u;'),
 ('static uint8_t ru_pressure=58,ru_balance=50,ru_memory=52,ru_density=68;', 'static uint8_t ru_pressure=58,ru_balance=50,ru_memory=52,ru_density=68;\nstatic uint8_t ru_main_wave=0u; /* 0 CXS, 1 SAW; preserved per project/preset */'),
 ('/* SUB pages: TYPE OFF/CXS/CLASSIC, OCT', '/* SUB pages: TYPE OFF/CXS/CLASSIC/SQUARE/SAW, OCT'),
 ('native_controls.cxs_weight=ru_weight;', 'native_controls.main_wave=ru_main_wave;\n native_controls.cxs_weight=ru_weight;'),
 (' ru_sub_state_store(v,ru_sub_type,ru_sub_oct,ru_sub_detune,ru_sub_drift);',' ru_sub_state_store(v,ru_sub_type,ru_sub_oct,ru_sub_detune,ru_sub_drift);\n ru_main_state_store(v,ru_main_wave);'),
 (' (void)ru_sub_state_restore(v,ru_sub_type,ru_sub_oct,ru_sub_detune,ru_sub_drift);',' (void)ru_sub_state_restore(v,ru_sub_type,ru_sub_oct,ru_sub_detune,ru_sub_drift);\n ru_main_wave=ru_patch_is_cxs(&native_library.patches[slot])?ru_main_state_restore(v):0u;'),
 ('if(control==0u)ru_sub_type[z]=(uint8_t)ui_bound((int32_t)ru_sub_type[z]+amount,0,2);', 'if(control==0u)ru_sub_type[z]=(uint8_t)ui_bound((int32_t)ru_sub_type[z]+amount,0,4);'),
 ('''    ru_mark_dirty();first_frame=1u;
   }/* ALGORITHM is inert on every page without a dedicated mapping. */''','''    ru_mark_dirty();first_frame=1u;
   }else if(ru_page==RU_PAGE_PLAY||ru_page==RU_PAGE_SHAPE){
    ru_main_wave=(uint8_t)(alg>0?1u:0u);
    ru_mark_dirty();first_frame=1u;
   }/* Other pages retain their own ALGORITHM mappings. */'''),
 ('native_controls.cxs_pressure=ru_pressure;native_controls.cxs_balance=ru_balance;', 'native_controls.main_wave=ru_main_wave;\n native_controls.cxs_pressure=ru_pressure;native_controls.cxs_balance=ru_balance;'),
 ('if(ru_saver_active){ru_saver_active=0u;ru_saver_frame=0u;first_frame=1u;}', 'if(ru_saver_active){ru_saver_active=0u;ru_saver_frame=0u;ru_saver_reverse=0u;ru_saver_hold_until_ms=0u;first_frame=1u;}'),
 ('if(!ru_saver_active){ru_saver_active=1u;ru_saver_frame=0u;ru_saver_last_frame_ms=etch_ms;first_frame=1u;}', 'if(!ru_saver_active){ru_saver_active=1u;ru_saver_frame=0u;ru_saver_reverse=0u;ru_saver_hold_until_ms=0u;ru_saver_last_frame_ms=etch_ms;first_frame=1u;}'),
 ('''      if(ru_saver_frame<63u){ru_saver_frame++;first_frame=1u;}''','''      if(ru_saver_hold_until_ms){
       if((int32_t)(etch_ms-ru_saver_hold_until_ms)>=0){ru_saver_hold_until_ms=0u;ru_saver_reverse=1u;}
      }else if(ru_saver_reverse){
       if(ru_saver_frame>0u){ru_saver_frame--;first_frame=1u;}
       if(ru_saver_frame==0u)ru_saver_reverse=0u;
      }else{
       if(ru_saver_frame<63u){ru_saver_frame++;first_frame=1u;}
       if(ru_saver_frame==63u)ru_saver_hold_until_ms=etch_ms+2200u;
      }''')
])
# Move default immediately to brand new project, don't inherit prior project's mode.
change('bringup.c',[
 ('native_mod_mode=0;native_mod_ratio=3;\n native_page=ETCH_UI_PLAY;', 'native_mod_mode=0;native_mod_ratio=3;\n#ifdef RUPTURE_BUILD\n ru_main_wave=0u; /* NEW always starts at CXS */\n#endif\n native_page=ETCH_UI_PLAY;'),
 ('native_controls.cxs_pressure=58u;native_controls.cxs_balance=50u;', 'native_controls.main_wave=ru_main_wave;\n native_controls.cxs_pressure=58u;native_controls.cxs_balance=50u;'),
 ('v.saver_active=ru_saver_active;v.saver_frame=ru_saver_frame;', 'v.saver_active=ru_saver_active;v.saver_frame=ru_saver_frame;v.main_wave=ru_main_wave;')
])
change('rupture_ui.c',[
 ('uint8_t audio_fault,settings_open,auto_save,auto_choice,boot_choice,saver_active,saver_frame,saver_enabled', 'uint8_t audio_fault,settings_open,auto_save,auto_choice,boot_choice,saver_active,saver_frame,main_wave,saver_enabled'),
 ('if(i==0)val=val*50u;else if(i==1)val=val*100u/3u;break;', 'if(i==0)val=val*25u;else if(i==1)val=val*100u/3u;break;'),
 ('static const char *const kinds[]={"OFF","CXS","CLASSIC"};txt=kinds[v->k[0]%3u];', 'static const char *const kinds[]={"OFF","CXS","CLASSIC","SQUARE","SAW"};txt=kinds[v->k[0]%5u];'),
 ('if(page==RU_PAGE_SHAPE){ru_frame(8,50,224,30);ru_center("CXS SOUND SHAPING",120,61,1);ru_draw_knobs(v);ru_footer("HOME SUB 1 / LONG HOLD MAIN");return;}', 'if(page==RU_PAGE_SHAPE){ru_frame(8,50,224,30);ru_center(v->main_wave?"SAW SOUND SHAPING":"CXS SOUND SHAPING",120,61,1);ru_draw_knobs(v);ru_footer("ALG OSC / HOME SUB 1");return;}'),
 ('if(page==RU_PAGE_PLAY){ru_frame(8,50,224,30);', 'if(page==RU_PAGE_PLAY){ru_center(v->main_wave?"SAWTOOTH":"CXS",120,51,1);ru_frame(8,65,224,30);'),
 ('ru_footer("HOME SOUND SHAPE / GLO MENU");return;', 'ru_footer("ALG OSC / HOME SHAPE");return;')
])
print('Modified sources for V1.45')
