from pathlib import Path
p=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c'
s=p.read_text()
old='''static uint8_t mono_perf_tick(struct etch_native_audio *s){
 uint8_t out=0,changed=0;
 for(uint8_t i=0;i<s->mono_count;i++){
  struct etch_mono_held m=s->mono_stack[i];
  if(m.id>=4096u && m.gate_left && !--m.gate_left){changed=1;continue;}
  s->mono_stack[out++]=m;
 }
 if(out!=s->mono_count){s->mono_count=out;mono_resume(s);}
 return changed;
}'''
new='''static uint8_t mono_perf_tick(struct etch_native_audio *s){
#ifdef RUPTURE_BUILD
 /* V1.81: Most PCM samples contain no performance gate expiry. Countdown
  * in place, and compact the stack only on an actual expiry. Identical
  * ordered mono-note state, but no six-note read/write copy every sample.
  * Timer-driven sequence notes still expire at the EXACT same sample. */
 uint8_t out=0u,changed=0u;
 for(uint8_t i=0u;i<s->mono_count;i++){
  struct etch_mono_held *m=&s->mono_stack[i];
  if(m->id>=4096u && m->gate_left && !--m->gate_left){changed=1u;continue;}
  if(changed && out!=i)s->mono_stack[out]=*m;
  ++out;
 }
 if(changed){s->mono_count=out;mono_resume(s);}
 return changed;
#else
 uint8_t out=0,changed=0;
 for(uint8_t i=0;i<s->mono_count;i++){
  struct etch_mono_held m=s->mono_stack[i];
  if(m.id>=4096u && m.gate_left && !--m.gate_left){changed=1;continue;}
  s->mono_stack[out++]=m;
 }
 if(out!=s->mono_count){s->mono_count=out;mono_resume(s);}
 return changed;
#endif
}'''
assert old in s;s=s.replace(old,new,1)
old='''static void native_rates(const struct etch_native_audio *s,const struct etch_native_controls *c,
                         uint32_t increments[8],uint32_t modincrements[8]){
 static const uint8_t ratios[12]={8,1,2,4,12,16,24,32,48,64,96,128};
 for(uint32_t i=0;i<8;i++){'''
new='''static void native_rates(const struct etch_native_audio *s,const struct etch_native_controls *c,
                         uint32_t increments[8],uint32_t modincrements[8]){
 static const uint8_t ratios[12]={8,1,2,4,12,16,24,32,48,64,96,128};
#ifdef RUPTURE_BUILD
 /* V1.81: RUPTURE renders one monophonic slot; even rapid note retriggers
  * must never recalculate seven silent ETCH polyphonic slot rates. */
 for(uint32_t i=1u;i<8u;i++){increments[i]=0u;modincrements[i]=0u;}
 for(uint32_t i=0;i<1u;i++){
#else
 for(uint32_t i=0;i<8;i++){
#endif'''
assert old in s;s=s.replace(old,new,1)
# Note pitch updating on every note - restrict per-voice control update to existing mono active slot.
old='''   for(uint32_t oi=0;oi<ETCH_NATIVE_VOICES;oi++){
    struct etch_native_voice *ov=&s->voice[oi];'''
new='''#ifdef RUPTURE_BUILD
   for(uint32_t oi=0;oi<1u;oi++){
#else
   for(uint32_t oi=0;oi<ETCH_NATIVE_VOICES;oi++){
#endif
    struct etch_native_voice *ov=&s->voice[oi];'''
assert old in s;s=s.replace(old,new,1)
old='''   for(uint32_t li=0;li<ETCH_NATIVE_VOICES;li++){
    struct etch_native_voice *lv=&s->voice[li];'''
new='''#ifdef RUPTURE_BUILD
   for(uint32_t li=0;li<1u;li++){
#else
   for(uint32_t li=0;li<ETCH_NATIVE_VOICES;li++){
#endif
    struct etch_native_voice *lv=&s->voice[li];'''
assert old in s;s=s.replace(old,new,1)
# perf_tick gate countdown for offboard 8 voices irrelevant under RUPTURE mono.
old=''' for(uint32_t i=0;i<ETCH_NATIVE_VOICES;i++){
  struct etch_native_voice *v=&s->voice[i];
  if(v->active&&v->note>=4096&&v->gate_left){if(!--v->gate_left)v->held=v->down=0;}
 }'''
new='''#ifdef RUPTURE_BUILD
 for(uint32_t i=0;i<1u;i++){
#else
 for(uint32_t i=0;i<ETCH_NATIVE_VOICES;i++){
#endif
  struct etch_native_voice *v=&s->voice[i];
  if(v->active&&v->note>=4096&&v->gate_left){if(!--v->gate_left)v->held=v->down=0;}
 }'''
assert old in s;s=s.replace(old,new,1)
p.write_text(s)
print('patched note-on control path',p)
