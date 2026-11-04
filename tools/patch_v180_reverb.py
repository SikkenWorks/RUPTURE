from pathlib import Path
root=Path(__file__).resolve().parents[1]
d=root/'vendor/etch-v1.88-source/firmware/fm1-native'
p=d/'native_fx.h';t=p.read_text();a=' int32_t reverb_mix_smooth;';assert a in t;t=t.replace(a,a+'''
 /* V1.80: fixed-cost 4-line room FDN / dual-waveguide spring.
  * 4096 existing reverb samples are reused: NO new PCM buffers.
  * Counters and damp states (36 bytes) are per-global-FX, not per voice. */
 uint16_t reverb_line_cursor[4];
 int32_t reverb_line_damp[4];
 int32_t spring_input_lowpass;
 uint8_t reverb_algorithm_seen;
''',1);p.write_text(t)
p=d/'native_fx.c';t=p.read_text();a=t.index('/* RUPTURE V1.78: Three genuinely distinct global reverb algorithms.');b=t.index('static inline void ru_reverb_hall(',a)
new='''/* RUPTURE V1.80: bounded true delay-network reverbs.
 * The 4096 existing reverb words are partitioned into independent
 * fixed-size delay lines. Every line is read and written exactly once.
 * No per-sample division, modulo, malloc, FIR convolution or 64-bit maths.
 * ROOM: four-line Householder FDN. The 4x4 feedback matrix is orthogonal
 * before decay/damping. Four unequal delays scatter early reflections.
 * SPRING: two coupled waveguides, two independent short dispersive allpass
 * sections and a gently high-passed exciter. The coupling and dispersers
 * produce the repeated, progressively smeared spring-like flutter without
 * the prior V1.78 alternating-sample harshness.
 * Both run only when their global wet send is active. Hall is unchanged.
 */
static inline void ru_reverb_spring(struct etch_native_fx *s,int32_t input,uint16_t decay,
                             int32_t *wl,int32_t *wr){
 /* Two delay lines fill the existing 4096 sample tank, independent of hall.
  * Asymmetric 2039/2057 samples => ~46.2/46.6 ms at 44.1 kHz. */
 const uint32_t p=s->reverb_pos++;
 const uint32_t k0=s->reverb_line_cursor[0];
 const uint32_t k1=s->reverb_line_cursor[1];
 const int32_t d0=s->reverb[k0];
 const int32_t d1=s->reverb[2039u+k1];
 /* Remove only the deepest subsonic energy from the wet excitation. */
 s->spring_input_lowpass+=(input-s->spring_input_lowpass)>>6;
 int32_t exc=input-(s->spring_input_lowpass>>1);
 /* Cascaded 32-tap lattice allpasses, sharing the pre-existing 64-sample
  * diffuser. This varies group delay across frequency (dispersion). */
 uint32_t ap=p&31u;
 int32_t hist0=space_diffuser[ap];
 int32_t dispersed=hist0-((exc*19456)>>15);
 space_diffuser[ap]=clip16(exc+((dispersed*19456)>>15));
 uint32_t ap2=32u+((p-11u)&31u);
 int32_t hist1=space_diffuser[ap2];
 int32_t dispersed2=hist1-((dispersed*17000)>>15);
 space_diffuser[ap2]=clip16(dispersed+((dispersed2*17000)>>15));
 /* Real spring coils absorb highs per round-trip. 0.5 pole at 44.1 kHz
  * prevents a synthetic, brittle metallic ring. */
 s->reverb_line_damp[0]+=(d0-s->reverb_line_damp[0])>>1;
 s->reverb_line_damp[1]+=(d1-s->reverb_line_damp[1])>>1;
 int32_t fb=19000+((int32_t)decay*11300>>15); /* 0.58 .. 0.925 */
 int32_t w0=(dispersed2>>2)+((s->reverb_line_damp[0]*fb)>>15)
            +((s->reverb_line_damp[1]*1900)>>15);
 int32_t w1=(dispersed>>2)+((s->reverb_line_damp[1]*fb)>>15)
            +((s->reverb_line_damp[0]*1900)>>15);
 s->reverb[k0]=clip16(w0);
 s->reverb[2039u+k1]=clip16(w1);
 s->reverb_line_cursor[0]=(uint16_t)(k0+1u==2039u?0u:k0+1u);
 s->reverb_line_cursor[1]=(uint16_t)(k1+1u==2057u?0u:k1+1u);
 /* Different dispersive line lengths provide the stereo decorrelation. */
 *wl=clip16((d0*5+d1*3)>>3);
 *wr=clip16((d0*3+d1*5)>>3);
}
static inline void ru_reverb_room(struct etch_native_fx *s,int32_t input,uint16_t decay,
                           int32_t *wl,int32_t *wr){
 /* Four real feedback lines. Unlike V1.78 ROOM (one circular delay with
  * 5 taps), each line has its own memory/cursor and damping pole.
  * Unequal line lengths: 16.5, 20.0, 23.0 and 33.4 milliseconds. */
 static const uint16_t starts[4]={0,727,1608,2621};
 static const uint16_t sizes[4]={727,881,1013,1475};
 int32_t v[4];
 for(uint32_t i=0;i<4u;i++)v[i]=s->reverb[starts[i]+s->reverb_line_cursor[i]];
 /* Householder reflector for N=4: m_i = (sum(v)/2)-v_i.
  * Energy-preserving coupling prior to damping and decay. */
 const int32_t half=(v[0]+v[1]+v[2]+v[3])>>1;
 const int32_t gain=20000+((int32_t)decay*10000>>15); /* 0.61..0.916 */
 int32_t newv[4];
 for(uint32_t i=0;i<4u;i++){
  int32_t diff=v[i]-s->reverb_line_damp[i];
  s->reverb_line_damp[i]+=diff>>1; /* cheap absorptive wall material */
 }
 /* Damp the feedback state itself, rather than the dry excitation. */
 const int32_t filtered=(s->reverb_line_damp[0]+s->reverb_line_damp[1]+
                         s->reverb_line_damp[2]+s->reverb_line_damp[3])>>1;
 for(uint32_t i=0;i<4u;i++){
  int32_t reflected=filtered-s->reverb_line_damp[i];
  int32_t injection=(i&1u)?-(input>>2):(input>>2);
  newv[i]=clip16(injection+((reflected*gain)>>15));
  s->reverb[starts[i]+s->reverb_line_cursor[i]]=(int16_t)newv[i];
  uint32_t next=s->reverb_line_cursor[i]+1u;
  s->reverb_line_cursor[i]=(uint16_t)(next==sizes[i]?0u:next);
 }
 s->reverb_pos=(s->reverb_pos+1u)&4095u;
 /* Unequal early/late reflections; gently decorrelated stereo returns.
  * The early FDN arrivals are naturally at 16..33 ms, not dry bleed. */
 *wl=clip16((v[0]*4+v[1]*3+v[2]*2-v[3])>>3);
 *wr=clip16((v[0]*2-v[1]+v[2]*4+v[3]*3)>>3);
}
'''
t=t[:a]+new+t[b:];p.write_text(t)
print('Patched native reverb DSP and FX struct')
