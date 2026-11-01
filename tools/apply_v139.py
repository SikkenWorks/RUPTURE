from pathlib import Path
root=Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
# EQ unity identification is safe only after any coefficient and headroom ramp finished.
p=native/'rupture_eq2.h';s=p.read_text();anchor='int32_t rupture_eq2_process(struct rupture_eq2_state *s,int32_t x);'
assert s.count(anchor)==1
s=s.replace(anchor,anchor+'\n/* A four-band 0 dB EQ must be an exact zero-CPU DSP bypass once its\n * existing coefficient/headroom ramps have settled. */\nuint8_t rupture_eq2_is_unity(const struct rupture_eq2_state *s);')
p.write_text(s)
p=native/'rupture_eq2.c';s=p.read_text();anchor='int32_t rupture_eq2_process(struct rupture_eq2_state *s,int32_t x){';assert s.count(anchor)==1
s=s.replace(anchor,'''uint8_t rupture_eq2_is_unity(const struct rupture_eq2_state *s){
 if(s->current_headroom!=32767u || s->headroom_q15[s->index]!=32767u)return 0u;
 for(uint8_t b=0u;b<4u;b++){
  const struct rupture_eq2_bq *v=&s->current[b],*t=&s->target[s->index][b];
  if(v->b0!=RU_EQ2_ONE||v->b1||v->b2||v->a1||v->a2||
     t->b0!=RU_EQ2_ONE||t->b1||t->b2||t->a1||t->a2)return 0u;
 }
 return 1u;
}
'''+anchor)
# Even active FX routing may call EQ processing; avoiding 4x biquads at unity is safe.
s=s.replace('''int32_t rupture_eq2_process(struct rupture_eq2_state *s,int32_t x){
 /* No division''','''int32_t rupture_eq2_process(struct rupture_eq2_state *s,int32_t x){
 if(rupture_eq2_is_unity(s))return x;
 /* No division''')
p.write_text(s)
p=native/'native_fx.c';s=p.read_text()
old='''    !s->eq2_enabled && /* EQ2 parameters live outside legacy FX settings */'''
new='''    (!s->eq2_enabled || (bypass_mask&ETCH_FXB_BIT(ETCH_FXB_EQ)) || rupture_eq2_is_unity(&s->eq2)) &&
    /* RUPTURE 1.39: when the parametrical EQ is at unity, its 4 biquads
     * must not keep every supposedly DRY sample in the expensive wet chain.
     * Active/tailing EQ still processes normally until all ramps settle. */'''
assert s.count(old)==1;s=s.replace(old,new)
p.write_text(s)
# In default neutral bend path there is no mathematical purpose to do a software
# emulated uint64 multiply and variable shift on every output sample.
p=native/'audio_core.c';s=p.read_text()
old='''    uint64_t adjusted=((uint64_t)s->mono_current_inc*mfactor)>>16;
    pitch_inc=(uint32_t)(adjusted>0xffffffffu?0xffffffffu:adjusted);'''
new='''    if(mfactor==65536u) pitch_inc=s->mono_current_inc;
    else {
     uint64_t adjusted=((uint64_t)s->mono_current_inc*mfactor)>>16;
     pitch_inc=(uint32_t)(adjusted>0xffffffffu?0xffffffffu:adjusted);
    }'''
assert s.count(old)==1;s=s.replace(old,new)
# Eliminate per-sample RUPTURE filter transition work entirely for stable bypass.
old=''' struct rupture_filter_transition *t=&filter_transition[voice_index];
 uint8_t mode=c->filter_type<5u?c->filter_type:0u;
 if(mode!=t->mode){'''
new=''' struct rupture_filter_transition *t=&filter_transition[voice_index];
 uint8_t mode=c->filter_type<5u?c->filter_type:0u;
 if(!mode && !t->mode && !t->remaining){
  v->low=v->band=0;
  t->last=(int16_t)x;
  return x; /* exact filter-OFF PCM parity, no coeff or xfade computation */
 }
 if(mode!=t->mode){'''
assert s.count(old)==1;s=s.replace(old,new)
p.write_text(s)
# Restore monophonic scheduling in the hardware safety path: reject ownership
# violations without lying about a physical error. Leave the 8-miss shutdown.
# No changes to core boot sectors, OTA, or guard thresholds.
print('Applied v1.39 dry-bypass & neutral-pitch fast paths')
