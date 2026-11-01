/* RUPTURE V1.47: SOUND SHAPING must alter a *sustained* SAW carrier,
 * without extra voices, floating-point DSP or out-of-range samples. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_bandwaves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_saw_tone.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_shape.h"

typedef struct {uint64_t hash; int64_t energy; int32_t peak, maxjump;} Result;
static Result render(unsigned sw,unsigned se,unsigned sb,unsigned sp){
 uint32_t phase=0u,inc=10712311u;int32_t low=0,previous=0;
 Result r={1469598103934665603ull,0,0,0};
 for(int n=0;n<150000;n++){
  phase+=inc;
  int32_t raw=ru_bandwave_read(RU_WAVE_SAW,phase,inc);
  int32_t x=rupture_saw_tone(&low,raw,58,50,52,68,(uint8_t)se,(uint8_t)sw,(uint8_t)sb,(uint8_t)sp);
  assert(x>=-32767 && x<=32767);
  /* Ignore initial attack: ensure each knob changes sustained 2-second audio. */
  if(n>=60000){
   int32_t step=x-previous;if(step<0)step=-step;if(step>r.maxjump)r.maxjump=step;
   int32_t magnitude=x<0?-x:x;if(magnitude>r.peak)r.peak=magnitude;
   r.energy+=(int64_t)x*x;
   r.hash=(r.hash^(uint16_t)x)*1099511628211ull;
  }
  previous=x;
 }
 return r;
}
int main(void){
 Result a=render(0,50,0,0);
 Result w=render(90,50,0,0),e=render(0,98,0,0),b=render(0,50,95,0),p=render(0,50,0,95);
 assert(a.energy>1000000 && a.maxjump>0);
 assert(w.hash!=a.hash && e.hash!=a.hash && b.hash!=a.hash && p.hash!=a.hash);
 /* Differences must be substantive, not a lone 1-LSB rounding artifact. */
 assert(w.energy*100ll>a.energy*103ll || w.energy*100ll<a.energy*97ll);
 assert(e.energy*100ll>a.energy*103ll || e.energy*100ll<a.energy*97ll);
 assert(b.energy*100ll>a.energy*103ll || b.energy*100ll<a.energy*97ll);
 assert(p.energy*100ll>a.energy*103ll || p.energy*100ll<a.energy*97ll);
 /* Neutral SAW controls (WEIGHT/BITE/PUNCH=0) match V1.46 algorithm
  * because these new stages are bypassed completely. */
 int32_t lp=0,body_low=0;uint32_t ph=0;
 for(int n=0;n<200000;n++){
  ph+=10712311u;int32_t raw=ru_bandwave_read(RU_WAVE_SAW,ph,10712311u);
  int32_t fresh=rupture_saw_tone(&lp,raw,58,50,52,68,50,0,0,0);
  int32_t old=rupture_shape_sample(&body_low,fresh,0,0,0,(uint32_t)n/193u);
  assert(fresh==old);
 }
 /* Stress overdrive and rapid edits; all calculations must stay defined. */
 lp=0;ph=0;
 for(int n=0;n<600000;n++){
  ph+=10712311u;int32_t raw=ru_bandwave_read(RU_WAVE_SAW,ph,10712311u);
  uint8_t x=(uint8_t)((n/17)&1?100:0);
  int32_t out=rupture_saw_tone(&lp,raw,100,100,100,100,x,x,x,x);
  assert(out>=-32767 && out<=32767);
 }
 printf("PASS V1.47 SOUND SHAPING SAW: neutral=%llu weight=%llu edge=%llu bite=%llu punch=%llu, all sustained audible and bounded\n",
  (unsigned long long)a.energy,(unsigned long long)w.energy,(unsigned long long)e.energy,(unsigned long long)b.energy,(unsigned long long)p.energy);
 return 0;
}
