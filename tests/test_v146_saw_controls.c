/* V1.46: real SAW audio dials and bounded fixed-point output. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_bandwaves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_saw_tone.h"
static uint64_t render(uint8_t bright,uint8_t edge,uint8_t body,uint8_t level,uint8_t shape_edge,int *peak){
 uint32_t phase=0u,inc=10712311u;int32_t lp=0;uint64_t hash=1469598103934665603ull;
 int high=0;
 for(int n=0;n<50000;n++){
  phase+=inc;
  int32_t raw=ru_bandwave_read(RU_WAVE_SAW,phase,inc);
  int32_t out=rupture_saw_tone(&lp,raw,bright,edge,body,level,shape_edge,0,0,0);
  assert(out>=-32767 && out<=32767);
  if(out>high)high=out;
  hash=(hash^(uint16_t)out)*1099511628211ull;
 }
 if(peak)*peak=high;
 return hash;
}
int main(void){
 int p=0;
 const uint64_t baseline=render(58,50,52,68,50,&p);
 assert(p>1000 && p<=32767);
 assert(render(2,50,52,68,50,0)!=baseline); /* BRIGHT */
 assert(render(58,94,52,68,50,0)!=baseline); /* EDGE */
 assert(render(58,50,2,68,50,0)!=baseline);  /* BODY */
 assert(render(58,50,52,2,50,0)!=baseline);  /* LEVEL */
 assert(render(0,0,0,0,50,0)!=render(100,100,100,100,50,0));
 /* Clear low-frequency state for new note; no unbounded accumulation. */
 int32_t low=0;
 for(int n=0;n<300000;n++){
  int32_t x=(n&1)?29000:-29000;
  int32_t y=rupture_saw_tone(&low,x,100,100,100,100,100,0,0,0);
  assert(y>=-32767&&y<=32767);
  assert(low>=-29000 && low<=29000);
 }
 puts("V1.46 SAW four-knob audio and bounds PASS");return 0;
}
