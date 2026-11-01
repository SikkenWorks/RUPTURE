from pathlib import Path
p=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c'
h=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h'
s=p.read_text(); hs=h.read_text()
old='static volatile uint8_t rt_fuzz_drive=55u,rt_fuzz_octave=75u,rt_fuzz_tone=65u,rt_fuzz_mix=0u,rt_fuzz_on=0u;'
new='''/* V1.42: Convert knob values to audio-domain Q15 targets ONCE in the UI.
 * The audio IRQ only slews and multiplies. These are not stored project data. */
static volatile uint8_t rt_fuzz_drive=55u,rt_fuzz_octave=75u,rt_fuzz_tone=65u,rt_fuzz_mix=0u,rt_fuzz_on=0u;
static volatile int32_t rt_fuzz_gain_q15=32767+55000;
static volatile int32_t rt_fuzz_anti_alpha=32767-6600;
static volatile int32_t rt_fuzz_octave_q15=24575;
static volatile int32_t rt_fuzz_tone_alpha=600+20800;'''
assert old in s;s=s.replace(old,new,1)
old=''' rt_fuzz_tone=tone>100u?100u:tone;rt_fuzz_mix=mix>100u?100u:mix;rt_fuzz_on=enabled?1u:0u;
}'''
new=''' rt_fuzz_tone=tone>100u?100u:tone;rt_fuzz_mix=mix>100u?100u:mix;rt_fuzz_on=enabled?1u:0u;
 rt_fuzz_gain_q15=32767+(int32_t)rt_fuzz_drive*1000;
 rt_fuzz_anti_alpha=32767-(int32_t)rt_fuzz_drive*120;
 rt_fuzz_octave_q15=((int32_t)rt_fuzz_octave*32767+50)/100;
 rt_fuzz_tone_alpha=600+(int32_t)rt_fuzz_tone*320;
}'''
assert old in s;s=s.replace(old,new,1)
a=s.index('static int16_t rupture_fuzz_process(')
b=s.index('/* SANDPAPER:',a)
orig=s[a:b];
new='''/* Band-limit the octave rectifier at its zero-crossing without costly
 * oversampling. The polynomial is C1-continuous at +/-4096 PCM, exact
 * abs(x) outside that narrow region, and exactly zero for silence.
 * All products are <= 4096^2 and use shifts instead of division. */
static inline int32_t rupture_fuzz_soft_rect(int32_t x){
 int32_t mag=x<0?-x:x;
 if(mag>=4096)return mag;
 int32_t q=(mag*mag)>>12;
 return (3*q-((q*q)>>12))>>1;
}
static int16_t rupture_fuzz_process(struct etch_native_fx *s,int16_t sample){
 int32_t x=sample;
 int32_t target=rt_fuzz_on?(uint32_t)rt_fuzz_mix*32767u/100u:0u;
 s->rupture_fuzz_mix=etch_fx_slew(s->rupture_fuzz_mix,target);
 if(!s->rupture_fuzz_mix){
  s->rupture_fuzz_dc=s->rupture_fuzz_tone=s->rupture_fuzz_input_lp=0;
  s->rupture_fuzz_in_env=s->rupture_fuzz_wet_env=0;
  s->rupture_fuzz_makeup=32767;s->rupture_fuzz_makeup_target=32767;
  s->rupture_fuzz_drive_slew=rt_fuzz_gain_q15;
  s->rupture_fuzz_anti_slew=rt_fuzz_anti_alpha;
  s->rupture_fuzz_oct_slew=rt_fuzz_octave_q15;
  s->rupture_fuzz_tone_slew=rt_fuzz_tone_alpha;
  s->rupture_fuzz_tick=0;return sample;
 }
 /* Knobs are now audio-rate smoothed; no instantaneous gain or tone jumps. */
 s->rupture_fuzz_drive_slew=etch_fx_slew(s->rupture_fuzz_drive_slew,rt_fuzz_gain_q15);
 s->rupture_fuzz_anti_slew=etch_fx_slew(s->rupture_fuzz_anti_slew,rt_fuzz_anti_alpha);
 s->rupture_fuzz_oct_slew=etch_fx_slew(s->rupture_fuzz_oct_slew,rt_fuzz_octave_q15);
 s->rupture_fuzz_tone_slew=etch_fx_slew(s->rupture_fuzz_tone_slew,rt_fuzz_tone_alpha);
 int32_t delta=x-s->rupture_fuzz_input_lp;
 s->rupture_fuzz_input_lp+=(delta*s->rupture_fuzz_anti_slew)/32768;
 /* The original clipping is retained to preserve aggressive fuzz identity.
  * 32-bit product bound: abs(input_lp) <= 32768; gain>>2 <= 33192. */
 int32_t driven=clip16((s->rupture_fuzz_input_lp*(s->rupture_fuzz_drive_slew>>2))/8192);
 int32_t rect=rupture_fuzz_soft_rect(driven);
 /* Full-wave octave DC tracker; no surprise nonzero output at silence. */
 s->rupture_fuzz_dc+=(rect-s->rupture_fuzz_dc)/128;
 int32_t octave=clip16((rect-s->rupture_fuzz_dc)*2);
 int32_t oct=s->rupture_fuzz_oct_slew;
 int32_t mixoct=driven+((octave-driven)*oct)/32767;
 s->rupture_fuzz_tone+=((mixoct-s->rupture_fuzz_tone)*s->rupture_fuzz_tone_slew)/32768;
 int32_t wet=clip16(s->rupture_fuzz_tone);
 /* Gain matching detects the envelope every sample. A new gain target is
  * computed at control rate, but APPLIED EVERY SAMPLE rather than jumping
  * once per 32 samples (which could produce an audible buzz). */
 int32_t in_mag=x<0?-x:x,wet_mag=wet<0?-wet:wet;
 s->rupture_fuzz_in_env+=(in_mag-s->rupture_fuzz_in_env)/256;
 s->rupture_fuzz_wet_env+=(wet_mag-s->rupture_fuzz_wet_env)/256;
 if(++s->rupture_fuzz_tick>=32u){
  s->rupture_fuzz_tick=0u;
  int32_t dry_ref=s->rupture_fuzz_in_env+256;
  int32_t wet_ref=s->rupture_fuzz_wet_env+256;
  int32_t ratio=(dry_ref*32767)/wet_ref;
  if(ratio>32767)ratio=32767;
  if(ratio<4096)ratio=4096;
  s->rupture_fuzz_makeup_target=ratio;
 }
 s->rupture_fuzz_makeup+=
  (s->rupture_fuzz_makeup_target-s->rupture_fuzz_makeup)/256;
 /* Minimum 1-count progress prevents a stalled ramp on tiny differences. */
 if(s->rupture_fuzz_makeup!=s->rupture_fuzz_makeup_target){
  int32_t diff=s->rupture_fuzz_makeup_target-s->rupture_fuzz_makeup;
  if(diff>0)s->rupture_fuzz_makeup++;
  else if(diff<0)s->rupture_fuzz_makeup--;
 }
 wet=(wet*s->rupture_fuzz_makeup)/32767;
 return clip16(rupture_q15_mix(x,wet,(uint16_t)s->rupture_fuzz_mix));
}
'''
s=s[:a]+new+s[b:]
old='int32_t rupture_fuzz_in_env,rupture_fuzz_wet_env,rupture_fuzz_makeup;uint8_t rupture_fuzz_tick;'
new='''int32_t rupture_fuzz_in_env,rupture_fuzz_wet_env,rupture_fuzz_makeup;
 int32_t rupture_fuzz_makeup_target;
 int32_t rupture_fuzz_drive_slew,rupture_fuzz_anti_slew,rupture_fuzz_oct_slew,rupture_fuzz_tone_slew;
 uint8_t rupture_fuzz_tick;'''
assert old in hs;hs=hs.replace(old,new,1)
s=s.replace('s->comp_gain=32767;s->comp_target=32767;s->rupture_fuzz_makeup=32767;','s->comp_gain=32767;s->comp_target=32767;s->rupture_fuzz_makeup=32767;s->rupture_fuzz_makeup_target=32767;')
p.write_text(s);h.write_text(hs)
print('PATCHED v142, original FX bytes',len(orig),'new',len(new))
