/* Audible dark GROWL and bounded LATCH/RETURN gestures. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_waves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_expression.h"
static double rms(uint8_t b,uint8_t e,uint8_t body,uint8_t expression,uint8_t level){
 RuGrowlParams p=ru_growl_make_params(b,e,body,level,0,0,0,0);
 ru_growl_apply_expression(&p,expression);
 RuGrowlState s={0};uint32_t phase=0;double energy=0.0;
 for(uint32_t n=0;n<22050u;n++){
  phase+=3919000u;
  int32_t y=ru_growl_process(&s,ru_growl_read(phase,3919000u),&p);
  assert(y<=31000&&y>=-31000);
  if(n>=11025u)energy+=(double)y*y;
 }
 return sqrt(energy/11025.0);
}
int main(void){
 for(uint8_t b=0;b<=100;b+=50)for(uint8_t e=0;e<=100;e+=50){
  assert(rms(b,e,0,0,68)>300.0);
  assert(rms(b,e,50,0,68)>300.0);
 }
 assert(rms(0,0,0,0,0)==0.0);
 RuGrowlParams a=ru_growl_make_params(58,60,52,68,50,60,45,40),z=a;
 ru_growl_apply_expression(&z,50u);
 assert(memcmp(&a,&z,sizeof(a))==0);
 assert(ru_growl_expression_return_step(0,1124u,1000u)==0u);
 uint8_t low=0u,high=100u;
 for(unsigned i=0;i<10u;i++){
  low=ru_growl_expression_return_step(low,1125u+i*25u,1000u);
  high=ru_growl_expression_return_step(high,1125u+i*25u,1000u);
 }
 assert(low==50u&&high==50u);
 puts("PASS V2.0.14: audible zero, exact neutral, RETURN 125ms + 10 ticks");
 return 0;
}
