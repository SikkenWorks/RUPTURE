/* SPDX-License-Identifier: GPL-3.0-only
 * Bounded, allocation-free global FX. Settable LFO speed/depth, variable echo
 * time/feedback, reverb size/decay and serial or parallel signal routing.
 * The hard limiter and signed intermediate arithmetic keep feedback stable.
 */
#include "native_fx.h"
#include "native_fx_bypass_v181.h"
#ifdef RUPTURE_BUILD
#include "rupture_mosfet_lut.h"
#endif
/* The FM1 application has a single global FX instance. These buffers occupy
 * verified spare .noinit SRAM rather than the saturated DMA/voice POOL.
 * They are explicitly cleared by etch_fx_reset before sound starts. */
static int16_t reverse_line[3584] __attribute__((section(".noinit")));
static int16_t vibrato_line[512] __attribute__((section(".noinit")));
static int16_t tape_line[512] __attribute__((section(".noinit")));
/* V1.22 SPACE 2.0: one 64-sample diffusion line shared by both channels.
 * Global rather than per-voice: fixed 128 B SRAM, zero heap allocation. */
static int16_t space_diffuser[64] __attribute__((section(".noinit")));
static int16_t clip16(int32_t v){return (int16_t)(v<-32768?-32768:v>32767?32767:v);}
static uint16_t qclamp(uint16_t x){return x>32767?32767:x;}
/* V1.74: signed 32-bit equivalent of the previous int64/32768 LP steps.
 * BOUND PROOF: EQ and SPACE inputs are clipped int16 samples; one-pole
 * states start at zero and have 0 <= alpha < 32768, so they remain within
 * [-32768,32767]. |delta| <= 65535. Largest coefficient is 15000:
 * 65535*15000 = 983025000 < INT32_MAX. No rounding/phase change.
 * The independent V1.73 golden PCM tests cover dry/wet, EQ/HPF/SPACE.
 * Do NOT reuse for a feedback integrator without the same proof. */
_Static_assert(65535LL * 15000LL <= INT32_MAX, "LP step must not overflow 32-bit");
static inline int32_t etch_fx_bounded_lp_step(int32_t delta, int32_t alpha){
 return (delta*alpha)/32768;
}
/* RC candidate F03: wet-output soft knee before the hardware DAC/USB stage.
 * No effect sends means exact bit-for-bit dry bypass. The knee starts at
 * 22,000 and asymptotically approaches 30,192 in Q15, avoiding full-scale
 * feedback spikes without resetting time-based FX state. This is deliberately
 * a cheap bounded integer operation, not loudness matching or DAC calibration.
 */
/* RUPTURE V1.18: division-free output soft knee with a guaranteed 28,000
 * Q15 output ceiling (about 1.4 dB below full scale). The previous wet
 * limiter performed a variable integer division for EVERY loud sample in
 * the realtime ISR. Both branches use only additions/shifts and remain
 * stable for summed stereo reverb peaks. The audio path does not allocate.
 * This is an audio peak guard, NOT a substitute for timing/THD profiling. */
static int16_t etch_fx_wet_safety(int32_t x,uint8_t wet_active){
 if(!wet_active)return clip16(x);
 int32_t mag=x<0?(x==INT32_MIN?INT32_MAX:-x):x;
 if(mag<=22000)return (int16_t)x;
 int32_t excess=mag-22000;
 int32_t limited;
 if(excess<=8000)limited=22000+(excess>>1);
 else limited=26000+((excess-8000)>>3);
 if(limited>28000)limited=28000;
 return (int16_t)(x<0?-limited:limited);
}
/* Both operands are signed Q15 audio differences/coefficients. The exact
 * product bound is 65,535 * 32,767 = 2,147,385,345 < INT32_MAX.
 * Use a 32-bit multiply rather than compiler-emulated 64-bit divide on the
 * pi32v2, executed hundreds of times per 128-frame DMA half. */
_Static_assert(65535LL*32767LL<=INT32_MAX,"Q15 audio mix requires 32bit bound");
static inline int32_t rupture_q15_mix(int32_t dry,int32_t wet,uint16_t mix){
 /* The older ETCH serial FX may temporarily exceed int16 range before
  * its saturation step. Never multiply an unbounded intermediate. */
 dry=clip16(dry);wet=clip16(wet);
 int32_t d=wet-dry;
 return dry+(d*(int32_t)mix)/32767;
}
/* V1.79: the input/output of the normal FX signal path are bounded
 * Q15, so (wet-dry)*mix is ALWAYS in signed int32:
 * 65,535 * 32,767 = 2,147,385,345 < 2^31. For the uncommon unclipped
 * intermediate use the ORIGINAL 64-bit expression, preserving PCM exactly.
 * This is a CPU fastpath, not a reduced-precision wet mix. */
static inline int32_t ru_fx_delta_mix(int32_t dry,int32_t wet,int32_t mix){
 if(dry>=-32768 && dry<=32767 && wet>=-32768 && wet<=32767 &&
    mix>=0 && mix<=32767){
  return dry+((wet-dry)*mix)/32767;
 }
 return dry+(int32_t)((((int64_t)wet-(int64_t)dry)*mix)/32767);
}
static inline int32_t ru_fx_reverb_lerp(int32_t dry,int32_t wet,uint16_t space){
 /* Convex weighted sum of two Q15 samples cannot overflow int32. The
  * exact old numerator and truncation-to-zero division are preserved. */
 if(dry>=-32768 && dry<=32767 && wet>=-32768 && wet<=32767){
  return (dry*(int32_t)(32767u-space)+wet*(int32_t)space)/32767;
 }
 return (int32_t)(((int64_t)dry*(32767u-space)+
                    (int64_t)wet*space)/32767);
}
void etch_fx_reset(struct etch_native_fx *s){
 uint8_t *p=(uint8_t*)s;for(uint32_t i=0;i<sizeof(*s);i++)p[i]=0;
 for(uint32_t i=0;i<3584;i++)reverse_line[i]=0;
 for(uint32_t i=0;i<512;i++)vibrato_line[i]=0;
 for(uint32_t i=0;i<512;i++)tape_line[i]=0;
 for(uint32_t i=0;i<64;i++)space_diffuser[i]=0;
 s->eq_gain_smooth[0]=s->eq_gain_smooth[1]=s->eq_gain_smooth[2]=16384;
#ifdef RUPTURE_BUILD
 s->eq2_enabled=0u;
 s->ru_reverb_last_type=0xffu;
 for(uint8_t slot=0u;slot<RU_FX_SLOTS;slot++){
  s->ru_slot_fuzz[slot].makeup=32767;
  s->ru_slot_fuzz[slot].makeup_target=32767;
  s->ru_slot_small[slot].comp_gain=32767;
  s->ru_slot_small[slot].comp_target=32767;
 }
 s->eq2.current_headroom=32767u;
 s->eq2.headroom_q15[0]=s->eq2.headroom_q15[1]=32767u;
 s->eq2.initialized=1u;
 s->eq2.unity_hint=1u;
 s->eq2.active_mask=0u;
 s->eq2.slew_active=0u;
 for(int b=0;b<4;b++){
  s->eq2.current[b].b0=RU_EQ2_ONE;
  for(int k=0;k<2;k++)s->eq2.target[k][b].b0=RU_EQ2_ONE;
 }
#endif
 s->comp_gain=32767;s->comp_target=32767;s->rupture_fuzz_makeup=32767;s->rupture_fuzz_makeup_target=32767;
}
/* Ring Mod and channel EQ run once after the voice sum, never per voice.
 * No heap, no trigonometry, no flash access inside the audio ISR. */
/* Fractional mono tap shared by the two chorus output phases. This avoids
 * 1-sample read-address jumps on slow chorus modulation (Q8 position). */
static int32_t etch_fx_chorus_read_at(const struct etch_native_fx *s,uint32_t pos,uint32_t offset_q8){
 uint32_t idx=(pos-(offset_q8>>8))&(ETCH_CHORUS_SIZE-1u);
 int32_t a=s->chorus[idx],b=s->chorus[(idx-1u)&(ETCH_CHORUS_SIZE-1u)];
 return a+(((b-a)*(int32_t)(offset_q8&255u))/256);
}
static int32_t etch_fx_chorus_read(const struct etch_native_fx *s,uint32_t offset_q8){
 return etch_fx_chorus_read_at(s,s->chorus_pos,offset_q8);
}
static int32_t etch_fx_slew(int32_t current,int32_t target){
 int32_t delta=target-current;
 if(!delta)return current;
 return current+(delta>0?(delta+63)/64:-((-delta+63)/64));
}
static int16_t etch_fx_ring_eq(struct etch_native_fx *s,int16_t sample,const struct etch_fx_settings *c,uint32_t bypass_mask){
 int32_t x=sample;
 /* Slew gain and wet controls over approximately 1.5ms @44.1kHz. */
 s->ring_mix_smooth=etch_fx_slew(s->ring_mix_smooth,qclamp(c->ring_mix));
 /* Many pre-V0.96 callers supply zeroed optional FX settings. Treat this
  * *legacy uninitialised* signature as neutral EQ, not as mute-all. */
 uint8_t legacy_zero=(uint8_t)(!c->ring_rate&&!c->ring_mix&&!c->ring_shape&&!c->ring_bias&&
     !c->eq_low&&!c->eq_mid&&!c->eq_high&&!c->eq_hpf);
 for(uint32_t i=0;i<3u;i++){
  uint16_t goal=legacy_zero?16384:(i==0?c->eq_low:i==1?c->eq_mid:c->eq_high);
  s->eq_gain_smooth[i]=etch_fx_slew(s->eq_gain_smooth[i],qclamp(goal));
 }
 if(s->ring_mix_smooth){
  uint32_t p=s->ring_phase>>16;
  int32_t triangle=p<32768u?(int32_t)(p*2u)-32767:98303-(int32_t)(p*2u);
  int32_t square=p<32768u?32767:-32767;
  int32_t shape=qclamp(c->ring_shape);
  int32_t carrier=(triangle*(32767-shape)+square*shape)/32767;
  int32_t bias=qclamp(c->ring_bias);
  carrier=(carrier*(32767-bias)+32767*bias)/32767;
  int32_t wet=(x*carrier)/32767;
  x=ru_fx_delta_mix(x,wet,s->ring_mix_smooth);
 }
 s->ring_phase+=s->ring_increment;
 if(!(bypass_mask&ETCH_FXB_BIT(ETCH_FXB_EQ))){
#ifdef RUPTURE_BUILD
 if(s->eq2_enabled){
  x=rupture_eq2_process(&s->eq2,x);
 }else
#endif
 {
 /* One-pole crossovers: low <~200Hz, mid ~200Hz..~6kHz, high >~6kHz.
  * The three bands recombine exactly to dry when each gain is at unity. */
 s->eq_lp_low+=(int32_t)(etch_fx_bounded_lp_step(x-s->eq_lp_low,900));
 s->eq_lp_high+=(int32_t)(etch_fx_bounded_lp_step(x-s->eq_lp_high,9000));
#ifndef RUPTURE_BUILD
 if(c->eq_hpf){
  s->eq_lp_hpf+=(int32_t)(etch_fx_bounded_lp_step(x-s->eq_lp_hpf,s->eq_hpf_alpha));
  x-=s->eq_lp_hpf;
 }else s->eq_lp_hpf=x;
#endif
 if(s->eq_gain_smooth[0]!=16384||s->eq_gain_smooth[1]!=16384||s->eq_gain_smooth[2]!=16384){
  int32_t low=s->eq_lp_low,mid=s->eq_lp_high-low,high=x-s->eq_lp_high;
  x=(int32_t)(((int64_t)low*s->eq_gain_smooth[0]+(int64_t)mid*s->eq_gain_smooth[1]+
      (int64_t)high*s->eq_gain_smooth[2])/16384);
 }
#ifdef RUPTURE_BUILD
 /* Signed HIGH AIR shelf: legacy 0 means unity. Positive codes 1..16384
  * add high detail; negative codes 16385..32767 reduce it. No cutoff jump. */
 if(c->eq_hpf){
  int32_t air=x-s->eq_lp_high;
  int32_t delta=(c->eq_hpf<=16384u)?(int32_t)c->eq_hpf:-((int32_t)c->eq_hpf-16384);
  x=clip16(x+(int32_t)(((int64_t)air*delta)/16384));
 }
 s->eq_lp_hpf=x;
#endif
 }
 }else{
  /* Skip EQ filters and band recombination entirely while bypassed.
   * Reset small one-pole state to avoid resurrecting an old tone. */
  s->eq_lp_low=s->eq_lp_high=s->eq_lp_hpf=0;
  s->eq_gain_smooth[0]=s->eq_gain_smooth[1]=s->eq_gain_smooth[2]=16384;
 }
 return clip16(x);
}

/* RUPTURE V1.80: all Q15 one-pole states and the allpass stage input
 * are bounded to signed 16-bit before the indicated multiply. The signed
 * difference bound is 65535, Q15 coefficient <=32767; product <=2147385345,
 * strictly below INT32_MAX. Keep truncation toward zero exactly unchanged.
 * These remove compiler-emulated 64-bit division from active phaser / tape /
 * wah paths without changing waveform arithmetic. See golden PCM tests. */
_Static_assert(65535LL*32767LL < INT32_MAX,"Q15 stage multiplication safe");
/* V1.02 six global FX: signed fixed point, no malloc/no libm. 44.1 kHz.
 * Mix parameters are slewed each sample. No DSP here touches flash/UI.
 * Every effect is exact bypass at its factory settings. */
/* A symmetric wavefolder must map digital silence to exact zero even
 * when the SYMMETRY knob is offset. Cache the zero-input transfer value
 * instead of doing a second eight-reflection calculation per audio sample. */
static int16_t fx_fold_transfer(int32_t x,uint16_t fold_control,uint16_t symmetry_control){
 int32_t symmetry=((int32_t)qclamp(symmetry_control)-16384)/2;
 int32_t fold=1+(int32_t)(qclamp(fold_control)*7u/32767u);
 int32_t y=(x+symmetry)*fold;
 for(int j=0;j<8;j++){
  if(y>32767)y=65534-y;
  else if(y<-32767)y=-65534-y;
  else break;
 }
 return clip16(y-symmetry);
}
static uint32_t fx_triangle(uint32_t phase){
 uint32_t p=phase>>16;return p<32768u?p*2u:131070u-p*2u;
}
/* RUPTURE V1.15: independent transient shaping controls.
 * This is deliberately not mapped to old ETCH FX slots or storage bytes. */
static volatile uint8_t rt_attack=50u,rt_sustain=50u,rt_sens=60u,rt_mix=0u,rt_on=0u;
void rupture_transient_controls(uint8_t attack,uint8_t sustain,uint8_t sensitivity,uint8_t mix,uint8_t enabled){
 rt_attack=attack>100u?100u:attack;
 rt_sustain=sustain>100u?100u:sustain;
 rt_sens=sensitivity>100u?100u:sensitivity;
 rt_mix=mix>100u?100u:mix;
 rt_on=enabled?1u:0u;
}
static int16_t rupture_transient_process(struct etch_native_fx *s,int16_t sample){
 int32_t target=(rt_on?(uint32_t)rt_mix*32767u/100u:0u);
 s->rupture_transient_mix=etch_fx_slew(s->rupture_transient_mix,target);
 if(s->rupture_transient_mix==0){s->rupture_transient_fast=0;s->rupture_transient_slow=0;return sample;}
 int32_t x=sample,mag=x<0?-x:x;
 /* Independent 1ms and ~18ms magnitude followers at 44.1kHz, fixed 1/256. */
 s->rupture_transient_fast+=((mag-s->rupture_transient_fast)*7)/256;
 s->rupture_transient_slow+=((mag-s->rupture_transient_slow)*1)/256;
 int32_t impulse=s->rupture_transient_fast-s->rupture_transient_slow;
 int32_t sens=(uint32_t)rt_sens*32767u/100u;
 int32_t attack=(int32_t)rt_attack-50;
 int32_t sustain=(int32_t)rt_sustain-50;
 /* Transient and sustain are bipolar: 50 neutral; bounded up to +6dB. */
 int32_t bump=(impulse*attack)/50;
 int32_t body=(s->rupture_transient_slow*sustain)/100;
 int32_t change=((bump+body)*sens)/32767;
 int32_t gain=32767+(change*32767)/20000;
 if(gain<4096)gain=4096;
 if(gain>65534)gain=65534;
 int32_t processed=clip16((x*gain)/32767);
 int32_t wet=s->rupture_transient_mix;
 return clip16(rupture_q15_mix(x,processed,(uint16_t)wet));
}
/* Tilt EQ and octave-up fuzz: zero delay allocation, bounded Q15 state. */
static volatile uint8_t rt_tilt_amount=50u,rt_tilt_freq=50u,rt_tilt_mix=0u,rt_tilt_output=50u,rt_tilt_on=0u;
/* V1.42: Convert knob values to audio-domain Q15 targets ONCE in the UI.
 * The audio IRQ only slews and multiplies. These are not stored project data. */
static volatile uint8_t rt_fuzz_drive=55u,rt_fuzz_octave=75u,rt_fuzz_tone=65u,rt_fuzz_mix=0u,rt_fuzz_on=0u;
static volatile int32_t rt_fuzz_gain_q15=32767+55000;
static volatile int32_t rt_fuzz_anti_alpha=32767-6600;
static volatile int32_t rt_fuzz_octave_q15=24575;
static volatile int32_t rt_fuzz_tone_alpha=600+20800;
void rupture_tilt_controls(uint8_t tilt,uint8_t frequency,uint8_t mix,uint8_t output,uint8_t enabled){
 rt_tilt_amount=tilt>100u?100u:tilt;rt_tilt_freq=frequency>100u?100u:frequency;
 rt_tilt_mix=mix>100u?100u:mix;rt_tilt_output=output>100u?100u:output;rt_tilt_on=enabled?1u:0u;
}
void rupture_fuzz_controls(uint8_t drive,uint8_t octave,uint8_t tone,uint8_t mix,uint8_t enabled){
 rt_fuzz_drive=drive>100u?100u:drive;rt_fuzz_octave=octave>100u?100u:octave;
 rt_fuzz_tone=tone>100u?100u:tone;rt_fuzz_mix=mix>100u?100u:mix;rt_fuzz_on=enabled?1u:0u;
 rt_fuzz_gain_q15=32767+(int32_t)rt_fuzz_drive*1000;
 rt_fuzz_anti_alpha=32767-(int32_t)rt_fuzz_drive*120;
 rt_fuzz_octave_q15=((int32_t)rt_fuzz_octave*32767+50)/100;
 rt_fuzz_tone_alpha=600+(int32_t)rt_fuzz_tone*320;
}
static int16_t rupture_tilt_process(struct etch_native_fx *s,int16_t sample){
 int32_t x=sample,target=rt_tilt_on?(uint32_t)rt_tilt_mix*32767u/100u:0u;
 s->rupture_tilt_mix=etch_fx_slew(s->rupture_tilt_mix,target);
 if(!s->rupture_tilt_mix){s->rupture_tilt_lp=0;return sample;}
 int32_t alpha=250+((uint32_t)rt_tilt_freq*(uint32_t)rt_tilt_freq*2u);
 s->rupture_tilt_lp+=((x-s->rupture_tilt_lp)*alpha)/32768;
 int32_t bass=s->rupture_tilt_lp,treble=x-bass;
 int32_t tilt=(int32_t)rt_tilt_amount-50;
 /* When tilt=0 and output=50, exact unity regardless of crossover position. */
 /* Strong, true bipolar frequency tilt. Difference and gain clipped
  * before multiplying to keep all fixed point math in signed 32 bits. */
 int32_t delta=clip16(treble-bass);
 int32_t wet=x+((delta*tilt*1)/60);
 int32_t out=(int32_t)rt_tilt_output;
 wet=clip16(wet);
 int32_t gain=(out<=50)?(8192+out*(32767-8192)/50):(32767+(out-50)*(65534-32767)/50);
 wet=(wet*gain)/32767;
 wet=clip16(wet);
 return clip16(rupture_q15_mix(x,wet,(uint16_t)s->rupture_tilt_mix));
}
/* Band-limit the octave rectifier at its zero-crossing without costly
 * oversampling. The polynomial is C1-continuous at +/-4096 PCM, exact
 * abs(x) outside that narrow region, and exactly zero for silence.
 * All products are <= 4096^2 and use shifts instead of division. */
/* V1.42 periodic fuzz makeup: 2 KiB flash-only reciprocal table.
 * wet_ref is 256..33023, so index is 4..516. No variable integer
 * division inside the audio IRQ; linear interpolation limits quantization.
 * Q24 reciprocal * dry_ref >> 9 produces Q15 output ratio. */
static const uint32_t rupture_fuzz_recip_q24[518]={
 0,0,0,0,65536,52429,43691,37449,32768,29127,
 26214,23831,21845,20165,18725,17476,16384,15420,14564,13797,
 13107,12483,11916,11398,10923,10486,10082,9709,9362,9039,
 8738,8456,8192,7944,7710,7490,7282,7085,6899,6722,
 6554,6394,6242,6096,5958,5825,5699,5578,5461,5350,
 5243,5140,5041,4946,4855,4766,4681,4599,4520,4443,
 4369,4297,4228,4161,4096,4033,3972,3913,3855,3799,
 3745,3692,3641,3591,3542,3495,3449,3404,3361,3318,
 3277,3236,3197,3158,3121,3084,3048,3013,2979,2945,
 2913,2881,2849,2819,2789,2759,2731,2703,2675,2648,
 2621,2595,2570,2545,2521,2497,2473,2450,2427,2405,
 2383,2362,2341,2320,2300,2280,2260,2241,2222,2203,
 2185,2166,2149,2131,2114,2097,2081,2064,2048,2032,
 2016,2001,1986,1971,1956,1942,1928,1913,1900,1886,
 1872,1859,1846,1833,1820,1808,1796,1783,1771,1759,
 1748,1736,1725,1713,1702,1691,1680,1670,1659,1649,
 1638,1628,1618,1608,1598,1589,1579,1570,1560,1551,
 1542,1533,1524,1515,1507,1498,1489,1481,1473,1464,
 1456,1448,1440,1432,1425,1417,1409,1402,1394,1387,
 1380,1372,1365,1358,1351,1344,1337,1331,1324,1317,
 1311,1304,1298,1291,1285,1279,1273,1266,1260,1254,
 1248,1242,1237,1231,1225,1219,1214,1208,1202,1197,
 1192,1186,1181,1176,1170,1165,1160,1155,1150,1145,
 1140,1135,1130,1125,1120,1116,1111,1106,1101,1097,
 1092,1088,1083,1079,1074,1070,1066,1061,1057,1053,
 1049,1044,1040,1036,1032,1028,1024,1020,1016,1012,
 1008,1004,1001,997,993,989,986,982,978,975,
 971,967,964,960,957,953,950,946,943,940,
 936,933,930,926,923,920,917,913,910,907,
 904,901,898,895,892,889,886,883,880,877,
 874,871,868,865,862,859,857,854,851,848,
 846,843,840,838,835,832,830,827,824,822,
 819,817,814,812,809,807,804,802,799,797,
 794,792,790,787,785,783,780,778,776,773,
 771,769,767,764,762,760,758,755,753,751,
 749,747,745,743,741,738,736,734,732,730,
 728,726,724,722,720,718,716,714,712,710,
 708,707,705,703,701,699,697,695,694,692,
 690,688,686,684,683,681,679,677,676,674,
 672,670,669,667,665,664,662,660,659,657,
 655,654,652,650,649,647,646,644,643,641,
 639,638,636,635,633,632,630,629,627,626,
 624,623,621,620,618,617,615,614,612,611,
 610,608,607,605,604,603,601,600,599,597,
 596,594,593,592,590,589,588,586,585,584,
 583,581,580,579,577,576,575,574,572,571,
 570,569,567,566,565,564,563,561,560,559,
 558,557,555,554,553,552,551,550,548,547,
 546,545,544,543,542,541,539,538,537,536,
 535,534,533,532,531,530,529,527,526,525,
 524,523,522,521,520,519,518,517,516,515,
 514,513,512,511,510,509,508,507,
};
static inline int32_t rupture_fuzz_ratio_q15(int32_t dry_ref,int32_t wet_ref){
 if(dry_ref>=wet_ref)return 32767;
 uint32_t idx=(uint32_t)wet_ref>>6, frac=(uint32_t)wet_ref&63u;
 uint32_t a=rupture_fuzz_recip_q24[idx],b=rupture_fuzz_recip_q24[idx+1u];
 uint32_t recip=(uint32_t)((int32_t)a+(((int32_t)b-(int32_t)a)*(int32_t)frac)/64);
 int32_t ratio=(int32_t)(((uint32_t)dry_ref*recip)>>9);
 return ratio<4096?4096:ratio>32767?32767:ratio;
}
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
  s->rupture_fuzz_makeup_target=rupture_fuzz_ratio_q15(dry_ref,wet_ref);
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
/* SANDPAPER: a no-delay gentle spectral polish before modulation effects.
 * SMOOTH moves a two-pole lowpass from open to rolled-off; SOFT rounds
 * protruding peaks without a hard gate; WARMTH adds a tiny sub-bass shelf.
 * Three bounded 32-bit one-pole states, no tables, malloc, delay or flash.
 * Each control is written by the UI; audio calls never touch persistent data.
 * Disabled state is sample-exact dry after the 64-sample wet ramp. */
static volatile uint8_t rt_sand_smooth=60u,rt_sand_soft=40u,rt_sand_warm=45u,rt_sand_mix=80u,rt_sand_on=0u;
void rupture_sandpaper_controls(uint8_t smooth,uint8_t soft,uint8_t warmth,uint8_t mix,uint8_t enabled){
 rt_sand_smooth=smooth>100u?100u:smooth;
 rt_sand_soft=soft>100u?100u:soft;
 rt_sand_warm=warmth>100u?100u:warmth;
 rt_sand_mix=mix>100u?100u:mix;
 rt_sand_on=enabled?1u:0u;
}
static int16_t rupture_sandpaper_process(struct etch_native_fx *s,int16_t sample){
 int32_t x=sample;
 int32_t target=rt_sand_on?(uint32_t)rt_sand_mix*32767u/100u:0u;
 s->rupture_sand_mix=etch_fx_slew(s->rupture_sand_mix,target);
 if(!s->rupture_sand_mix){
  s->rupture_sand_pole1=s->rupture_sand_pole2=s->rupture_sand_body=0;
  s->rupture_sand_soft_slew=0;s->rupture_sand_warm_slew=0;s->rupture_sand_smooth_slew=0;
  return sample;
 }
 /* Control range ~9 kHz down to ~600 Hz at nominal 44.1 kHz.
  * Two poles temper grittiness without removing the sub fundamental. */
 s->rupture_sand_smooth_slew=etch_fx_slew(s->rupture_sand_smooth_slew,(int32_t)rt_sand_smooth*256);
 int32_t a=26000-(uint32_t)(s->rupture_sand_smooth_slew>>8)*22000u/100u;
 int32_t delta=x-s->rupture_sand_pole1;
 s->rupture_sand_pole1+=(delta*a)>>15;
 delta=s->rupture_sand_pole1-s->rupture_sand_pole2;
 s->rupture_sand_pole2+=(delta*a)>>15;
 /* Keep low body independently of the upper-spectrum smoothing. */
 s->rupture_sand_body+=((x-s->rupture_sand_body)*650)>>15;
 /* SOFT and WARMTH both have audible ranges. Avoid step discontinuities
  * by slewing each knob in audio, without IRQ-time division or buffers. */
 int32_t soft_target=(int32_t)rt_sand_soft*256;
 int32_t warm_target=(int32_t)rt_sand_warm*256;
 s->rupture_sand_soft_slew=etch_fx_slew(s->rupture_sand_soft_slew,soft_target);
 s->rupture_sand_warm_slew=etch_fx_slew(s->rupture_sand_warm_slew,warm_target);
 int32_t soft=s->rupture_sand_soft_slew>>8,warm=s->rupture_sand_warm_slew>>8;
 int32_t wet=s->rupture_sand_pole2;
 int32_t mag=wet<0?-wet:wet;
 /* Smooth amplitude-dependent saturation, including below 6000 PCM;
  * factor min ~40% at max SOFT, no abrupt knee at crossing. */
 int32_t reduction=(soft*(3000+(mag>>1)))/100;
 int32_t factor=32767-reduction;
 if(factor<12000)factor=12000;
 wet=(wet*factor)/32767;
 /* 0..60% bass-body reinforcement; double pole remains clean. */
 wet+=(s->rupture_sand_body*warm*3)/500;
 wet=clip16(wet);
 return clip16(rupture_q15_mix(x,wet,(uint16_t)s->rupture_sand_mix));
}
static int16_t fx_v102_process(struct etch_native_fx *s,int16_t sample,const struct etch_fx_settings *c,uint8_t ordered){
 int32_t x=sample,wet=0;
#ifdef RUPTURE_BUILD
 /* Normal RUPTURE tiles use COMP + TILT + OCT FUZZ + SANDPAPER.
  * All the remaining ETCH modulation engines have no active page on the
  * instrument. When their wet parameters AND fades are zero, skip their
  * expensive per-sample setup, math and unneeded delay-buffer updates.
  * If a saved legacy project uses one, retain the original full path. */
 /* Mode 2 is the ordered chain's explicitly proven COMP/TILT/FUZZ/SAND
  * quartet. It must not run WAH or DISTORTION ahead of their later slots,
  * and it must not reset their wet-mix smoothing state. */
 if(ordered==2u || (!ordered && !c->trem_mix && !c->vibrato_mix && !c->wah_mix &&
    !c->phaser_mix && !c->flanger_mix && !c->tape_mix &&
    !c->decim_mix && !c->folder_mix &&
    !s->trem_mix_smooth && !s->vibrato_mix_smooth &&
    !s->wah_mix_smooth && !s->phaser_mix_smooth &&
    !s->flanger_mix_smooth && !s->tape_mix_smooth &&
    !s->decim_mix_smooth && !s->folder_mix_smooth)){
  if(c->comp_ratio){
   int32_t mag=x<0?-x:x;
   int32_t atk=s->comp_attack_step;
   int32_t rel=s->comp_release_step;
   int32_t step=mag>s->comp_env?atk:rel;
   s->comp_env+=((mag-s->comp_env)*step)/32768;
   if(++s->comp_tick>=16){
    s->comp_tick=0;
    int32_t thr=s->comp_threshold_cached;
    int32_t ratio=s->comp_ratio_cached;
    int32_t target=32767;
    if(s->comp_env>thr){
     int32_t limited=thr+(s->comp_env-thr)/ratio;
     target=(limited*32767)/s->comp_env;
    }
    s->comp_target=target;
   }
   s->comp_gain=etch_fx_slew(s->comp_gain,s->comp_target);
   x=(x*s->comp_gain)/32767;
  }else{
   /* Release compressor gain smoothly when bypassed. The old immediate
    * reset from a compressed gain to unity made a click, and kept the
    * non-dry chain busy on successive effects. Exact when already unity. */
   s->comp_target=32767;
   s->comp_gain=etch_fx_slew(s->comp_gain,32767);
   if(s->comp_gain!=32767)x=(x*s->comp_gain)/32767;
   s->comp_env=0;s->comp_tick=0;
  }
  return rupture_sandpaper_process(s,rupture_fuzz_process(s,rupture_tilt_process(s,clip16(x))));
 }
#endif
 /* Compressor: envelope follower, 16-sample control rate, smoothly applied
  * gain. Ratio 1:1 (zero control) provides exact bypass. */
 if(!ordered){
 if(c->comp_ratio){
  int32_t mag=x<0?-x:x;
#ifdef RUPTURE_BUILD
  int32_t atk=s->comp_attack_step;
  int32_t rel=s->comp_release_step;
#else
  int32_t atk=2+(32767u-qclamp(c->comp_attack))/512u;
  int32_t rel=1+(32767u-qclamp(c->comp_release))/2048u;
#endif
  int32_t step=mag>s->comp_env?atk:rel;
  s->comp_env+=((mag-s->comp_env)*step)/32768;
  if(++s->comp_tick>=16){
   s->comp_tick=0;
#ifdef RUPTURE_BUILD
   int32_t thr=s->comp_threshold_cached;
   int32_t ratio=s->comp_ratio_cached;
#else
   int32_t thr=2000+(32767u-qclamp(c->comp_threshold))*25000u/32767u;
   int32_t ratio=1+qclamp(c->comp_ratio)*11u/32767u;
#endif
   int32_t target=32767;
   if(s->comp_env>thr){
    int32_t limited=thr+(s->comp_env-thr)/ratio;
    target=(limited*32767)/s->comp_env;
   }
   s->comp_target=target;
  }
  s->comp_gain=etch_fx_slew(s->comp_gain,s->comp_target);
  x=(x*s->comp_gain)/32767;
 }else{
   /* Release compressor gain smoothly when bypassed. The old immediate
    * reset from a compressed gain to unity made a click, and kept the
    * non-dry chain busy on successive effects. Exact when already unity. */
   s->comp_target=32767;
   s->comp_gain=etch_fx_slew(s->comp_gain,32767);
   if(s->comp_gain!=32767)x=(x*s->comp_gain)/32767;
   s->comp_env=0;s->comp_tick=0;
  }
 }
 /* Tremolo: 0.15..15Hz, triangle -> square, wet/dry blend. */
 s->trem_mix_smooth=etch_fx_slew(s->trem_mix_smooth,qclamp(c->trem_mix));
 if(s->trem_mix_smooth){
  uint32_t tri=fx_triangle(s->trem_phase), square=(s->trem_phase&0x80000000u)?0u:65535u;
  uint32_t sh=qclamp(c->trem_shape);uint32_t lfo=(tri*(32767u-sh)+square*sh)/32767u;
  uint32_t depth=qclamp(c->trem_depth);
  uint32_t amp=32767u-(uint32_t)(((uint32_t)(65535u-lfo)*depth)/65535u);
  wet=(int32_t)(((int32_t)x*(int32_t)amp)/32767);
  x=ru_fx_delta_mix(x,wet,s->trem_mix_smooth);
 }
 s->trem_phase+=s->trem_step;
 /* V1.06 Vibrato: fractional variable delay, not amplitude tremolo.
  * Slow triangle/square morph varies read time by up to +/- ~3ms.
  * Read head always trails write head; no dynamic allocations or libm. */
 s->vibrato_mix_smooth=etch_fx_slew(s->vibrato_mix_smooth,qclamp(c->vibrato_mix));
 vibrato_line[s->vibrato_pos]=clip16(x);
 if(s->vibrato_mix_smooth){
  uint32_t tri=fx_triangle(s->vibrato_phase);
  uint32_t squ=(s->vibrato_phase&0x80000000u)?0u:65535u;
  uint32_t shape=qclamp(c->vibrato_shape);
  uint32_t lfo=(tri*(32767u-shape)+squ*shape)/32767u;
  int32_t offset=(int32_t)lfo-32767;
  int32_t shift=(int32_t)(((int64_t)offset*qclamp(c->vibrato_depth)*140u*256)/(32767*32767));
  uint32_t distance=(uint32_t)(200*256+shift);
  uint16_t a=(uint16_t)((s->vibrato_pos-(distance>>8))&511u);
  int32_t wetv=(vibrato_line[a]*(int32_t)(256-(distance&255u))+
     vibrato_line[(a-1u)&511u]*(int32_t)(distance&255u))/256;
  x=ru_fx_delta_mix(x,wetv,s->vibrato_mix_smooth);
 }
 s->vibrato_pos=(uint16_t)((s->vibrato_pos+1u)&511u);
 s->vibrato_phase+=s->vibrato_step;
 /* V1.06 Auto-Wah: smoothed envelope controls resonant-like bandpass.
  * Strictly bounded one-pole sections and wet slew, no unstable feedback. */
 s->wah_mix_smooth=etch_fx_slew(s->wah_mix_smooth,qclamp(c->wah_mix));
 if(s->wah_mix_smooth){
  int32_t mag=x<0?-x:x;
  uint32_t attack=4u+(32767u-qclamp(c->wah_attack))*1000u/32767u;
  uint32_t rel=16u;
  s->wah_env+=(int32_t)(((int32_t)(mag-s->wah_env)*(int32_t)(mag>s->wah_env?attack:rel))/32768);
  int32_t response=(int32_t)(((int32_t)s->wah_env*qclamp(c->wah_sens))/32767);
  if(response>24000)response=24000;
  /* 15000 / 24000 == 5 / 8: exact integer result, no division. */
  uint32_t alpha=300u+(((uint32_t)response*5u)>>3u);
  s->wah_lp+=(int32_t)(((int32_t)(x-s->wah_lp)*(int32_t)alpha)/32768);
  s->wah_hp+=(int32_t)(((int32_t)(s->wah_lp-s->wah_hp)*(int32_t)((alpha>>2)+100u))/32768);
  int32_t band=s->wah_lp-s->wah_hp;
  int32_t resonance=16384+(int32_t)qclamp(c->wah_reso)*16383/32767;
  int32_t wetwah=clip16((int32_t)(((int32_t)band*resonance)/16384));
  x=ru_fx_delta_mix(x,wetwah,s->wah_mix_smooth);
 }else s->wah_env=0;
 /* Four cascaded stable first-order all-pass stages with bounded feedback. */
 s->phaser_mix_smooth=etch_fx_slew(s->phaser_mix_smooth,qclamp(c->phaser_mix));
 if(s->phaser_mix_smooth){
  uint32_t tri=fx_triangle(s->phaser_phase);
  int32_t coeff=5000+(int32_t)(((uint64_t)tri*qclamp(c->phaser_depth)*22500u)/(65535u*32767u));
  int32_t fb=(int32_t)(((int64_t)s->phaser_feedback_state*qclamp(c->phaser_feedback)*6)/(32767u*10u));
  int32_t y=clip16(x+fb);
  for(int j=0;j<4;j++){
   int32_t v=s->phaser_z[j]-(int32_t)(((int32_t)coeff*y)/32768);
   s->phaser_z[j]=clip16(y+(int32_t)(((int32_t)coeff*v)/32768));
   y=clip16(v);
  }
  s->phaser_feedback_state=y;
  x=ru_fx_delta_mix(x,y,s->phaser_mix_smooth);
 }
 s->phaser_phase+=s->phaser_step;
 /* Flanger: 1024 signed samples (2 KiB), modulated 1..12ms delay. */
 s->flanger_mix_smooth=etch_fx_slew(s->flanger_mix_smooth,qclamp(c->flanger_mix));
 if(s->flanger_mix_smooth){
  uint32_t tri=fx_triangle(s->flanger_phase);
  uint32_t offset=32u+(uint32_t)(((uint64_t)tri*qclamp(c->flanger_depth)*490u)/(65535u*32767u));
  uint32_t index=(s->flanger_pos-offset)&511u;
  wet=s->flanger_line[index];
  int32_t fb=(int32_t)(((int64_t)wet*qclamp(c->flanger_feedback)*65)/(32767u*100u));
  s->flanger_line[s->flanger_pos]=clip16(x+fb);
  s->flanger_pos=(s->flanger_pos+1u)&511u;
  x=ru_fx_delta_mix(x,wet,s->flanger_mix_smooth);
 }
 s->flanger_phase+=s->flanger_step;
 /* Tape Instability: variable interpolated short delay yields real pitch wow
  * and flutter, not tremolo. The tape head is global once after voice mix.
  * Read distance: 180 +/- 90 (wow) +/- 15 (flutter) samples, always
  * >=75 samples, never reads the write head. Crossfade wet/dry smoothly.
  * AGE applies bounded lowpass + soft saturation; default is exact bypass.
  */
 s->tape_mix_smooth=etch_fx_slew(s->tape_mix_smooth,qclamp(c->tape_mix));
 tape_line[s->tape_pos]=clip16(x);
 if(s->tape_mix_smooth){
  int32_t wow=(int32_t)fx_triangle(s->tape_wow_phase)-32767;
  int32_t flutter=(int32_t)fx_triangle(s->tape_flutter_phase)-32767;
  int32_t delay_q8=180*256;
  delay_q8+=(int32_t)(((int64_t)wow*qclamp(c->tape_wow)*90*256)/(32767*32767));
  delay_q8+=(int32_t)(((int64_t)flutter*qclamp(c->tape_flutter)*15*256)/(32767*32767));
  uint32_t delay=(uint32_t)delay_q8;
  uint16_t rd=(uint16_t)((s->tape_pos-(delay>>8))&511u);
  int32_t a=tape_line[rd],b=tape_line[(rd-1u)&511u];
  int32_t y=(int32_t)((a*(256-(int32_t)(delay&255u))+b*(int32_t)(delay&255u))/256);
  uint32_t age=qclamp(c->tape_age);
  uint32_t alpha=32767u-(age*26000u/32767u);
  s->tape_tone_state+=(int32_t)(((int32_t)(y-s->tape_tone_state)*(int32_t)alpha)/32768);
  y=s->tape_tone_state;
  /* Subtle tape drive with hard numerical bounds and no artificial hiss. */
  if(age){
   int32_t drive=32767u+age/2u;
   int32_t raw=(int32_t)(((int32_t)y*drive)/32767);
   y=clip16(raw);
  }
  x=ru_fx_delta_mix(x,y,s->tape_mix_smooth);
 }else s->tape_tone_state=x;
 s->tape_pos=(uint16_t)((s->tape_pos+1u)&511u);
 s->tape_wow_phase+=s->tape_wow_step;
 s->tape_flutter_phase+=s->tape_flutter_step;
 /* Decimator: 4..16 significant bits, hold 1..64 samples. */
 s->decim_mix_smooth=etch_fx_slew(s->decim_mix_smooth,qclamp(c->decim_mix));
 if(s->decim_mix_smooth){
  uint32_t bits=4u+qclamp(c->decim_bits)*12u/32767u;
  uint32_t skip=16u-bits;
  uint32_t hold=1u+qclamp(c->decim_rate)*63u/32767u;
  if(!s->decim_counter){
   int32_t driven=clip16((int32_t)(((int64_t)x*(32767u+qclamp(c->decim_drive)*3u))/32767u));
   s->decim_held=(driven / (int32_t)(1u<<skip))*(int32_t)(1u<<skip);
   s->decim_counter=(uint16_t)hold;
  }
  s->decim_counter--;
  x+=(int32_t)(((int32_t)(s->decim_held-x)*s->decim_mix_smooth)/32767);
 }else s->decim_counter=0;
 /* Wavefolder intentionally removed from RUPTURE due to hard clicks.
  * Keep legacy ETCH implementation for upstream compatibility only. */
#ifndef RUPTURE_BUILD
 s->folder_mix_smooth=etch_fx_slew(s->folder_mix_smooth,qclamp(c->folder_mix));
 if(s->folder_mix_smooth){
  if(!s->folder_baseline_valid || s->folder_last_fold!=c->folder_fold ||
     s->folder_last_symmetry!=c->folder_symmetry){
   s->folder_baseline=fx_fold_transfer(0,c->folder_fold,c->folder_symmetry);
   s->folder_last_fold=c->folder_fold;
   s->folder_last_symmetry=c->folder_symmetry;
   s->folder_baseline_valid=1u;
  }
  /* Fold > 50%: prefilter before nonlinear reflections to reduce the
   * fold-down products at the top of the 44.1-kHz band. This is NOT
   * full bandlimited oversampling, but uses just one state and multiply. */
  int32_t fold_input=clip16(x);
  int32_t aa_alpha=32767;
  if(c->folder_fold>16384u)
   aa_alpha-=((uint32_t)(c->folder_fold-16384u)*12000u)/16383u;
  s->rupture_folder_input_lp+=((fold_input-s->rupture_folder_input_lp)*aa_alpha)/32768;
  int32_t y=(int32_t)fx_fold_transfer(s->rupture_folder_input_lp,c->folder_fold,c->folder_symmetry)-s->folder_baseline;
  y=clip16(y);
  uint32_t alpha=256u+qclamp(c->folder_tone)*32511u/32767u;
  s->folder_tone_state+=((y-s->folder_tone_state)*alpha)/32767;
  x=rupture_q15_mix(x,s->folder_tone_state,(uint16_t)s->folder_mix_smooth);
 }else{s->folder_tone_state=x;s->rupture_folder_input_lp=0;}
#else
 s->folder_mix_smooth=0;s->rupture_folder_input_lp=0;
#endif
 return ordered?clip16(x):rupture_sandpaper_process(s,rupture_fuzz_process(s,rupture_tilt_process(s,clip16(x))));
}

/* RUPTURE V1.80: bounded true delay-network reverbs.
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
 (void)decay; /* feedback gain is precomputed from the decay control */
 /* Two delay lines fill the existing 4096 sample tank, independent of hall.
  * Asymmetric 2039/2057 samples => ~46.2/46.6 ms at 44.1 kHz. */
 const uint32_t p=s->reverb_pos;
 s->reverb_pos=(p+1u)&4095u;
 const uint32_t k0=s->reverb_line_cursor[0];
 const uint32_t k1=s->reverb_line_cursor[1];
 const int32_t d0=s->reverb[k0];
 const int32_t d1=s->reverb[2039u+k1];
 /* Remove only the deepest subsonic energy from the wet excitation. */
 s->spring_input_lowpass+=(input-s->spring_input_lowpass)>>6;
 /* Recirculate some scattered wave energy through both dispersive stages.
  * This is essential: a single allpass on dry input alone is not a spring
  * model and would leave an ordinary two-tap comb after the onset. */
 int32_t exc=clip16(input-(s->spring_input_lowpass>>1)+((d0+d1)>>3));
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
 int32_t fb=s->reverb_spring_feedback; /* cached at knob updates */
 int32_t w0=(dispersed2>>2)+((s->reverb_line_damp[0]*fb)>>15)
            +((s->reverb_line_damp[1]*1400)>>15);
 int32_t w1=(dispersed>>2)+((s->reverb_line_damp[1]*fb)>>15)
            +((s->reverb_line_damp[0]*1400)>>15);
 s->reverb[k0]=clip16(w0);
 s->reverb[2039u+k1]=clip16(w1);
 s->reverb_line_cursor[0]=(uint16_t)(k0+1u==s->tap1?0u:k0+1u);
 s->reverb_line_cursor[1]=(uint16_t)(k1+1u==s->tap2?0u:k1+1u);
 /* Different dispersive line lengths provide the stereo decorrelation. */
 *wl=clip16(((d0*5+d1*3)*3)>>3);
 *wr=clip16(((d0*3+d1*5)*3)>>3);
}
static inline void ru_reverb_room(struct etch_native_fx *s,int32_t input,uint16_t decay,
                           int32_t *wl,int32_t *wr){
 (void)decay; /* feedback gain is precomputed from the decay control */
 /* Four real feedback lines. Unlike V1.78 ROOM (one circular delay with
  * 5 taps), each line has its own memory/cursor and damping pole.
  * Unequal line lengths: 16.5, 20.0, 23.0 and 33.4 milliseconds. */
 static const uint16_t starts[4]={0,727,1608,2621};
 int32_t v[4];
 for(uint32_t i=0;i<4u;i++)v[i]=s->reverb[starts[i]+s->reverb_line_cursor[i]];
 /* Householder reflector for N=4: m_i = (sum(v)/2)-v_i.
  * Energy-preserving coupling prior to damping and decay. */
 const int32_t gain=s->reverb_room_feedback; /* cached at knob updates */
 for(uint32_t i=0;i<4u;i++){
  int32_t diff=v[i]-s->reverb_line_damp[i];
  s->reverb_line_damp[i]+=diff>>1; /* cheap absorptive wall material */
 }
 /* Damp the feedback state itself, rather than the dry excitation. */
 const int32_t filtered=(s->reverb_line_damp[0]+s->reverb_line_damp[1]+
                         s->reverb_line_damp[2]+s->reverb_line_damp[3])>>1;
 for(uint32_t i=0;i<4u;i++){
  int32_t reflected=filtered-s->reverb_line_damp[i];
  int32_t injection=(i&1u)?-(input>>1):(input>>1);
  s->reverb[starts[i]+s->reverb_line_cursor[i]]=clip16(injection+((reflected*gain)>>15));
  uint32_t next=s->reverb_line_cursor[i]+1u;
  uint32_t length=i==0u?s->tap1:i==1u?s->tap2:i==2u?s->tap3:s->reverb_room_line4;
  s->reverb_line_cursor[i]=(uint16_t)(next==length?0u:next);
 }
 s->reverb_pos=(s->reverb_pos+1u)&4095u;
 /* Unequal early/late reflections; gently decorrelated stereo returns.
  * The early FDN arrivals are naturally at 16..33 ms, not dry bleed. */
 *wl=clip16(((v[0]*4+v[1]*3+v[2]*2-v[3])*3)>>3);
 *wr=clip16(((v[0]*2-v[1]+v[2]*4+v[3]*3)*3)>>3);
}
static inline void ru_reverb_hall(struct etch_native_fx *s,int32_t input,uint16_t decay,
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
void etch_fx_process_stereo_masked(struct etch_native_fx *s,int16_t in,uint16_t chorus,uint16_t delay,uint16_t reverb,const struct etch_fx_settings *cfg,uint32_t bypass_mask,int16_t *left,int16_t *right){
 if(!left||!right)return;
 if(!s){*left=*right=in;return;}
 struct etch_fx_settings defaults;
 if(!cfg){etch_fx_defaults(&defaults);cfg=&defaults;}
 chorus=qclamp(chorus);delay=qclamp(delay);reverb=qclamp(reverb);
 /* Smooth physical SEL bypass toggles instead of amputating wet signal.
  * All mixing stays in the shared global FX instance, not per voice.
  * Existing exact-dry signal is preserved when both ramps are zero. */
 const uint16_t chorus_target=(bypass_mask&ETCH_FXB_BIT(ETCH_FXB_CHORUS))?0u:chorus;
 const uint16_t delay_target=(bypass_mask&ETCH_FXB_BIT(ETCH_FXB_DELAY))?0u:delay;
 const uint16_t reverb_target=(bypass_mask&ETCH_FXB_BIT(ETCH_FXB_REVERB))?0u:reverb;
 s->chorus_mix_smooth=etch_fx_slew(s->chorus_mix_smooth,chorus_target);
 s->delay_mix_smooth=etch_fx_slew(s->delay_mix_smooth,delay_target);
 const uint16_t chorus_render=(uint16_t)s->chorus_mix_smooth;
 const uint16_t delay_render=(uint16_t)s->delay_mix_smooth;
 /* V1.10 fast dry: return EXACT samples, not a signal that traversed
  * apparently disabled effects. Skips costly filters, 64-bit multiplies,
  * multiple circular-buffer reads and the serial wet safety knee.
  * Never cut an ongoing wet->dry slew short: all smooth controls must
  * have naturally reached zero, and previous EQ/compression must be unity. */
 /* EQ bypass must be truly dry even if a stored EQ page still has nonzero
  * gains/frequencies. The old dry-control test inspected EQ knobs without
  * respecting the EQ bypass flag, activating the wet safety knee while EQ
  * was OFF. Do not copy/modify the large FX settings struct per sample. */
 const uint8_t eq_masked=(uint8_t)!!(bypass_mask&ETCH_FXB_BIT(ETCH_FXB_EQ));
 const uint8_t controls_dry=(uint8_t)(etch_fx_controls_dry(cfg,chorus,delay,reverb) ||
    (eq_masked && !chorus && !delay && !reverb &&
     !cfg->ring_mix && !cfg->comp_ratio && !cfg->trem_mix &&
     !cfg->phaser_mix && !cfg->flanger_mix && !cfg->decim_mix &&
     !cfg->folder_mix && !cfg->tape_mix && !cfg->reverse_mix &&
     !cfg->vibrato_mix && !cfg->rotary_mix && !cfg->wah_mix));
 if(controls_dry &&
#ifdef RUPTURE_BUILD
    (!s->eq2_enabled || (bypass_mask&ETCH_FXB_BIT(ETCH_FXB_EQ)) || rupture_eq2_is_unity(&s->eq2)) &&
    /* RUPTURE 1.39: when the parametrical EQ is at unity, its 4 biquads
     * must not keep every supposedly DRY sample in the expensive wet chain.
     * Active/tailing EQ still processes normally until all ramps settle. */
#endif
    !s->reverb_mix_smooth && !s->chorus_mix_smooth && !s->delay_mix_smooth &&
    !s->ring_mix_smooth && !s->trem_mix_smooth && !s->phaser_mix_smooth &&
    !s->flanger_mix_smooth && !s->decim_mix_smooth &&
    !s->folder_mix_smooth && !s->tape_mix_smooth &&
    
    !rt_tilt_on && !s->rupture_tilt_mix &&
    !rt_fuzz_on && !s->rupture_fuzz_mix &&
    !rt_sand_on && !s->rupture_sand_mix &&
    !s->reverse_mix_smooth && !s->vibrato_mix_smooth &&
    !s->rotary_mix_smooth && !s->wah_mix_smooth &&
    s->eq_gain_smooth[0]==16384 && s->eq_gain_smooth[1]==16384 &&
    s->eq_gain_smooth[2]==16384 && s->comp_gain==32767){
   /* Previously the dry fast-path froze every delay/reverb ring. A later
    * wet preset could replay obsolete audio from the previous patch.
    * Clear one indexed position per sample during dry playback; after
    * 16384 samples (~372 ms @44.1 kHz) every FX ring is flushed.
    * This costs bounded stores and NEVER stalls the DMA IRQ with memset. */
#ifdef RUPTURE_BUILD
   /* A neutral EQ2 is mathematically an exact pass-through. Keep only its
    * two latest samples warm while skipping all 20 coefficients/biquads:
    * activating a bell/shelf will not start with stale zero IIR history. */
   if(s->eq2_enabled && rupture_eq2_is_unity(&s->eq2)){
    for(uint8_t b=0u;b<4u;b++){
     s->eq2.x2[b]=s->eq2.x1[b];s->eq2.x1[b]=in;
     s->eq2.y2[b]=s->eq2.y1[b];s->eq2.y1[b]=in;
    }
   }
#endif
   if(s->dry_was_wet){
    s->dry_was_wet=0u;s->dry_flush_active=1u;s->dry_flush_cursor=0u;
   }
   if(s->dry_flush_active){
    uint32_t i=s->dry_flush_cursor;
    s->delay[i]=0;
    if(i<ETCH_REVERB_SIZE)s->reverb[i]=0;
    if(i<ETCH_CHORUS_SIZE)s->chorus[i]=0;
    if(i<64u)space_diffuser[i]=0;
    if(i<3584u)reverse_line[i]=0;
    if(i<1024u)s->flanger_line[i]=0;
    if(i<512u){vibrato_line[i]=0;tape_line[i]=0;}
    if(++s->dry_flush_cursor==ETCH_DELAY_SIZE){
     s->dry_flush_active=0u;s->dry_flush_cursor=0u;
     /* Small persistent filter states are not allowed to resurrect a tone. */
     s->eq_lp_low=s->eq_lp_high=s->eq_lp_hpf=0;
     s->phaser_feedback_state=s->flanger_feedback_state=0;
     for(uint32_t z=0;z<4u;z++)s->phaser_z[z]=0;
     s->wah_env=s->wah_lp=s->wah_hp=0;
     s->tape_tone_state=s->reverse_tone_state=0;
     s->folder_tone_state=s->folder_baseline=0;
     s->space_damp_state=0;
     s->reverse_accum=0;s->reverse_ready=0;
    }
   }
   *left=in;*right=in;return;
 }
 s->dry_was_wet=1u;s->dry_flush_active=0u;
 /* Host-updated FX settings change at UI rate. Never perform parameter
  * conversion divisions inside the 44.1 kHz audio loop unnecessarily. */
 if(!s->applied_valid||
    s->applied.chorus_rate!=cfg->chorus_rate||s->applied.chorus_depth!=cfg->chorus_depth||
    s->applied.delay_time!=cfg->delay_time||s->applied.delay_feedback!=cfg->delay_feedback||
    s->applied.reverb_size!=cfg->reverb_size||s->applied.reverb_decay!=cfg->reverb_decay||
    s->applied.routing!=cfg->routing||
    s->applied.ring_rate!=cfg->ring_rate||s->applied.eq_hpf!=cfg->eq_hpf||
    /* Avoid checking inactive compressor coefficients at the sample rate.
     * The ratio transition itself still invalidates all cached values. */
    s->applied.comp_ratio!=cfg->comp_ratio||
    (cfg->comp_ratio &&
     (s->applied.comp_attack!=cfg->comp_attack||s->applied.comp_release!=cfg->comp_release||
      s->applied.comp_threshold!=cfg->comp_threshold))||
    s->applied.trem_rate!=cfg->trem_rate||s->applied.phaser_rate!=cfg->phaser_rate||
    s->applied.flanger_rate!=cfg->flanger_rate||s->applied.tape_wow!=cfg->tape_wow||
    s->applied.tape_flutter!=cfg->tape_flutter||
    s->applied.vibrato_rate!=cfg->vibrato_rate||s->applied.rotary_rate!=cfg->rotary_rate){
  s->applied=*cfg;s->applied_valid=1;
  /* Identical V1.68 integer rounding. Recompute only on parameter changes,
   * not at the 44.1 kHz sample rate. Includes live control automation. */
#ifdef RUPTURE_BUILD
  s->comp_attack_step=(uint8_t)(2u+(32767u-qclamp(cfg->comp_attack))/512u);
  s->comp_release_step=(uint8_t)(1u+(32767u-qclamp(cfg->comp_release))/2048u);
  s->comp_threshold_cached=(uint16_t)(2000u+(32767u-qclamp(cfg->comp_threshold))*25000u/32767u);
  s->comp_ratio_cached=(uint8_t)(1u+qclamp(cfg->comp_ratio)*11u/32767u);
#endif
  /* 1..1600Hz, gentle resolution around low rates. */
  uint64_t r=qclamp(cfg->ring_rate);
  uint32_t hz=1u+(uint32_t)((r*r*1599u)/(32767u*32767u));
  s->ring_increment=hz*97391u;
  s->trem_step=14609u+(uint32_t)qclamp(cfg->trem_rate)*1450000u/32767u;
  s->phaser_step=2922u+(uint32_t)qclamp(cfg->phaser_rate)*980000u/32767u;
  s->flanger_step=2922u+(uint32_t)qclamp(cfg->flanger_rate)*980000u/32767u; /* round(2^32/44100), bounded */
  s->vibrato_step=19479u+(uint32_t)qclamp(cfg->vibrato_rate)*856020u/32767u; /* .2..9Hz */
  s->rotary_step=19479u+(uint32_t)qclamp(cfg->rotary_rate)*681520u/32767u; /* .2..7.2Hz */
  /* 0.13..1.3 Hz and 3..12 Hz @44.1k, independent modulator clocks. */
  s->tape_wow_step=12659u+(uint32_t)qclamp(cfg->tape_wow)*113700u/32767u;
  s->tape_flutter_step=292153u+(uint32_t)qclamp(cfg->tape_flutter)*876457u/32767u;
 #ifndef RUPTURE_BUILD
 s->eq_hpf_alpha=(uint16_t)(((uint32_t)qclamp(cfg->eq_hpf)*1700u)/32767u);
#endif
  s->rate_step=1u+(uint32_t)qclamp(cfg->chorus_rate)*9u/32768u;
  /* Precompute span at UI rate, not with an expensive 64-bit audio multiply. */
  s->chorus_span=(uint16_t)((uint32_t)qclamp(cfg->chorus_depth)*924u/32767u);
  s->space_damp_alpha=(uint16_t)(7000u+(uint32_t)(32767u-qclamp(cfg->reverb_decay))*8000u/32767u);
  s->delay_samples=384u+(uint32_t)qclamp(cfg->delay_time)*15488u/32767u;
  /* Distinct delay geometries, computed only when settings change. */
  uint32_t span=(uint32_t)qclamp(cfg->reverb_size);
  switch(etch_fx_reverb_type(cfg)){
   case 0: /* SPRING: two independent coils in the existing 4096-word tank. */
    s->tap1=(uint16_t)(641u+span*1398u/32767u); /* max 2039 */
    s->tap2=(uint16_t)(803u+span*1254u/32767u); /* max 2057 */
    s->reverb_spring_feedback=(uint16_t)(18500u+(uint32_t)qclamp(cfg->reverb_decay)*10200u/32767u);
    break;
   case 2: /* ROOM: 4 independent delays, max capacities 727/881/1013/1475. */
    s->tap1=(uint16_t)(241u+span*486u/32767u);
    s->tap2=(uint16_t)(283u+span*598u/32767u);
    s->tap3=(uint16_t)(377u+span*636u/32767u);
    s->reverb_room_line4=(uint16_t)(489u+span*986u/32767u);
    s->reverb_room_feedback=(uint16_t)(20000u+(uint32_t)qclamp(cfg->reverb_decay)*10000u/32767u);
    break;
   default: /* HALL: preserve V1.79 timing, tone and PCM. */
    s->tap1=(uint16_t)(521u+span*727u/32767u);
    s->tap2=(uint16_t)(1471u+span*1201u/32767u);
    s->tap3=(uint16_t)(2897u+span*1133u/32767u);
    break;
  }
  /* A SIZE change can shorten a delay line while its cursor is beyond
   * the new bound. Clamp ONCE at the control update, not per sample. */
  if(etch_fx_reverb_type(cfg)==0u || etch_fx_reverb_type(cfg)==2u){
   uint16_t len[4]={s->tap1,s->tap2,s->tap3,s->reverb_room_line4};
   uint32_t count=etch_fx_reverb_type(cfg)==0u?2u:4u;
   for(uint32_t j=0;j<count;j++)if(s->reverb_line_cursor[j]>=len[j])s->reverb_line_cursor[j]=0u;
  }
 }
 uint16_t feedback=qclamp(cfg->delay_feedback),decay=qclamp(cfg->reverb_decay);
 if((bypass_mask&(ETCH_FXB_BIT(ETCH_FXB_RING)|ETCH_FXB_BIT(ETCH_FXB_EQ))) ==
    (ETCH_FXB_BIT(ETCH_FXB_RING)|ETCH_FXB_BIT(ETCH_FXB_EQ))){
  s->ring_mix_smooth=0;s->ring_phase=0;
 }else in=etch_fx_ring_eq(s,in,cfg,bypass_mask);
 in=fx_v102_process(s,in,cfg,0u);
 int32_t chorus_delta=0,chorus_right=0;
 /* V1.40: disabled sends MUST NOT run the expensive wet DSP merely
 * because their bypass flag is clear. The old OR condition rendered three
 * delay engines at MIX=0 whenever DRIVE/FUZZ broke the dry fast path.
 * The slewed mix is authoritative, so switch-off tails still finish. */
 if(chorus_render){
 uint32_t phase=s->chorus_phase;
 s->chorus_phase=(phase+s->rate_step)&65535u;
 uint32_t tri=phase<32768u?phase:65535u-phase;
 /* V1.22: opposite L/R modulation, plus interpolated sub-sample reads.
  * The same 2048-sample ring is retained; no second stereo chorus buffer.
  * 264 samples is 6ms; depth adds up to ~21ms. */
 uint32_t shift_l=((uint32_t)tri*s->chorus_span)>>7;
 uint32_t shift_r=((uint32_t)(32767u-tri)*s->chorus_span)>>7;
 int32_t wetc=etch_fx_chorus_read(s,264u*256u+shift_l);
 int32_t wetc_r=etch_fx_chorus_read(s,281u*256u+shift_r);
 s->chorus[s->chorus_pos]=in;
 s->chorus_pos=(s->chorus_pos+1u)&(ETCH_CHORUS_SIZE-1u);
 chorus_delta=((wetc-in)*(int32_t)chorus_render)/65536;
 chorus_right=((wetc_r-in)*(int32_t)chorus_render)/65536;
 }else{
  /* Cheap zero-fill of a single ring entry avoids stale echoes on restore. */
  s->chorus[s->chorus_pos]=0;
  s->chorus_pos=(s->chorus_pos+1u)&(ETCH_CHORUS_SIZE-1u);
 }
 /* Echo spans about 9..360ms @ 44.1kHz. Never read the write head. */
 int32_t delay_delta=0,delay_right=0;
 int32_t fxin=etch_fx_route(cfg)?in:clip16((int32_t)in+chorus_delta);
 if(delay_render){ /* active or fading delay only */
 uint32_t didx=(s->delay_pos-s->delay_samples)&(ETCH_DELAY_SIZE-1u);
 int32_t wetd=s->delay[didx];
 int32_t wetd_right=s->delay[(didx-97u)&(ETCH_DELAY_SIZE-1u)];
 /* Equivalent 32-bit staged Q15: no software 64-bit divide in echo feedback. */
 s->delay[s->delay_pos]=clip16(fxin+(((wetd*(int32_t)feedback)>>15)*27)/32);
 s->delay_pos=(s->delay_pos+1u)&(ETCH_DELAY_SIZE-1u);
 delay_delta=(wetd*(int32_t)delay_render)/65536;
 delay_right=(wetd_right*(int32_t)delay_render)/65536;
 }else{
  s->delay[s->delay_pos]=0;
  s->delay_pos=(s->delay_pos+1u)&(ETCH_DELAY_SIZE-1u);
 }
 int32_t output=0,output_right=0;
 if(reverb_target || s->reverb_mix_smooth){ /* active or fading reverb only */
 int32_t input_r=etch_fx_route(cfg)?in:clip16(fxin+delay_delta);
 int32_t dry_left=etch_fx_route(cfg)?(int32_t)in+chorus_delta+delay_delta:input_r;
 int32_t dry_right=(int32_t)in+chorus_right+delay_right;
 int32_t wet_left=0,wet_right=0;
 switch(etch_fx_reverb_type(cfg)){
  case 0:ru_reverb_spring(s,input_r,decay,&wet_left,&wet_right);break;
  case 2:ru_reverb_room(s,input_r,decay,&wet_left,&wet_right);break;
  default:ru_reverb_hall(s,input_r,decay,&wet_left,&wet_right);break;
 }
 /* Shared dry/wet interpolation remains true wet-only at MIX=100. */
 s->reverb_mix_smooth=etch_fx_slew(s->reverb_mix_smooth,reverb_target);
 uint16_t space=(uint16_t)s->reverb_mix_smooth;
 output=ru_fx_reverb_lerp(dry_left,wet_left,space);
 output_right=ru_fx_reverb_lerp(dry_right,wet_right,space);
 }else{
  s->reverb[s->reverb_pos]=0;
  s->reverb_pos=(s->reverb_pos+1u)&(ETCH_REVERB_SIZE-1u);
  s->reverb_mix_smooth=0;
  output=etch_fx_route(cfg)?(int32_t)in+chorus_delta+delay_delta:clip16(fxin+delay_delta);
  output_right=(int32_t)in+chorus_right+delay_right;
 }
 /* Reverse delay: alternate recording and reading two disjoint blocks.
  * Entire last block is read backwards while current block is filled.
  * TIME maps 1024..8192 samples (23..186ms at 44.1k).
  * Playback/capture boundaries are guarded by a 128-sample triangular fade.
  * RATE changes only at a new capture block (no mid-block index tear). */
 if(!(bypass_mask&ETCH_FXB_BIT(ETCH_FXB_REVERSE))){
 s->reverse_mix_smooth=etch_fx_slew(s->reverse_mix_smooth,qclamp(cfg->reverse_mix));
 if(!s->reverse_len)s->reverse_len=1024u;
 uint16_t n=s->reverse_len;
 int32_t reverse_wet=0;
 if(s->reverse_mix_smooth && s->reverse_ready){
  uint32_t prev=(uint32_t)(s->reverse_bank^1u)*1792u;
  uint16_t idx=(uint16_t)(n-1u-s->reverse_pos);
  int32_t now=reverse_line[prev+idx];
  int32_t next=reverse_line[prev+(idx?idx-1u:n-1u)];
  int32_t wet=(now*(int32_t)(4u-s->reverse_subphase)+next*s->reverse_subphase)/4;
  uint32_t remaining=(uint32_t)n>(uint32_t)s->reverse_pos?((uint32_t)n-(uint32_t)s->reverse_pos):0u;
  uint32_t edge=s->reverse_pos<128u?(uint32_t)s->reverse_pos:(remaining<128u?remaining:128u);
  wet=(int32_t)(((int64_t)wet*edge)/128u);
  uint32_t alpha=1024u+(uint32_t)qclamp(cfg->reverse_tone)*31743u/32767u;
  s->reverse_tone_state+=(int32_t)(((int64_t)(wet-s->reverse_tone_state)*alpha)/32768);
  reverse_wet=s->reverse_tone_state;
 }
 uint32_t current=(uint32_t)s->reverse_bank*1792u;
 int32_t fb=(int32_t)(((int64_t)reverse_wet*qclamp(cfg->reverse_feed)*19660u)/(32767u*32767u));
 /* Four-sample averaged capture at 11.025kHz extends the maximum reverse
  * length without additional SRAM. Fractional playback between captured
  * samples avoids zipper artifacts; TONE shapes the decimated bandwidth. */
 s->reverse_accum+=output;
 if(++s->reverse_subphase>=4u){
  reverse_line[current+s->reverse_pos]=clip16(s->reverse_accum/4+fb);
  s->reverse_accum=0;s->reverse_subphase=0;
  if(++s->reverse_pos>=n){
   s->reverse_bank^=1u;s->reverse_pos=0;s->reverse_ready=1;
   /* Only change capture length at a completed block boundary. */
   uint16_t desired=(uint16_t)(512u+((uint32_t)qclamp(cfg->reverse_time)*1280u)/32767u);
   if(desired!=s->reverse_len){s->reverse_len=desired;s->reverse_ready=0;s->reverse_tone_state=0;}
  }
 }
 if(s->reverse_mix_smooth){
  output=ru_fx_delta_mix(output,reverse_wet,s->reverse_mix_smooth);
  output_right=ru_fx_delta_mix(output_right,reverse_wet,s->reverse_mix_smooth);
 }
 }else{
  s->reverse_mix_smooth=0;s->reverse_ready=0;s->reverse_accum=0;
 }
 /* Rotary: complementary L/R spinning horn tremolo plus softened drive.
  * Global, bounded, mono-compatible by averaging the stereo return. */
 s->rotary_mix_smooth=etch_fx_slew(s->rotary_mix_smooth,qclamp(cfg->rotary_mix));
 if(s->rotary_mix_smooth){
  uint32_t tri=fx_triangle(s->rotary_phase);
  int32_t lfo=(int32_t)tri-32767;
  uint32_t depth=qclamp(cfg->rotary_depth);
  int32_t leftgain=32767+(int32_t)(((int64_t)lfo*depth*3)/(32767*4));
  int32_t rightgain=32767-(int32_t)(((int64_t)lfo*depth*3)/(32767*4));
  int32_t drive=32767+(int32_t)qclamp(cfg->rotary_drive);
  int32_t l=clip16((int32_t)(((int64_t)output*leftgain*drive)/(32767LL*32767LL)));
  int32_t r=clip16((int32_t)(((int64_t)output_right*rightgain*drive)/(32767LL*32767LL)));
  output=ru_fx_delta_mix(output,l,s->rotary_mix_smooth);
  output_right=ru_fx_delta_mix(output_right,r,s->rotary_mix_smooth);
 }
 s->rotary_phase+=s->rotary_step;
 /* Protect all active FX (including compressor/folder/shape/fuzz/SAND),
  * not only time-based sends. Leave the full dry bypass sample-exact. */
 uint8_t wet_active=(uint8_t)(chorus_render||delay_render||s->reverb_mix_smooth||
       s->reverse_mix_smooth||s->rotary_mix_smooth||
       cfg->comp_ratio||cfg->ring_mix||
       cfg->trem_mix||cfg->phaser_mix||cfg->flanger_mix||
       cfg->decim_mix||cfg->tape_mix||cfg->reverse_mix||
       cfg->vibrato_mix||cfg->rotary_mix||cfg->wah_mix||
       (!eq_masked &&
         (cfg->eq_hpf||
#ifdef RUPTURE_BUILD
          (s->eq2_enabled && !rupture_eq2_is_unity(&s->eq2))||
#endif
          cfg->eq_low!=16384u||cfg->eq_mid!=16384u||cfg->eq_high!=16384u))||
       s->rupture_transient_mix||s->rupture_tilt_mix||
       s->rupture_fuzz_mix||s->rupture_sand_mix);
 *left=etch_fx_wet_safety(output,wet_active);
 *right=etch_fx_wet_safety(output_right,wet_active);
}
void etch_fx_process_stereo_ex(struct etch_native_fx *s,int16_t in,uint16_t chorus,uint16_t delay,uint16_t reverb,const struct etch_fx_settings *cfg,int16_t *left,int16_t *right){
 etch_fx_process_stereo_masked(s,in,chorus,delay,reverb,cfg,0u,left,right);
}
int16_t etch_fx_process_ex(struct etch_native_fx *s,int16_t in,uint16_t chorus,uint16_t delay,uint16_t reverb,const struct etch_fx_settings *cfg){
 int16_t left=in,right=in;
 etch_fx_process_stereo_ex(s,in,chorus,delay,reverb,cfg,&left,&right);
 return left;
}
int16_t etch_fx_process(struct etch_native_fx *s,int16_t in,uint16_t chorus,uint16_t delay,uint16_t reverb){
 return etch_fx_process_ex(s,in,chorus,delay,reverb,0);
}

#ifdef RUPTURE_BUILD
/* V1.94: real ordered chain with repeatable effect types. Time-based types
 * share their original fixed-size tank; calculate each tank's current wet
 * return once per PCM frame and reuse it at later occurrences. This avoids
 * corrupting ring timing or allocating extra PCM memory. Other types apply
 * their processing at each slot. No allocations/IO in the audio callback. */
void rupture_fx_process_ordered(struct etch_native_fx *s,int16_t sample,
 uint16_t chorus,uint16_t delay,uint16_t reverb,uint16_t mosfet,
 const struct etch_fx_settings *c,const struct rupture_fx_chain *chain,
 uint32_t bypass,int16_t *left,int16_t *right){
 if(!s||!c||!chain||!left||!right){if(left)*left=sample;if(right)*right=sample;return;}
 /* An untouched/new project starts with eight EMPTY insert slots. Reuse
  * V1.89's proven exact-dry/bounded ring-flush path rather than calculate
  * EQ/filter coefficients and scan eight inert slots at audio rate. Never
  * take the shortcut while a just-bypassed processor is still fading. */
 if(!chain->enabled && !c->ring_mix && !c->eq_hpf &&
    (c->eq_low==16384u||!c->eq_low) &&
    (c->eq_mid==16384u||!c->eq_mid) &&
    (c->eq_high==16384u||!c->eq_high) &&
    !c->trem_mix&&!c->vibrato_mix&&!c->phaser_mix&&
    !c->flanger_mix&&!c->tape_mix&&!c->decim_mix&&
    !c->reverse_mix&&!c->rotary_mix){
  uint8_t tail=0u;
  for(uint8_t slot=0;slot<RU_FX_SLOTS;slot++)if(s->rupture_chain_fade[slot]){tail=1u;break;}
  if(!tail){
   /* Cold/settled dry: no audio FX arithmetic, no eight-slot traversal.
    * Keep the neutral parametric EQ's IIR history warm, exactly as the
    * existing V1.89 dry path does. A formerly wet state must FIRST pass
    * through the old bounded tail/feedback-memory flush below. */
   if(!s->dry_was_wet&&!s->dry_flush_active&&!s->ring_mix_smooth&&
      !s->trem_mix_smooth&&!s->vibrato_mix_smooth&&
      !s->phaser_mix_smooth&&!s->flanger_mix_smooth&&!s->tape_mix_smooth&&
      !s->decim_mix_smooth&&!s->rupture_tilt_mix&&!s->rupture_fuzz_mix&&
      !s->rupture_sand_mix&&!rt_tilt_on&&!rt_fuzz_on&&!rt_sand_on&&
      s->comp_gain==32767&&s->eq_gain_smooth[0]==16384&&
      s->eq_gain_smooth[1]==16384&&s->eq_gain_smooth[2]==16384&&
      (!s->eq2_enabled||rupture_eq2_is_unity(&s->eq2))){
    if(s->eq2_enabled){
     for(uint8_t b=0u;b<4u;b++){
      s->eq2.x2[b]=s->eq2.x1[b];s->eq2.x1[b]=sample;
      s->eq2.y2[b]=s->eq2.y1[b];s->eq2.y1[b]=sample;
     }
    }
    *left=*right=sample;return;
   }
   static const struct etch_fx_settings inactive_fx={0};
   etch_fx_process_stereo_masked(s,sample,0u,0u,0u,&inactive_fx,bypass,left,right);
   return;
  }
 }
 int32_t l=sample,r=sample;
 /* Channel EQ is independent of the eight insert FX, just like the mixer. */
 int16_t eq=etch_fx_ring_eq(s,sample,c,bypass);
 /* The eight exposed effects have their own ordered implementations below.
  * Normally the hidden legacy modulation engines are all disabled, so skip
  * their costly per-sample DSP. Keep decoding old projects correctly if
  * they carry one of those less common ETCH modulation settings. */
 uint8_t hidden_processing=(uint8_t)(c->trem_mix||c->vibrato_mix||c->phaser_mix||
    c->flanger_mix||c->tape_mix||c->decim_mix||c->reverse_mix||c->rotary_mix||
    s->trem_mix_smooth||s->vibrato_mix_smooth||
    s->phaser_mix_smooth||s->flanger_mix_smooth||
    s->tape_mix_smooth||s->decim_mix_smooth||
    s->reverse_mix_smooth||s->rotary_mix_smooth);
 if(hidden_processing){
  /* In the ordered chain WAH and DISTORTION run ONLY where their slots
   * sit, never a second time in the old hidden FX processor. This cold
   * compatibility path is unused for normal RUPTURE patches. */
  struct etch_fx_settings hidden=*c;
  hidden.wah_mix=0u;hidden.folder_mix=0u;
  eq=fx_v102_process(s,eq,&hidden,1u);
 }
 l=r=eq;
 /* Even independent chains keep the dormant legacy caches warm. Older
  * hidden modulation can be switched on mid-note and reads these cached
  * coefficients BEFORE the refresh below. This preserves V1.94 PCM. */
 if(!s->applied_valid||s->applied.chorus_rate!=c->chorus_rate||
    s->applied.chorus_depth!=c->chorus_depth||s->applied.delay_time!=c->delay_time||
    s->applied.reverb_size!=c->reverb_size||s->applied.reverb_decay!=c->reverb_decay||
    s->applied.routing!=c->routing||s->applied.comp_ratio!=c->comp_ratio||
    s->applied.comp_attack!=c->comp_attack||s->applied.comp_release!=c->comp_release||
    s->applied.comp_threshold!=c->comp_threshold||
    s->applied.trem_rate!=c->trem_rate||s->applied.phaser_rate!=c->phaser_rate||
    s->applied.flanger_rate!=c->flanger_rate||s->applied.vibrato_rate!=c->vibrato_rate||
    s->applied.rotary_rate!=c->rotary_rate||s->applied.tape_wow!=c->tape_wow||
    s->applied.tape_flutter!=c->tape_flutter){
  s->applied=*c;s->applied_valid=1u;
  s->rate_step=(uint16_t)(1u+(uint32_t)qclamp(c->chorus_rate)*9u/32768u);
  s->chorus_span=(uint16_t)((uint32_t)qclamp(c->chorus_depth)*924u/32767u);
  s->delay_samples=(uint16_t)(384u+(uint32_t)qclamp(c->delay_time)*15488u/32767u);
  s->trem_step=14609u+(uint32_t)qclamp(c->trem_rate)*1450000u/32767u;
  s->phaser_step=2922u+(uint32_t)qclamp(c->phaser_rate)*980000u/32767u;
  s->flanger_step=2922u+(uint32_t)qclamp(c->flanger_rate)*980000u/32767u;
  s->vibrato_step=19479u+(uint32_t)qclamp(c->vibrato_rate)*856020u/32767u;
  s->rotary_step=19479u+(uint32_t)qclamp(c->rotary_rate)*681520u/32767u;
  s->tape_wow_step=12659u+(uint32_t)qclamp(c->tape_wow)*113700u/32767u;
  s->tape_flutter_step=292153u+(uint32_t)qclamp(c->tape_flutter)*876457u/32767u;
  s->comp_attack_step=(uint8_t)(2u+(32767u-qclamp(c->comp_attack))/512u);
  s->comp_release_step=(uint8_t)(1u+(32767u-qclamp(c->comp_release))/2048u);
  s->comp_threshold_cached=(uint16_t)(2000u+(32767u-qclamp(c->comp_threshold))*25000u/32767u);
  s->comp_ratio_cached=(uint8_t)(1u+qclamp(c->comp_ratio)*11u/32767u);
  uint32_t span=qclamp(c->reverb_size);
  switch(etch_fx_reverb_type(c)){
   case 0u:s->tap1=(uint16_t)(641u+span*1398u/32767u);
    s->tap2=(uint16_t)(803u+span*1254u/32767u);
    s->reverb_spring_feedback=(uint16_t)(18500u+(uint32_t)qclamp(c->reverb_decay)*10200u/32767u);break;
   case 2u:s->tap1=(uint16_t)(241u+span*486u/32767u);
    s->tap2=(uint16_t)(283u+span*598u/32767u);
    s->tap3=(uint16_t)(377u+span*636u/32767u);
    s->reverb_room_line4=(uint16_t)(489u+span*986u/32767u);
    s->reverb_room_feedback=(uint16_t)(20000u+(uint32_t)qclamp(c->reverb_decay)*10000u/32767u);break;
   default:s->tap1=(uint16_t)(521u+span*727u/32767u);
    s->tap2=(uint16_t)(1471u+span*1201u/32767u);
    s->tap3=(uint16_t)(2897u+span*1133u/32767u);break;
  }
  if(etch_fx_reverb_type(c)!=1u){
   uint16_t lengths[4]={s->tap1,s->tap2,s->tap3,s->reverb_room_line4};
   uint8_t count=etch_fx_reverb_type(c)==0u?2u:4u;
   for(uint8_t k=0u;k<count;k++)if(s->reverb_line_cursor[k]>=lengths[k])s->reverb_line_cursor[k]=0u;
  }
  s->ru_reverb_last_type=0xffu; /* independent insert will refresh geometry */
 }
 uint8_t wet_active=0u;
 /* Local returns, not new audio buffers. Reverb, chorus and delay each own
  * ONE global PCM ring. Multiple slots can reuse that frame's return; each
  * placement still applies its wet/dry mix in the requested signal order.
  * We deliberately do not feed a ring twice per PCM frame, which would
  * halve its effective delay time and cause pitch/phase artefacts. */
 uint16_t shared_fx_done=0u;
 int32_t combined_delay_feedback=0,delay_input=0;
 uint8_t combined_delay_count=0u;
 int32_t chorus_wl=0,chorus_wr=0,delay_wl=0,delay_wr=0,reverb_wl=0,reverb_wr=0;
 int32_t wah_wet=0,tilt_delta=0,fuzz_delta=0,sand_delta=0;
 for(uint8_t slot=0;slot<RU_FX_SLOTS;slot++){
  /* Common original chain order can reuse the V1.89 four-effect fastpath.
   * Only fuse when its exact first four processors are all enabled and
   * fully faded-in, no hidden ETCH engines run, and the signal is still
   * mono. Distinct order, bypass or stereo input uses the real per-slot
   * processors below. No DSP is skipped or reordered by this shortcut. */
  if(slot==0u && l==r && !hidden_processing && !chain->independent &&
     (chain->enabled&15u)==15u &&
     chain->type[0]==RU_FX_COMP && chain->type[1]==RU_FX_TILT &&
     chain->type[2]==RU_FX_FUZZ && chain->type[3]==RU_FX_SAND &&
     s->rupture_chain_fade[0]==32767 &&
     s->rupture_chain_fade[1]==32767 &&
     s->rupture_chain_fade[2]==32767 &&
     s->rupture_chain_fade[3]==32767){
    l=r=fx_v102_process(s,(int16_t)l,c,2u);
    wet_active=1u;slot=3u;continue;
  }
  uint8_t type=chain->type[slot];
  if(type==RU_FX_EMPTY)continue;
  const uint16_t *p=chain->parameter[slot];
  const uint8_t separated=chain->independent;
  int32_t target=(chain->enabled&(1u<<slot))?32767:0;
  int32_t fade=s->rupture_chain_fade[slot]=etch_fx_slew(s->rupture_chain_fade[slot],target);
  if(!fade)continue;
  /* Zero-wet custom FX must be genuinely cheap when inserted but silent. */
  if((type==RU_FX_WAH&&!(separated?p[3]:c->wah_mix)&&
      !(separated?s->ru_slot_small[slot].wah_mix:s->wah_mix_smooth))||
     (type==RU_FX_DISTORTION&&!(separated?p[3]:c->folder_mix)&&
      !(separated?s->ru_slot_small[slot].distortion_mix:s->folder_mix_smooth)))continue;
  wet_active=1u;
  int32_t dry_l=l,dry_r=r;
  int16_t in=clip16((l+r)/2);
  switch(type){
   case RU_FX_DISTORTION:{
    /* Hard-edged, asymmetrical clipping with one-pole tone and direct mix.
     * Distinct from the soft MOSFET transfer and with no division by a
     * variable, heap allocation, or extra audio ring buffer. */
    uint32_t gain_q8=256u+((uint32_t)qclamp(separated?p[0]:c->folder_fold)*15u>>7u);
    int32_t bias=((int32_t)qclamp(separated?p[1]:c->folder_symmetry)-16384)*3/8;
    int32_t driven=((int32_t)in*(int32_t)gain_q8>>8u)+bias;
    int32_t shaped=(int32_t)clip16(driven)-(int32_t)clip16(bias);
    uint32_t alpha=1024u+((uint32_t)qclamp(separated?p[2]:c->folder_tone)*31743u>>15u);
    /* Half-scale Q14 avoids costly 64-bit multiplies on the FM1 core;
     * state/PCM differences remain bounded to the signed 32-bit range. */
    int32_t tone_state=separated?s->ru_slot_small[slot].distortion_tone:s->folder_tone_state;
    int32_t tone=tone_state+
      ((((shaped-tone_state)>>1)*(int32_t)alpha)>>14);
    if(separated || !(shared_fx_done&256u)){
     shared_fx_done|=256u;
     if(separated){
      s->ru_slot_small[slot].distortion_tone=tone;
      s->ru_slot_small[slot].distortion_mix=etch_fx_slew(s->ru_slot_small[slot].distortion_mix,qclamp(p[3]));
     }else{
      s->folder_tone_state=tone;
      s->folder_mix_smooth=etch_fx_slew(s->folder_mix_smooth,qclamp(c->folder_mix));
     }
    }
    int32_t delta=(((tone-in)>>1)*(separated?s->ru_slot_small[slot].distortion_mix:s->folder_mix_smooth))>>14;
    l=clip16(l+delta);r=clip16(r+delta);
    break;
   }
   case RU_FX_WAH:{
    /* The envelope/filter must not advance eight times per real PCM sample
     * when eight WAH slots are used: that would shift its cutoff response.
     * Reuse the same frame's wet output, but apply it at each chain position. */
    if(separated || !(shared_fx_done&8u)){
     shared_fx_done|=8u;
     int32_t mag=in<0?-(int32_t)in:in;
     uint32_t attack;
     int32_t resonance;
     if(separated){
      /* Independent WAH slots carry their own tiny parameter cache. Knob
       * edits are very rare compared with audio samples. Refresh without
       * interpolation changes, including after moving/replacing a slot. */
      if(!s->ru_slot_small[slot].wah_cache_valid||
         s->ru_slot_small[slot].wah_attack_param!=p[1]||
         s->ru_slot_small[slot].wah_reso_param!=p[2]){
       s->ru_slot_small[slot].wah_attack_param=p[1];
       s->ru_slot_small[slot].wah_reso_param=p[2];
       s->ru_slot_small[slot].wah_attack_cached=(uint16_t)(4u+((uint32_t)qclamp(p[1])*1000u>>15u));
       s->ru_slot_small[slot].wah_resonance_cached=(uint16_t)(16384u+((uint32_t)qclamp(p[2])*16383u>>15u));
       s->ru_slot_small[slot].wah_cache_valid=1u;
      }
      attack=s->ru_slot_small[slot].wah_attack_cached;
      resonance=s->ru_slot_small[slot].wah_resonance_cached;
     }else{
      attack=4u+((uint32_t)qclamp(c->wah_attack)*1000u>>15u);
      resonance=16384+(int32_t)((uint32_t)qclamp(c->wah_reso)*16383u>>15u);
     }
     int32_t *env=separated?&s->ru_slot_small[slot].wah_env:&s->wah_env;
     int32_t *lp=separated?&s->ru_slot_small[slot].wah_lp:&s->wah_lp;
     int32_t *hp=separated?&s->ru_slot_small[slot].wah_hp:&s->wah_hp;
     int32_t speed=mag>*env?(int32_t)attack:16;
     *env+=((mag-*env)*speed)>>15;
     int32_t response=(*env*(int32_t)qclamp(separated?p[0]:c->wah_sens))>>15;
     if(response>24000)response=24000;
     uint32_t alpha=300u+(((uint32_t)response*5u)>>3u);
     *lp+=((in-*lp)*(int32_t)alpha)>>15;
     *hp+=((*lp-*hp)*(int32_t)((alpha>>2u)+100u))>>15;
     int32_t band=*lp-*hp;
     wah_wet=clip16((band*resonance)>>14);
     if(separated)s->ru_slot_small[slot].wah_mix=etch_fx_slew(s->ru_slot_small[slot].wah_mix,qclamp(p[3]));
     else s->wah_mix_smooth=etch_fx_slew(s->wah_mix_smooth,qclamp(c->wah_mix));
    }
    int32_t delta=((wah_wet-in)*(separated?s->ru_slot_small[slot].wah_mix:s->wah_mix_smooth))>>15;
    l=clip16(l+delta);r=clip16(r+delta);
    break;
   }
   case RU_FX_MOSFET:{
    /* Post-mix MOSFET shaping preserves real chain ordering and stereo.
     * Previous code used two variable divisions per PCM frame. The ROM
     * transfer avoids software division on the pi32v2 audio interrupt. */
    if(separated?p[0]:mosfet){
     uint16_t gain_q8=(uint16_t)(256u+(((uint32_t)qclamp(separated?p[0]:mosfet)*3u)>>7));
     l=ru_mosfet_shaper(l,gain_q8);
     r=ru_mosfet_shaper(r,gain_q8);
    }
    break;
   }
   case RU_FX_COMP:{
    if(separated){
     /* Each compressor gets its own envelope and gain reduction. The
      * controls are simple scalar values; no large history buffers. */
     int32_t *env=&s->ru_slot_small[slot].comp_env;
     int32_t *gain=&s->ru_slot_small[slot].comp_gain;
     int32_t *target_gain=&s->ru_slot_small[slot].comp_target;
     uint8_t *tick=&s->ru_slot_small[slot].comp_tick;
     if(!*gain){*gain=32767;*target_gain=32767;}
     int32_t mag=in<0?-(int32_t)in:in;
     uint8_t attack_step=(uint8_t)(2u+(32767u-qclamp(p[2]))/512u);
     uint8_t release_step=(uint8_t)(1u+(32767u-qclamp(p[3]))/2048u);
     *env+=((mag-*env)*(mag>*env?attack_step:release_step))/32768;
     if(++*tick>=16u){
      *tick=0u;*target_gain=32767;
      uint16_t threshold=(uint16_t)(2000u+(32767u-qclamp(p[0]))*25000u/32767u);
      uint8_t ratio=(uint8_t)(1u+qclamp(p[1])*11u/32767u);
      if(*env>threshold&&p[1]){
       int32_t limited=threshold+(*env-threshold)/ratio;
       *target_gain=limited*32767/(*env);
      }
     }
     *gain=etch_fx_slew(*gain,*target_gain);
     l=(l*(*gain))/32767;r=(r*(*gain))/32767;
    }else{
     if(!(shared_fx_done&16u)){
      shared_fx_done|=16u;
      int32_t mag=in<0?-(int32_t)in:in;
      s->comp_env+=((mag-s->comp_env)*(mag>s->comp_env?s->comp_attack_step:s->comp_release_step))/32768;
      if(++s->comp_tick>=16u){s->comp_tick=0u;s->comp_target=32767;
       if(s->comp_env>s->comp_threshold_cached&&c->comp_ratio){
        int32_t limited=s->comp_threshold_cached+(s->comp_env-s->comp_threshold_cached)/s->comp_ratio_cached;
        s->comp_target=limited*32767/s->comp_env;
       }
      }
      s->comp_gain=etch_fx_slew(s->comp_gain,s->comp_target);
     }
     l=(l*s->comp_gain)/32767;r=(r*s->comp_gain)/32767;
    }
    break;
   }
   /* These three mono-state effects process the mid signal. Apply their
    * transfer delta to both channels so a preceding stereo echo/reverb does
    * not suddenly collapse to mono when a saturation/EQ slot follows it. */
   case RU_FX_TILT:{
    if(separated){
     int32_t target=(uint32_t)p[2]*32767u/100u;
     int32_t *mix=&s->ru_slot_tilt[slot].mix;
     *mix=etch_fx_slew(*mix,target);
     if(!*mix)s->ru_slot_tilt[slot].lp=0;
     else{
      int32_t alpha=250+(uint32_t)p[1]*p[1]*2u;
      int32_t *lp=&s->ru_slot_tilt[slot].lp;
      *lp+=((in-*lp)*alpha)/32768;
      int32_t bass=*lp,treble=in-bass;
      int32_t wet=clip16(in+(clip16(treble-bass)*((int32_t)p[0]-50))/60);
      int32_t output=(int32_t)p[3];
      int32_t gain=output<=50?8192+output*(32767-8192)/50:
       32767+(output-50)*(65534-32767)/50;
      wet=clip16((wet*gain)/32767);
      tilt_delta=rupture_q15_mix(in,wet,(uint16_t)*mix)-in;
     }
    }else if(!(shared_fx_done&32u)){
     shared_fx_done|=32u;tilt_delta=rupture_tilt_process(s,in)-in;
    }
    l=clip16(l+tilt_delta);r=clip16(r+tilt_delta);break;}
   case RU_FX_FUZZ:{
    if(separated){
     /* Isolated octave fuzz envelope, DC tracker, antialias and makeup.
      * Same bounded transfer curve and reciprocal table as the legacy FX. */
     int32_t *mix=&s->ru_slot_fuzz[slot].mix;
     *mix=etch_fx_slew(*mix,(uint32_t)p[3]*32767u/100u);
     if(!*mix){
      s->ru_slot_fuzz[slot].dc=s->ru_slot_fuzz[slot].tone=0;
      s->ru_slot_fuzz[slot].input_lp=0;
      s->ru_slot_fuzz[slot].in_env=s->ru_slot_fuzz[slot].wet_env=0;
      s->ru_slot_fuzz[slot].makeup=s->ru_slot_fuzz[slot].makeup_target=32767;
      s->ru_slot_fuzz[slot].drive_slew=32767+(int32_t)p[0]*1000;
      s->ru_slot_fuzz[slot].anti_slew=32767-(int32_t)p[0]*120;
      s->ru_slot_fuzz[slot].oct_slew=((int32_t)p[1]*32767+50)/100;
      s->ru_slot_fuzz[slot].tone_slew=600+(int32_t)p[2]*320;
      s->ru_slot_fuzz[slot].tick=0;
     }else{
      int32_t *drive=&s->ru_slot_fuzz[slot].drive_slew;
      int32_t *anti=&s->ru_slot_fuzz[slot].anti_slew;
      int32_t *oct=&s->ru_slot_fuzz[slot].oct_slew;
      int32_t *tone_slew=&s->ru_slot_fuzz[slot].tone_slew;
      *drive=etch_fx_slew(*drive,32767+(int32_t)p[0]*1000);
      *anti=etch_fx_slew(*anti,32767-(int32_t)p[0]*120);
      *oct=etch_fx_slew(*oct,((int32_t)p[1]*32767+50)/100);
      *tone_slew=etch_fx_slew(*tone_slew,600+(int32_t)p[2]*320);
      int32_t *input_lp=&s->ru_slot_fuzz[slot].input_lp;
      *input_lp+=((in-*input_lp)*(*anti))/32768;
      int32_t driven=clip16(((*input_lp)*((*drive)>>2))/8192);
      int32_t rect=rupture_fuzz_soft_rect(driven);
      int32_t *dc=&s->ru_slot_fuzz[slot].dc;
      *dc+=(rect-*dc)/128;
      int32_t octave=clip16((rect-*dc)*2);
      int32_t mixoct=driven+((octave-driven)*(*oct))/32767;
      int32_t *tone=&s->ru_slot_fuzz[slot].tone;
      *tone+=((mixoct-*tone)*(*tone_slew))/32768;
      int32_t wet=clip16(*tone);
      int32_t in_mag=in<0?-(int32_t)in:in,wet_mag=wet<0?-wet:wet;
      int32_t *in_env=&s->ru_slot_fuzz[slot].in_env;
      int32_t *wet_env=&s->ru_slot_fuzz[slot].wet_env;
      *in_env+=(in_mag-*in_env)/256;
      *wet_env+=(wet_mag-*wet_env)/256;
      int32_t *makeup_target=&s->ru_slot_fuzz[slot].makeup_target;
      int32_t *makeup=&s->ru_slot_fuzz[slot].makeup;
      if(++s->ru_slot_fuzz[slot].tick>=32u){
       s->ru_slot_fuzz[slot].tick=0u;
       *makeup_target=rupture_fuzz_ratio_q15(*in_env+256,*wet_env+256);
      }
      *makeup+=(*makeup_target-*makeup)/256;
      if(*makeup!=*makeup_target)*makeup+=*makeup_target>*makeup?1:-1;
      wet=(wet*(*makeup))/32767;
      fuzz_delta=rupture_q15_mix(in,wet,(uint16_t)*mix)-in;
     }
    }else if(!(shared_fx_done&64u)){
     shared_fx_done|=64u;fuzz_delta=rupture_fuzz_process(s,in)-in;
    }
    l=clip16(l+fuzz_delta);r=clip16(r+fuzz_delta);break;}
   case RU_FX_SAND:{
    if(separated){
     int32_t *mix=&s->ru_slot_sand[slot].mix;
     *mix=etch_fx_slew(*mix,(uint32_t)p[3]*32767u/100u);
     if(!*mix){
      s->ru_slot_sand[slot].pole1=s->ru_slot_sand[slot].pole2=s->ru_slot_sand[slot].body=0;
      s->ru_slot_sand[slot].soft_slew=s->ru_slot_sand[slot].warm_slew=s->ru_slot_sand[slot].smooth_slew=0;
     }else{
      int32_t *smooth=&s->ru_slot_sand[slot].smooth_slew;
      *smooth=etch_fx_slew(*smooth,(int32_t)p[0]*256);
      int32_t a=26000-(uint32_t)(*smooth>>8)*22000u/100u;
      int32_t *pole1=&s->ru_slot_sand[slot].pole1;
      int32_t *pole2=&s->ru_slot_sand[slot].pole2;
      int32_t *body=&s->ru_slot_sand[slot].body;
      *pole1+=((in-*pole1)*a)>>15;
      *pole2+=((*pole1-*pole2)*a)>>15;
      *body+=((in-*body)*650)>>15;
      int32_t *soft=&s->ru_slot_sand[slot].soft_slew;
      int32_t *warm=&s->ru_slot_sand[slot].warm_slew;
      *soft=etch_fx_slew(*soft,(int32_t)p[1]*256);
      *warm=etch_fx_slew(*warm,(int32_t)p[2]*256);
      int32_t wet=*pole2;
      int32_t mag=wet<0?-wet:wet;
      int32_t reduction=((*soft>>8)*(3000+(mag>>1)))/100;
      int32_t factor=32767-reduction;
      if(factor<12000)factor=12000;
      wet=(wet*factor)/32767;
      wet+=(*body*(*warm>>8)*3)/500;
      wet=clip16(wet);
      sand_delta=rupture_q15_mix(in,wet,(uint16_t)*mix)-in;
     }
    }else if(!(shared_fx_done&128u)){
     shared_fx_done|=128u;sand_delta=rupture_sandpaper_process(s,in)-in;
    }
    l=clip16(l+sand_delta);r=clip16(r+sand_delta);break;}
   case RU_FX_CHORUS:{
    if(separated){
     int32_t *mix=&s->ru_slot_chorus_mix[slot];
     *mix=etch_fx_slew(*mix,qclamp(p[3]));
     if(!s->ru_slot_chorus_step[slot]||s->ru_slot_chorus_rate[slot]!=p[0]||s->ru_slot_chorus_depth[slot]!=p[1]){
      s->ru_slot_chorus_rate[slot]=p[0];s->ru_slot_chorus_depth[slot]=p[1];
      s->ru_slot_chorus_step[slot]=(uint16_t)(1u+(uint32_t)qclamp(p[0])*9u/32768u);
      s->ru_slot_chorus_span[slot]=(uint16_t)((uint32_t)qclamp(p[1])*924u/32767u);
     }
     uint32_t phase=s->ru_slot_chorus_phase[slot];
     s->ru_slot_chorus_phase[slot]=(phase+s->ru_slot_chorus_step[slot])&65535u;
     uint32_t tri=phase<32768u?phase:65535u-phase;
     uint32_t shift_l=(tri*s->ru_slot_chorus_span[slot])>>7u;
     uint32_t shift_r=((32767u-tri)*s->ru_slot_chorus_span[slot])>>7u;
     /* Every chorus slot uses independently modulated taps on the existing
      * single input history, which is written exactly once per PCM frame. */
     uint32_t pos=(s->chorus_pos-((shared_fx_done&1u)?1u:0u))&(ETCH_CHORUS_SIZE-1u);
     chorus_wl=etch_fx_chorus_read_at(s,pos,264u*256u+shift_l);
     chorus_wr=etch_fx_chorus_read_at(s,pos,281u*256u+shift_r);
     if(!(shared_fx_done&1u)){
      shared_fx_done|=1u;
      s->chorus[s->chorus_pos]=in;s->chorus_pos=(s->chorus_pos+1u)&(ETCH_CHORUS_SIZE-1u);
     }
     l=clip16(l+((chorus_wl-in)*(*mix))/32767);
     r=clip16(r+((chorus_wr-in)*(*mix))/32767);
    }else{
     uint16_t wet=qclamp(chorus);
     if(!(shared_fx_done&1u)){
      shared_fx_done|=1u;
      s->chorus_mix_smooth=etch_fx_slew(s->chorus_mix_smooth,wet);
      uint32_t phase=s->chorus_phase;
      s->chorus_phase=(phase+s->rate_step)&65535u;
      uint32_t tri=phase<32768u?phase:65535u-phase;
      uint32_t shift_l=(tri*s->chorus_span)>>7u;
      uint32_t shift_r=((32767u-tri)*s->chorus_span)>>7u;
      chorus_wl=etch_fx_chorus_read(s,264u*256u+shift_l);
      chorus_wr=etch_fx_chorus_read(s,281u*256u+shift_r);
      s->chorus[s->chorus_pos]=in;s->chorus_pos=(s->chorus_pos+1u)&(ETCH_CHORUS_SIZE-1u);
     }
     l=clip16(l+((chorus_wl-in)*s->chorus_mix_smooth)/32767);
     r=clip16(r+((chorus_wr-in)*s->chorus_mix_smooth)/32767);
    }
    break;
   }
   case RU_FX_DELAY:{
    if(separated){
     if(!s->ru_slot_delay_samples[slot]||s->ru_slot_delay_time[slot]!=p[0]){
      s->ru_slot_delay_time[slot]=p[0];
      s->ru_slot_delay_samples[slot]=(uint16_t)(384u+(uint32_t)qclamp(p[0])*15488u/32767u);
     }
     int32_t *mix=&s->ru_slot_delay_mix[slot];
     *mix=etch_fx_slew(*mix,qclamp(p[3]));
     uint32_t idx=(s->delay_pos-s->ru_slot_delay_samples[slot])&(ETCH_DELAY_SIZE-1u);
     delay_wl=s->delay[idx];delay_wr=s->delay[(idx-97u)&(ETCH_DELAY_SIZE-1u)];
     if(!(shared_fx_done&2u)){
      shared_fx_done|=2u;
      delay_input=in;
     }
     /* Each slot's FEEDBACK contributes to a single shared return. This
      * is not an independent echo tank, but a second FEEDBACK knob now
      * genuinely affects the sound without more PCM buffers. */
     combined_delay_feedback+=(delay_wl*(int32_t)qclamp(p[1]))>>15;
     combined_delay_count++;
     l=clip16(l+(delay_wl*(*mix))/32767);
     r=clip16(r+(delay_wr*(*mix))/32767);
     break;
    }
    if(!(shared_fx_done&2u)){
     shared_fx_done|=2u;
     s->delay_mix_smooth=etch_fx_slew(s->delay_mix_smooth,qclamp(delay));
     uint32_t idx=(s->delay_pos-s->delay_samples)&(ETCH_DELAY_SIZE-1u);
     delay_wl=s->delay[idx];delay_wr=s->delay[(idx-97u)&(ETCH_DELAY_SIZE-1u)];
     int32_t fb=(delay_wl*(int32_t)qclamp(c->delay_feedback))>>15;
     s->delay[s->delay_pos]=clip16(in+(fb*27)/32);
     s->delay_pos=(s->delay_pos+1u)&(ETCH_DELAY_SIZE-1u);
    }
    l=clip16(l+(delay_wl*s->delay_mix_smooth)/32767);
    r=clip16(r+(delay_wr*s->delay_mix_smooth)/32767);break;
   }
   case RU_FX_REVERB:{
    if(separated){
     /* Shared FDN tank is advanced only once. Per-slot wet level is
      * independent; truly independent decays would require more audio RAM. */
     if(!(shared_fx_done&4u)){
      shared_fx_done|=4u;
      uint16_t decay=qclamp(p[1]);
      uint8_t engine=(uint8_t)p[0];
      uint16_t span=qclamp(p[2]);
      if(s->ru_reverb_last_type!=engine||s->ru_reverb_last_size!=span||
         s->ru_reverb_last_decay!=decay){
       s->ru_reverb_last_type=engine;s->ru_reverb_last_size=span;
       s->ru_reverb_last_decay=decay;
       switch(engine){
        case 0u:s->tap1=(uint16_t)(641u+(uint32_t)span*1398u/32767u);
         s->tap2=(uint16_t)(803u+(uint32_t)span*1254u/32767u);
         s->reverb_spring_feedback=(uint16_t)(18500u+(uint32_t)decay*10200u/32767u);break;
        case 2u:s->tap1=(uint16_t)(241u+(uint32_t)span*486u/32767u);
         s->tap2=(uint16_t)(283u+(uint32_t)span*598u/32767u);
         s->tap3=(uint16_t)(377u+(uint32_t)span*636u/32767u);
         s->reverb_room_line4=(uint16_t)(489u+(uint32_t)span*986u/32767u);
         s->reverb_room_feedback=(uint16_t)(20000u+(uint32_t)decay*10000u/32767u);break;
        default:s->tap1=(uint16_t)(521u+(uint32_t)span*727u/32767u);
         s->tap2=(uint16_t)(1471u+(uint32_t)span*1201u/32767u);
         s->tap3=(uint16_t)(2897u+(uint32_t)span*1133u/32767u);break;
       }
       if(engine!=1u){
        uint16_t lengths[4]={s->tap1,s->tap2,s->tap3,s->reverb_room_line4};
        for(uint8_t k=0u;k<(engine==0u?2u:4u);k++)
         if(s->reverb_line_cursor[k]>=lengths[k])s->reverb_line_cursor[k]=0u;
       }
      }
      switch(engine){
       case 0u:ru_reverb_spring(s,in,decay,&reverb_wl,&reverb_wr);break;
       case 2u:ru_reverb_room(s,in,decay,&reverb_wl,&reverb_wr);break;
       default:ru_reverb_hall(s,in,decay,&reverb_wl,&reverb_wr);break;
      }
     }
     int32_t *mix=&s->ru_slot_reverb_mix[slot];
     *mix=etch_fx_slew(*mix,qclamp(p[3]));
     l=ru_fx_reverb_lerp(l,reverb_wl,(uint16_t)*mix);
     r=ru_fx_reverb_lerp(r,reverb_wr,(uint16_t)*mix);
     break;
    }
    if(!(shared_fx_done&4u)){
     shared_fx_done|=4u;
     uint16_t decay=qclamp(c->reverb_decay);
     switch(etch_fx_reverb_type(c)){
      case 0u:ru_reverb_spring(s,in,decay,&reverb_wl,&reverb_wr);break;
      case 2u:ru_reverb_room(s,in,decay,&reverb_wl,&reverb_wr);break;
      default:ru_reverb_hall(s,in,decay,&reverb_wl,&reverb_wr);break;
     }
     s->reverb_mix_smooth=etch_fx_slew(s->reverb_mix_smooth,qclamp(reverb));
    }
    l=ru_fx_reverb_lerp(l,reverb_wl,(uint16_t)s->reverb_mix_smooth);
    r=ru_fx_reverb_lerp(r,reverb_wr,(uint16_t)s->reverb_mix_smooth);break;
   }
   default:break;
  }
  if(fade!=32767){
   l=clip16(dry_l+(int32_t)(((int64_t)(l-dry_l)*fade)/32767));
   r=clip16(dry_r+(int32_t)(((int64_t)(r-dry_r)*fade)/32767));
  }
 }
 if(chain->independent&&combined_delay_count){
  /* Control count 1..8: Q9 reciprocal avoids a variable integer divide in
   * the audio IRQ. A single delay remains exactly sample-identical. */
  static const uint16_t reciprocal_q9[9]={0,512,256,171,128,102,85,73,64};
  int32_t feedback=(combined_delay_feedback*reciprocal_q9[combined_delay_count])>>9;
  s->delay[s->delay_pos]=clip16(delay_input+(feedback*27)/32);
  s->delay_pos=(s->delay_pos+1u)&(ETCH_DELAY_SIZE-1u);
 }
 *left=wet_active?etch_fx_wet_safety(l,1u):clip16(l);
 *right=wet_active?etch_fx_wet_safety(r,1u):clip16(r);
 if(wet_active)s->dry_was_wet=1u;
}
#endif
