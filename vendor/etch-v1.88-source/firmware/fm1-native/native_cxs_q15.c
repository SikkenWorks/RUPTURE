/* SPDX-License-Identifier: GPL-3.0-only */
#include "native_cxs_q15.h"
static uint32_t ru_rng(RuptureCXSQ15 *s){
 uint32_t x=s->seed;x^=x<<13;x^=x>>17;x^=x<<5;s->seed=x;return x;
}
static int32_t clamp32(int32_t x,int32_t a,int32_t b){return x<a?a:x>b?b:x;}
static void exchange(RuptureCXSQ15 *s){
 for(uint32_t i=0;i<8;i++){
  uint32_t j=(i+1u)&7u;
  int32_t noise=(int32_t)(ru_rng(s)&255u)-128;
  int32_t bias=((i&1u)?-1:1)*((int32_t)s->balance-50);
  int32_t restoring=((int32_t)s->durations[j]-(int32_t)s->durations[i])/8;
  int32_t desired=(noise+bias*2+restoring/8)*(int32_t)s->pressure/150u;
  int32_t prev=s->flux[i];
  /* Values are bounded after division, but the weighted numerator needs
   * 64-bit arithmetic to stay defined under extreme parameter sweeps. */
  int32_t flow=(int32_t)(((int64_t)prev*(int32_t)s->memory+
    (int64_t)desired*(100-(int32_t)s->memory))/100);
  flow=clamp32(flow,-512,512);
  flow=clamp32(flow,-((int32_t)s->durations[j]-512),(int32_t)s->durations[i]-512);
  s->flux[i]=(int16_t)flow;
  s->durations[i]=(uint16_t)((int32_t)s->durations[i]-flow);
  s->durations[j]=(uint16_t)((int32_t)s->durations[j]+flow);
 }
 uint32_t sum=0;
 int32_t dc=0;
 for(uint32_t i=0;i<8;i++){
  s->boundaries[i]=(uint16_t)sum;sum+=s->durations[i];
  /* Density controls relative segment height. No additional oscillator layer. */
  int32_t coarse=((i&3u)<2u?1:-1)*(100-(int32_t)s->density);
  int32_t fine=(i&1u?-1:1)*(int32_t)s->density;
  /* V1.55 PRESSURE: the former duration-only scaling was largely lost in
   * integer rounding and inaudible in sustained notes. Sculpt the eight
   * segment amplitudes as well, once per oscillator cycle (never per audio
   * sample). Zero keeps the classic segment levels; 1..100 provides a progressively stronger
   * harmonic change. No new buffers, voices or DSP loops. */
  static const int8_t pressure_profile[8]={-10,48,-36,-62,36,-48,64,-26};
  int32_t color=(int32_t)s->pressure*(int32_t)pressure_profile[i]*164/100;
  s->levels[i]=(int16_t)((coarse+fine)*164+color);
  dc+=(int32_t)s->durations[i]*(int32_t)s->levels[i]/32768;
 }
 s->dc=dc;s->cycles++;
}
void rupture_q15_init(RuptureCXSQ15 *s,uint32_t note_increment){
 unsigned char *b=(unsigned char*)s;for(uint32_t i=0;i<sizeof(*s);i++)b[i]=0;
 s->increment=note_increment;s->seed=0x76fe8217u;s->pressure=58;s->balance=50;s->memory=52;s->density=68;s->edge=50;
 for(uint32_t i=0;i<8;i++)s->durations[i]=4096;
 exchange(s);
}
void rupture_q15_params(RuptureCXSQ15 *s,uint8_t pressure,uint8_t balance,uint8_t memory,uint8_t density){
 s->pressure=(uint8_t)clamp32(pressure,0,100);s->balance=(uint8_t)clamp32(balance,0,100);
 s->memory=(uint8_t)clamp32(memory,0,100);s->density=(uint8_t)clamp32(density,0,100);
}
int16_t rupture_q15_sample(RuptureCXSQ15 *s){
 uint32_t previous=s->phase;s->phase+=s->increment;
 if(s->phase<previous)exchange(s);
 uint32_t t=s->phase>>17u;
 uint32_t segment=0;for(uint32_t i=1;i<8;i++)if(t>=s->boundaries[i])segment=i;
 int32_t raw=(int32_t)s->levels[segment]-s->dc;
 /* Preview anti-alias smoothing. Device release requires band-limited event
  * synthesis validation; this is intentionally NOT claimed final quality. */
 /* EDGE 50 exactly matches the original CXS 1/4 interpolation.
  * Lower values thicken, higher values reveal faster segment transients. */
 int32_t alpha=4096+((int32_t)s->edge-50)*67;
 if(alpha<512)alpha=512;
 if(alpha>8192)alpha=8192;
 s->smooth+=((raw-s->smooth)*alpha)/16384;
 return (int16_t)clamp32(s->smooth,-32767,32767);
}
int rupture_q15_valid(const RuptureCXSQ15 *s){
 uint32_t sum=0;for(uint32_t i=0;i<8;i++){if(s->durations[i]<512)return 0;sum+=s->durations[i];}
 return sum==32768u;
}
