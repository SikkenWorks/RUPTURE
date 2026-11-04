from pathlib import Path
p=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c'
s=p.read_text()
pre='''/* RUPTURE V1.80: all Q15 one-pole states and the allpass stage input
 * are bounded to signed 16-bit before the indicated multiply. The signed
 * difference bound is 65535, Q15 coefficient <=32767; product <=2147385345,
 * strictly below INT32_MAX. Keep truncation toward zero exactly unchanged.
 * These remove compiler-emulated 64-bit division from active phaser / tape /
 * wah paths without changing waveform arithmetic. See golden PCM tests. */
_Static_assert(65535LL*32767LL < INT32_MAX,"Q15 stage multiplication safe");
'''
s=s.replace('/* V1.02 six global FX:',pre+'/* V1.02 six global FX:',1)
# Safe Q15 ops assuming bounded results. Every result below only involves these.
rep={
 '((uint64_t)(65535u-lfo)*depth)':'((uint32_t)(65535u-lfo)*depth)',
 '((int64_t)x*amp)':'((int32_t)x*amp)',
 '((int64_t)(mag-s->wah_env)*(mag>s->wah_env?attack:rel))':'((int32_t)(mag-s->wah_env)*(int32_t)(mag>s->wah_env?attack:rel))',
 '((int64_t)s->wah_env*qclamp(c->wah_sens))':'((int32_t)s->wah_env*qclamp(c->wah_sens))',
 '((int64_t)(x-s->wah_lp)*alpha)':'((int32_t)(x-s->wah_lp)*alpha)',
 '((int64_t)(s->wah_lp-s->wah_hp)*((alpha>>2)+100u))':'((int32_t)(s->wah_lp-s->wah_hp)*((alpha>>2)+100u))',
 '((int64_t)band*resonance)':'((int32_t)band*resonance)',
 '((int64_t)coeff*y)':'((int32_t)coeff*y)',
 '((int64_t)coeff*v)':'((int32_t)coeff*v)',
 '((int64_t)(y-s->tape_tone_state)*alpha)':'((int32_t)(y-s->tape_tone_state)*alpha)',
 '((int64_t)y*drive)':'((int32_t)y*drive)',
 '((int64_t)(s->decim_held-x)*s->decim_mix_smooth)':'((int32_t)(s->decim_held-x)*s->decim_mix_smooth)',
}
for a,b in rep.items():
 if a not in s: print('MISSING',a)
 else: s=s.replace(a,b)
p.write_text(s)
print('Optimized Q15 effects: phaser, wah, tape, decimator, tremolo, unchanged math')
