#!/usr/bin/env python3
"""Three independent, bounded global reverbs with unchanged project record size."""
from pathlib import Path
p=Path(__file__).resolve().parents[1]
r=p/'vendor/etch-v1.88-source/firmware/fm1-native'
def edit(file, old, new, count=1):
 f=r/file; t=f.read_text(); n=t.count(old)
 if n!=count:raise RuntimeError(f'{file}: expected {count}, got {n}: {old[:120]}')
 f.write_text(t.replace(old,new))
# Keep legacy project routing 0/1 valid (and interpreted as HALL), use previously
# unused two bits. Existing 30-byte legacy FX prefix and storage CRC stay intact.
edit('native_fx.h','/* Values are Q15 (0..32767); routing: 0 serial, 1 parallel. */', '''/* V1.78: routing bits 0=serial/parallel, bits 1..2=reverb topology.
 * Old projects (0/1) decode as HALL, preserving their original default.
 * Type code: 0=HALL, 1=SPRING, 2=ROOM; code 3 is invalid.
 * This is the existing 1-byte setting, not an added field. Older firmwares
 * cannot necessarily open projects saved with the new type bits set. */''')
needle='struct etch_native_fx {'
functions='''/* Public UI order: 0 SPRING, 1 HALL, 2 ROOM. */
static inline uint8_t etch_fx_reverb_type(const struct etch_fx_settings *s){
 uint8_t code=(uint8_t)((s->routing>>1)&3u);
 return code==1u?0u:code==2u?2u:1u;
}
static inline void etch_fx_set_reverb_type(struct etch_fx_settings *s,uint8_t type){
 uint8_t code=type==0u?1u:type==2u?2u:0u;
 s->routing=(uint8_t)((s->routing&1u)|(code<<1));
}
static inline uint8_t etch_fx_route(const struct etch_fx_settings *s){
 return (uint8_t)(s->routing&1u);
}
static inline void etch_fx_set_route(struct etch_fx_settings *s,uint8_t route){
 s->routing=(uint8_t)((s->routing&~1u)|(route&1u));
}
static inline uint8_t etch_fx_valid_route_and_type(uint8_t packed){
 return (uint8_t)(packed<=5u && ((packed>>1)&3u)!=3u);
}
'''+needle
edit('native_fx.h',needle,functions)
# Validation on both global and per-patch routes. No codec order or bytes changed.
edit('native_project_codec.c','s->global_fx.routing<=1','etch_fx_valid_route_and_type(s->global_fx.routing)')
edit('native_project_codec.c','fx->routing>1','!etch_fx_valid_route_and_type(fx->routing)')
edit('native_project_codec.c','if(s->global_fx.routing>1)return -1;','if(!etch_fx_valid_route_and_type(s->global_fx.routing))return -1;')
edit('native_project_codec.c','s->preset_fx[i].routing>1','!etch_fx_valid_route_and_type(s->preset_fx[i].routing)')
# K1 changes algorithm only, no automatic changing of DECAY or SIZE.
edit('bringup.c', '''    /* Three distinct working tonal presets of the existing stable reverb
     * architecture, NOT separate physical spring-tank emulations. */
    native_fx_settings.reverb_size=(uint16_t)(new_type==0u?8500u:new_type==1u?30000u:20500u);
    native_fx_settings.reverb_decay=(uint16_t)(new_type==0u?12000u:new_type==1u?30000u:11000u);''',
'''    /* Choose a genuinely separate global topology. Retain the user's own
     * DECAY and SIZE values when switching engines. */
    etch_fx_set_reverb_type(&native_fx_settings,new_type);''')
# Preserve type when using serial/parallel routing controls; decoded route shown in UI.
edit('bringup.c','native_fx_settings.routing=(uint8_t)ui_bound((int32_t)native_fx_settings.routing+amount,0,1);',
'''etch_fx_set_route(&native_fx_settings,(uint8_t)ui_bound((int32_t)etch_fx_route(&native_fx_settings)+amount,0,1));''',2)
for d in ['down','up']:
 old=f'native_fx_settings.routing=(uint8_t)ui_bound((int32_t)native_fx_settings.routing+{d},0,1);'
 num=3 if d=='down' else 1
 edit('bringup.c',old,
      f'etch_fx_set_route(&native_fx_settings,(uint8_t)ui_bound((int32_t)etch_fx_route(&native_fx_settings)+{d},0,1));',num)
# Note the up-control also appears on legacy native page 17, accounted for above.
# One legacy FX reverb control uses down too; total 4 down instances (CHORUS DELAY REVERB). 
# Topology label must follow loaded global/preset FX, not stale UI runtime.
# Insert sync after four assignments including project and preset loads.
for rhs in ['native_project_cfg.global_fx','native_project_cfg.preset_fx[fx_slot]','v->global_fx','native_project_cfg.preset_fx[native_patch_slot]']:
 old='native_fx_settings='+rhs+';'
 edit('bringup.c',old,old+'ru_reverb_type=etch_fx_reverb_type(&native_fx_settings);',2 if rhs=='native_project_cfg.global_fx' else 1)
# Replace all route-only UI fields; applied only to route fields, not metadata/CRC.
f=r/'bringup.c';t=f.read_text()
for before in ['values[3]=native_fx_settings.routing','values[2]=native_fx_settings.routing','v.k[2]=native_fx_settings.routing']:
 t=t.replace(before,before.replace('native_fx_settings.routing','etch_fx_route(&native_fx_settings)'))
f.write_text(t)
# DSP separate engine functions, shared only bounded ring and common wet/dry mixer.
fx=r/'native_fx.c'; txt=fx.read_text(); anchor='void etch_fx_process_stereo_masked('
assert txt.count(anchor)==1
impl='''/* RUPTURE V1.78: Three genuinely distinct global reverb algorithms.
 * Only one runs per audio sample. They reuse the same 4096-word tank, 64-word
 * diffuser, existing damping state and wet/dry mixer: zero new SRAM/DMA buffers.
 * No allocation, floating-point, transcendentals or 64-bit divides here.
 * A type switch preserves the current tail instead of blocking the audio ISR
 * while clearing a 8KB ring. Output is bounded by clip16 and the wet limiter.
 */
static void ru_reverb_spring(struct etch_native_fx *s,int32_t input,uint16_t decay,
                             int32_t *wl,int32_t *wr){
 uint32_t p=s->reverb_pos;
 int32_t a=s->reverb[(p-s->tap1)&4095u];
 int32_t b=s->reverb[(p-s->tap2)&4095u];
 int32_t c=s->reverb[(p-s->tap3)&4095u];
 /* Dispersive, sign-alternating allpass lattice: spring-like metallic twang.
  * The damped feedback taps sum to <0.75 of their maximum amplitude. */
 int32_t dispersion=space_diffuser[(p-29u)&63u];
 int32_t scattered=dispersion-((input*3)>>2);
 space_diffuser[p&63u]=clip16(input+((dispersion*3)>>2));
 int32_t feedback=(a*3-b*2+c)>>3;
 s->space_damp_state+=(feedback-s->space_damp_state)>>3;
 s->reverb[p]=clip16((scattered>>1)+((s->space_damp_state*(int32_t)decay)>>15));
 s->reverb_pos=(p+1u)&4095u;
 *wl=clip16((dispersion*3+a*4-b*3+c*2)/6);
 *wr=clip16((dispersion*2+b*4-a*3-c*2)/6);
}
static void ru_reverb_room(struct etch_native_fx *s,int32_t input,uint16_t decay,
                           int32_t *wl,int32_t *wr){
 uint32_t p=s->reverb_pos;
 /* Specular, sparsely spaced wall reflections rather than a diffusion tank.
  * Early reflections and a short, damped comb tail retain transient detail. */
 int32_t a=s->reverb[(p-s->tap1)&4095u];
 int32_t b=s->reverb[(p-s->tap2)&4095u];
 int32_t c=s->reverb[(p-s->tap3)&4095u];
 int32_t early1=s->reverb[(p-197u)&4095u];
 int32_t early2=s->reverb[(p-467u)&4095u];
 int32_t reflections=(a*3+b*2+c)>>3;
 s->space_damp_state+=(reflections-s->space_damp_state)>>2;
 s->reverb[p]=clip16((input*3)/4+((s->space_damp_state*(int32_t)decay)>>16));
 s->reverb_pos=(p+1u)&4095u;
 *wl=clip16((early1*4+early2*3+a*4+b*2+c)/7);
 *wr=clip16((early2*4+early1*2+b*4+c*2+a)/7);
}
static void ru_reverb_hall(struct etch_native_fx *s,int32_t input,uint16_t decay,
                           int32_t *wl,int32_t *wr){
 uint32_t p=s->reverb_pos;
 /* Schroeder allpass diffusion into a long, low-passed recursive feedback
  * network. The three staggered tank taps create the diffuse late field. */
 int32_t a=s->reverb[(p-s->tap1)&4095u];
 int32_t b=s->reverb[(p-s->tap2)&4095u];
 int32_t c=s->reverb[(p-s->tap3)&4095u];
 int32_t early1=s->reverb[(p-197u)&4095u];
 int32_t early2=s->reverb[(p-467u)&4095u];
 int32_t previous=space_diffuser[(p-43u)&63u];
 int32_t feed=(input*2)/3;
 int32_t diffused=previous-(feed>>1);
 space_diffuser[p&63u]=clip16(feed+(previous>>1));
 int32_t diffusion=(a+b+c)/3;
 s->space_damp_state+=etch_fx_bounded_lp_step(diffusion-s->space_damp_state,s->space_damp_alpha);
 s->reverb[p]=clip16(diffused+(((s->space_damp_state*(int32_t)decay)>>15)*31)/32);
 s->reverb_pos=(p+1u)&4095u;
 *wl=clip16((early1*7+early2*5+a*6+b*4+c*3)/6);
 *wr=clip16((early2*7+early1*4+b*6+c*4+a*3)/6);
}
'''
txt=txt.replace(anchor,impl+anchor)
# Control conversion: each topology independently chooses delay line geometry.
start=txt.index('  /* V1.77: wider tap span')
end=txt.index('\n }\n uint16_t feedback=',start)
replacement='''  /* Distinct delay geometries, computed only when settings change. */
  uint32_t span=(uint32_t)qclamp(cfg->reverb_size);
  switch(etch_fx_reverb_type(cfg)){
   case 0: /* SPRING: inharmonic, tightly spaced dispersion sections. */
    s->tap1=(uint16_t)(101u+span*697u/32767u);
    s->tap2=(uint16_t)(617u+span*1201u/32767u);
    s->tap3=(uint16_t)(1801u+span*1703u/32767u);
    break;
   case 2: /* ROOM: sparse discrete wall echoes, then short comb decay. */
    s->tap1=(uint16_t)(197u+span*437u/32767u);
    s->tap2=(uint16_t)(619u+span*1103u/32767u);
    s->tap3=(uint16_t)(1223u+span*1871u/32767u);
    break;
   default: /* HALL: longer staggered diffuser/tank echo delays. */
    s->tap1=(uint16_t)(521u+span*727u/32767u);
    s->tap2=(uint16_t)(1471u+span*1201u/32767u);
    s->tap3=(uint16_t)(2897u+span*1133u/32767u);
    break;
  }'''
txt=txt[:start]+replacement+txt[end:]
start=txt.index(' /* Three size-dependent taps share the original 4096-sample tank. */')
end=txt.index(' s->reverb_mix_smooth=etch_fx_slew(s->reverb_mix_smooth,reverb_target);',start)
replacement=''' int32_t input_r=etch_fx_route(cfg)?in:clip16(fxin+delay_delta);
 int32_t dry_left=etch_fx_route(cfg)?(int32_t)in+chorus_delta+delay_delta:input_r;
 int32_t dry_right=(int32_t)in+chorus_right+delay_right;
 int32_t wet_left=0,wet_right=0;
 switch(etch_fx_reverb_type(cfg)){
  case 0:ru_reverb_spring(s,input_r,decay,&wet_left,&wet_right);break;
  case 2:ru_reverb_room(s,input_r,decay,&wet_left,&wet_right);break;
  default:ru_reverb_hall(s,input_r,decay,&wet_left,&wet_right);break;
 }
 /* Shared dry/wet interpolation remains true wet-only at MIX=100. */
'''
txt=txt[:start]+replacement+txt[end:]
txt=txt.replace('cfg->routing==1','etch_fx_route(cfg)')
fx.write_text(txt)
# UI labels: no change to menu routing, but explain that DSP engines are now distinct.
edit('rupture_tutorial.h','"K1 TYPE: SPRING HALL ROOM"','"K1: 3 UNIQUE ENGINES"')
edit('rupture_help_cards.h','"REVERB: 3 TONE TYPES"','"3 UNIQUE REVERB ENGINES"')
edit('rupture_ui.c','"V1.77","FM-1_968"','"V1.78","FM-1_968"')
edit('build.py',"'version':'RUPTURE V1.77'","'version':'RUPTURE V1.78'")
print('V1.78: 3 reverb topologies; packed settings; UI persistence; helper functions done')
