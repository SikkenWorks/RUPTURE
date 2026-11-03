/* V1.66: test real native CXS oscillator + actual sustained SOUND SHAPE DSP.
 * No device/audio hardware. Neutral PCM compatibility, tone difference,
 * clipping bounds, note-on and held-note behaviour at bass and mid pitches. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_shape.h"
#define N 88200
static int16_t neutral[N],shaped[N];
typedef struct { uint64_t rms,delta,low_energy,high_energy;uint64_t hash;uint32_t peak; } Stat;
static void render(uint32_t inc,uint8_t weight,uint8_t punch,int16_t *out){
 RuptureCXSQ15 osc;rupture_q15_init(&osc,inc);
 int32_t lp=0;
 for(unsigned i=0;i<N;i++){
  rupture_q15_params(&osc,58,50,52,68);osc.edge=50;
  int32_t raw=rupture_q15_sample(&osc);
  int32_t val=rupture_shape_sample(&lp,raw,weight,0,punch,osc.cycles);
  assert(val>=-32767&&val<=32767);
  out[i]=(int16_t)val;
 }
}
static Stat stats(const int16_t *out,const int16_t *baseline){
 Stat r={0};int32_t lp=0;
 for(int i=0;i<N;i++){
  int32_t x=out[i];lp+=(x-lp)/32;
  if(i>N/2){
   int64_t d=x-baseline[i],high=x-lp;
   r.delta+=(uint64_t)(d*d);r.rms+=(uint64_t)((int64_t)x*x);
   r.low_energy+=(uint64_t)((int64_t)lp*lp);
   r.high_energy+=(uint64_t)(high*high);
   if((uint32_t)abs(x)>r.peak)r.peak=(uint32_t)abs(x);
   r.hash=(r.hash^(uint16_t)x)*1099511628211ull;
  }
 }
 return r;
}
int main(void){
 for(int f=0;f<2;f++){
  uint32_t inc=f?50500000u:12687301u;
  render(inc,0,0,neutral);
  Stat base=stats(neutral,neutral);
  assert(base.rms>100000000u);
  render(inc,100,0,shaped);Stat w=stats(shaped,neutral);
  printf("%s WEIGHT: power base=%llu weight=%llu delta=%llu; high/low ratios baseline=%llu/%llu weight=%llu/%llu\n",
     f?"MID":"BASS",(unsigned long long)base.rms,(unsigned long long)w.rms,(unsigned long long)w.delta,
     (unsigned long long)base.high_energy,(unsigned long long)base.low_energy,
     (unsigned long long)w.high_energy,(unsigned long long)w.low_energy);
  assert(w.delta>base.rms/80u); // >1.25% sustained signal difference
  // Lower high/low energy ratio demonstrates tone (not merely volume).
  assert((long double)w.high_energy/(long double)w.low_energy <
         (long double)base.high_energy/(long double)base.low_energy*0.88L);
  render(inc,0,100,shaped);Stat p=stats(shaped,neutral);
  printf("%s PUNCH: power base=%llu punch=%llu delta=%llu, ratio=%Lf vs %Lf\n",
     f?"MID":"BASS",(unsigned long long)base.rms,(unsigned long long)p.rms,(unsigned long long)p.delta,
     (long double)p.high_energy/(long double)p.low_energy,(long double)base.high_energy/(long double)base.low_energy);
  assert(p.delta>base.rms/90u); // sustained, after 1s (>8 oscillations)
  assert((long double)p.high_energy/(long double)p.low_energy >
         (long double)base.high_energy/(long double)base.low_energy*1.04L);
  // Controls below 1% cannot create sudden nonfinite/overflow output.
  RuptureCXSQ15 osc;rupture_q15_init(&osc,inc);int32_t lp=0;
  for(int i=0;i<150000;i++){
   rupture_q15_params(&osc,58,50,52,68);osc.edge=50;
   int32_t a=rupture_q15_sample(&osc);
   int32_t b=rupture_shape_sample(&lp,a,(uint8_t)(i%101),0,(uint8_t)((i*5)%101),osc.cycles);
   assert(b>=-32767&&b<=32767);
  }
 }
 // Isolated shaper neutral must be 1:1 PCM, including cycle 0.
 int32_t lp=0;for(int x=-32767;x<=32767;x+=331){assert(rupture_shape_sample(&lp,x,0,0,0,0)==x);}
 puts("PASS V1.66: WEIGHT and PUNCH sustained CXS tone, neutral PCM, stress safety");
 return 0;
}
