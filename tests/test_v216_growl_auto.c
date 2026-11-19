/* GROWL free LFO + smooth drift: deterministic, foreground-only, bounded.
 * Does not claim physical FM1 DMA / audio measurements. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_auto.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_waves.h"

#define SR 44100u
static void w16(FILE *f,unsigned v){fputc(v&255u,f);fputc((v>>8)&255u,f);}
static void w32(FILE *f,uint32_t v){w16(f,v&65535u);w16(f,v>>16);}
static void header(FILE *f,uint32_t samples){
 fwrite("RIFF",1,4,f);w32(f,36u+2u*samples);
 fwrite("WAVEfmt ",1,8,f);w32(f,16u);w16(f,1u);w16(f,1u);
 w32(f,SR);w32(f,SR*2u);w16(f,2u);w16(f,16u);
 fwrite("data",1,4,f);w32(f,2u*samples);
}
int main(int argc,char **argv){
 RuGrowlAutoFrame start=ru_growl_auto_frame(0,0);
 assert(start.expression_offset==0&&start.formant_drift==0);
 uint32_t observed=0u;
 int32_t old=0;
 int32_t lo=100,hi=-100;
 for(uint32_t ms=0;ms<240000u;ms+=25u){
  RuGrowlAutoFrame v=ru_growl_auto_frame(ms,0);
  RuGrowlAutoFrame again=ru_growl_auto_frame(ms,0);
  assert(!memcmp(&v,&again,sizeof(v)));
  assert(v.expression_offset>=-16&&v.expression_offset<=16);
  assert(v.formant_drift>=-4&&v.formant_drift<=4);
  if(ms)assert(abs(v.expression_offset-old)<=3);
  if(v.expression_offset<lo)lo=v.expression_offset;
  if(v.expression_offset>hi)hi=v.expression_offset;
  if(v.expression_offset)observed++;
  old=v.expression_offset;
 }
 assert(lo<=-10&&hi>=10&&observed>7000u);
 assert(ru_growl_auto_frame(1000u,1000u).expression_offset==0);
 assert(ru_growl_auto_frame(1000u,1000u).formant_drift==0);
 RuGrowlAutoFrame restored=ru_growl_auto_frame(1450u,1000u);
 RuGrowlAutoFrame unblocked=ru_growl_auto_frame(1450u,0u);
 assert(!memcmp(&restored,&unblocked,sizeof(restored)));

 /* At the loudness and musical extremes, modulation may color the tone
  * but may NEVER mute a played note or push outside the soft output limit. */
 uint32_t phase=0,inc=(uint32_t)(36.708095*(4294967296.0/SR));
 RuGrowlState state={0};double sum=0;
 for(uint32_t ms=0;ms<16000u;ms+=25u){
  RuGrowlAutoFrame automation=ru_growl_auto_frame(ms,0);
  for(uint32_t setting=0;setting<=100u;setting+=50u){
   RuGrowlParams p=ru_growl_make_params(58u,60u,52u,68u,50u,60u,45u,40u);
   int32_t expression=(int32_t)setting+automation.expression_offset;
   if(expression<0)expression=0;
   if(expression>100)expression=100;
   ru_growl_apply_expression(&p,(uint8_t)expression);
   ru_growl_apply_auto_drift(&p,automation.formant_drift);
   for(unsigned k=0;k<8u;k++){
    phase+=inc;int32_t v=ru_growl_process(&state,ru_growl_read(phase,inc),&p);
    assert(v>=-31000&&v<=31000);sum+=(double)v*v;
   }
  }
 }
 assert(sum>1000000000.0);
 if(argc==3&&!strcmp(argv[1],"--preview")){
  FILE *f=fopen(argv[2],"wb");assert(f);
  uint32_t total=SR*12u;header(f,total);
  uint32_t phi=0;RuGrowlState s={0};RuGrowlAutoFrame a={0};
  RuGrowlParams p={0};
  for(uint32_t n=0;n<total;n++){
   /* Recalculate at 25ms ticks, exactly as in the foreground UI. */
   if(n==0u || n%(SR/40u)==0u){
    uint32_t ms=(uint32_t)(((uint64_t)n*1000u)/SR);
    a=ru_growl_auto_frame(ms,0);
    p=ru_growl_make_params(58u,60u,52u,68u,50u,60u,45u,40u);
    int32_t expression=50+(int32_t)a.expression_offset;
    if(expression<0)expression=0;
    if(expression>100)expression=100;
    ru_growl_apply_expression(&p,(uint8_t)expression);
    ru_growl_apply_auto_drift(&p,a.formant_drift);
   }
   phi+=inc;int32_t sample=ru_growl_process(&s,ru_growl_read(phi,inc),&p);
   if(n<SR/20u)sample=(sample*(int32_t)n)/(int32_t)(SR/20u);
   if(n>total-SR/20u)sample=(sample*(int32_t)(total-n))/(int32_t)(SR/20u);
   w16(f,(uint16_t)(int16_t)sample);
  }
  fclose(f);
 }
 printf("PASS V2.0.16: automatic expression/drift +/-16, 25ms continuity, manual priority, D1 headroom, variation %d..%d\n",lo,hi);
 return 0;
}
