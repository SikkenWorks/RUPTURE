/* RUPTURE V2.0.15: reference-driven GROWL and musical expression range.
 * Host DSP testing is NOT a measurement of the physical FM1 DMA budget. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_waves.h"

#define SR 44100u
static void w16(FILE *f,unsigned v){fputc(v&255u,f);fputc((v>>8)&255u,f);}
static void w32(FILE *f,uint32_t v){w16(f,v&65535u);w16(f,v>>16);}
static void header(FILE *f,uint32_t frames){
 fwrite("RIFF",1,4,f);w32(f,36u+frames*2u);fwrite("WAVEfmt ",1,8,f);
 w32(f,16u);w16(f,1u);w16(f,1u);w32(f,SR);w32(f,SR*2u);
 w16(f,2u);w16(f,16u);fwrite("data",1,4,f);w32(f,frames*2u);
}
static double listen(uint8_t expression,uint32_t inc,uint8_t b,uint8_t edge){
 RuGrowlParams p=ru_growl_make_params(b,edge,52,68,50,60,45,40);
 ru_growl_apply_expression(&p,expression);
 RuGrowlState s={0};uint32_t phase=0;double energy=0;
 for(uint32_t i=0;i<SR;i++){
  phase+=inc;
  int32_t y=ru_growl_process(&s,ru_growl_read(phase,inc),&p);
  assert(y>=-31000&&y<=31000);
  if(i>=SR/2u)energy+=(double)y*y;
 }
 return sqrt(energy/(SR/2u));
}
int main(int argc,char **argv){
 const uint32_t d1=(uint32_t)(36.708095*(4294967296.0/SR));
 const uint32_t notes[]={d1,(uint32_t)(110.0*(4294967296.0/SR)),
    (uint32_t)(220.0*(4294967296.0/SR)),
    (uint32_t)(440.0*(4294967296.0/SR))};
 for(unsigned note=0;note<sizeof(notes)/sizeof(notes[0]);note++){
  for(unsigned b=0;b<=100;b+=50)for(unsigned edge=0;edge<=100;edge+=50){
   double a=listen(0,notes[note],b,edge);
   double z=listen(100,notes[note],b,edge);
   assert(a>300&&z>300);
  }
 }
 RuGrowlParams neutral=ru_growl_make_params(58,60,52,68,50,60,45,40);
 RuGrowlParams alt=neutral;
 ru_growl_apply_expression(&alt,50);
 assert(memcmp(&neutral,&alt,sizeof(neutral))==0);
 double levels[5];
 for(unsigned i=0;i<5;i++)levels[i]=listen((uint8_t)(i*25u),d1,58,60);
 for(unsigned i=0;i<5;i++)printf("EXPRESSION %3u | RMS %.1f\n",i*25u,levels[i]);
 assert(levels[0]>300&&levels[4]>300);
 assert(neutral.low_a!=0&&neutral.throat_a!=0);
 /* Motion adds only a precomputed pressure impulse, never an oscillator. */
 RuGrowlParams movement=neutral;
 ru_growl_apply_motion(&movement,24u);
 assert(movement.drive_q8>neutral.drive_q8);
 assert(movement.band_q10>neutral.band_q10);
 assert(movement.fold_q8>neutral.fold_q8);

 if(argc==3&&!strcmp(argv[1],"--preview")){
  /* 8-second original synthesizer demonstration, D1 throughout:
   * closed->open->closed expressive sweep, then three stationary vowels. */
  const uint32_t len=SR*8u;
  FILE *wav=fopen(argv[2],"wb");assert(wav);header(wav,len);
  RuGrowlState s={0};uint32_t phase=0;uint8_t expr=50,motion=0;
  for(uint32_t n=0;n<len;n+=64u){
   double t=(double)n/(double)SR;
   uint8_t target;
   if(t<2.5)target=(uint8_t)(100.0*t/2.5);
   else if(t<5.0)target=(uint8_t)(100.0*(5.0-t)/2.5);
   else if(t<6)target=0;
   else if(t<7)target=50;
   else target=100;
   uint8_t prior=expr;
   /* Mirror FM1's two-per-64-frame target slewing. */
   if(expr<target)expr=(uint8_t)(expr+((target-expr)>2?2:(target-expr)));
   else if(expr>target)expr=(uint8_t)(expr-((expr-target)>2?2:(expr-target)));
   RuGrowlParams p=ru_growl_make_params(58,60,52,68,50,60,45,40);
   ru_growl_apply_expression(&p,expr);
   if(expr!=prior){uint8_t diff=expr>prior?expr-prior:prior-expr;
    uint8_t impact=(uint8_t)(diff*6u);if(impact>motion)motion=impact;}
   if(motion){ru_growl_apply_motion(&p,motion);motion--;}
   for(uint32_t i=0;i<64u&&n+i<len;i++){
    phase+=d1;
    int32_t y=ru_growl_process(&s,ru_growl_read(phase,d1),&p);
    uint32_t pos=n+i;
    if(pos<SR/20u)y=(y*(int32_t)pos)/(int32_t)(SR/20u);
    if(pos>=len-SR/20u)y=(y*(int32_t)(len-pos))/(int32_t)(SR/20u);
    w16(wav,(uint16_t)(int16_t)y);
   }
  }
  fclose(wav);
 }
 puts("PASS V2.0.15: 4 pitches x 9 edge/bright pairs x 2 expression endpoints, all audible/bounded; neutral; movement");
 return 0;
}
