from pathlib import Path
root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c'
old=src.read_text()
ref=root/'reference/audio_core-V1.82.c'
ref.write_text(old)
needle=' uint32_t evo_step=evolving?etch_evolution_step(e,(c->clock_follow&&s->clock_bpm)?s->clock_bpm:c->tempo):0u;\n'
replace=needle+''' /* V1.83: control snapshots remain immutable for the entire DMA render.
  * Compute the Q16 master gain target once per block; the sample-wise
  * slewing itself is unchanged and stays click-free. */
 const int32_t gain_target=(int32_t)(c->gain>32767?32767:c->gain)*65536;
'''
assert old.count(needle)==1
new=old.replace(needle,replace,1)
needle='  const int32_t gain_target=(int32_t)(c->gain>32767?32767:c->gain)*65536;\n'
assert new.count(needle)==1
new=new.replace(needle,'',1)
needle='''  uint32_t admitted=voices_sounding;
  for(uint32_t i=0;i<limit;i++)if(s->voice[i].active &&
      s->voice[i].held && s->voice[i].attack_fade &&
      s->voice[i].env<=256)++admitted;
  s->poly_div_q10=etch_poly_headroom_step(s->poly_div_q10,admitted);
'''
replace='''#ifdef RUPTURE_BUILD
  /* V1.83: mono always has <= 1 sounding voice. The upstream adaptive
   * headroom target is therefore invariant at 4096 (Q10 -> /4). Keep
   * the normal 8-voice algorithm untouched for the non-RUPTURE target.
   * No effect, attack, MIDI, glide or reverb sample is approximated. */
  s->poly_div_q10=4096;
#else
  uint32_t admitted=voices_sounding;
  for(uint32_t i=0;i<limit;i++)if(s->voice[i].active &&
      s->voice[i].held && s->voice[i].attack_fade &&
      s->voice[i].env<=256)++admitted;
  s->poly_div_q10=etch_poly_headroom_step(s->poly_div_q10,admitted);
#endif
'''
assert new.count(needle)==1
new=new.replace(needle,replace,1)
needle='''  int32_t mix=(bounded_sum*1024)/s->poly_div_q10;
'''
replace='''#ifdef RUPTURE_BUILD
  int32_t mix=bounded_sum/4;
#else
  int32_t mix=(bounded_sum*1024)/s->poly_div_q10;
#endif
'''
assert new.count(needle)==1
new=new.replace(needle,replace,1)
needle='''   preview_dry=etch_poly_safe_sample((bounded_preview*1024)/s->poly_div_q10);
'''
replace='''#ifdef RUPTURE_BUILD
   preview_dry=etch_poly_safe_sample(bounded_preview/4);
#else
   preview_dry=etch_poly_safe_sample((bounded_preview*1024)/s->poly_div_q10);
#endif
'''
assert new.count(needle)==1
new=new.replace(needle,replace,1)
src.write_text(new)
print('updated',src,len(old),len(new))
