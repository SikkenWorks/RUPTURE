/* GROWL expression must preserve the neutral patch and remain bounded. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_waves.h"
static uint32_t render(uint8_t expression){
 RuGrowlParams p=ru_growl_make_params(58,60,52,68,50,60,45,40);
 ru_growl_apply_expression(&p,expression);
 RuGrowlState state={0};uint32_t phase=0;
 uint32_t fingerprint=2166136261u, maxjump=0;
 int32_t prior=0;
 for(unsigned i=0;i<44100u;i++){
  phase+=5290000u;
  int32_t sample=ru_growl_process(&state,ru_growl_read(phase,5290000u),&p);
  assert(sample<=31000&&sample>=-31000);
  uint32_t jump=(uint32_t)abs(sample-prior);
  if(i>128u&&jump>maxjump)maxjump=jump;
  fingerprint=(fingerprint^(uint16_t)sample)*16777619u;
  prior=sample;
 }
 assert(maxjump<20000u);
 return fingerprint;
}
int main(void){
 RuGrowlParams original=ru_growl_make_params(58,60,52,68,50,60,45,40);
 RuGrowlParams neutral=original;
 ru_growl_apply_expression(&neutral,50u);
 assert(memcmp(&original,&neutral,sizeof(original))==0);
 uint32_t dark=render(0),middle=render(50),snarl=render(100);
 assert(dark!=middle&&snarl!=middle&&dark!=snarl);
 /* Four distinct tonal dimensions must move, not just master gain. */
 RuGrowlParams low=original,high=original;
 ru_growl_apply_expression(&low,0);ru_growl_apply_expression(&high,100);
 assert(low.low_a<high.low_a&&low.throat_a<high.throat_a);
 assert(low.drive_q8<high.drive_q8&&low.fold_q8<high.fold_q8);
 assert(low.band_q10<high.band_q10&&low.presence_q10<high.presence_q10);
 printf("PASS V2.0.13 GROWL expression: dark=%u neutral=%u snarling=%u\n",dark,middle,snarl);
 return 0;
}
