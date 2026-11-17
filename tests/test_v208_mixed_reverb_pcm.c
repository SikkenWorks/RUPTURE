/* Exact V2.0.7 PCM baseline for reverb under a full mixed FX chain.
 * Tests SPRING/HALL/ROOM with eight processors, live bypass/moves,
 * independent and legacy shared controls, and abrupt input transients. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static uint32_t run(unsigned engine,unsigned independent){
 struct etch_fx_settings c;etch_fx_defaults(&c);
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 c.reverb_size=24576u;c.reverb_decay=27000u;
 c.wah_mix=19500u;c.wah_sens=24000u;c.folder_mix=21000u;
 c.chorus_depth=18000u;c.delay_feedback=20000u;
 c.comp_ratio=21000u;c.comp_threshold=26000u;
 c.eq_low=23000u;c.eq_mid=17200u;c.eq_high=22000u;
 etch_fx_set_reverb_type(&c,(uint8_t)engine);
 static const uint8_t order[RU_FX_SLOTS]={
  RU_FX_REVERB,RU_FX_MOSFET,RU_FX_COMP,RU_FX_DELAY,
  RU_FX_WAH,RU_FX_CHORUS,RU_FX_DISTORTION,RU_FX_REVERB
 };
 for(unsigned slot=0;slot<RU_FX_SLOTS;slot++){
  assert(ru_chain_insert(&chain,(uint8_t)slot,order[slot]));
  if(order[slot]==RU_FX_REVERB){
   chain.parameter[slot][0]=(uint16_t)engine;
   chain.parameter[slot][1]=27000u;chain.parameter[slot][2]=24576u;
   chain.parameter[slot][3]=slot==0u?29000u:17000u;
  }
 }
 chain.independent=(uint8_t)independent;
 etch_fx_reset(&fx);
 uint32_t hash=2166136261u;
 for(uint32_t n=0u;n<60000u;n++){
  if(n==6000u||n==37000u)ru_chain_toggle(&chain,2u);
  if(n==12000u)ru_chain_move(&chain,3u,1);
  if(n==25000u)ru_chain_move(&chain,7u,-1);
  if(n==42000u)ru_chain_toggle(&chain,0u);
  if(n==46000u)ru_chain_toggle(&chain,0u);
  if(n==31000u){
   if(independent){chain.parameter[0][1]=23500u;chain.parameter[0][2]=21000u;}
   else{c.reverb_decay=23500u;c.reverb_size=21000u;}
  }
  int16_t sample=(int16_t)((n*3701u+(n>>2u)*439u)&65535u);
  if(n%241u==0u)sample=32767;
  int16_t left=0,right=0;
  rupture_fx_process_ordered(&fx,sample,22500u,21000u,28000u,14000u,
    &c,&chain,0u,&left,&right);
  hash=(hash^(uint16_t)left)*16777619u;
  hash=(hash^(uint16_t)right)*16777619u;
 }
 return hash;
}
int main(void){
 /* Filled with the unmodified V2.0.7 engine outputs. */
 static const uint32_t reference[3][2]={
  {101720485u,3957856958u},
  {1673518886u,3426968614u},
  {1531290309u,1261593923u}
 };
 for(unsigned engine=0;engine<3u;engine++)for(unsigned mode=0;mode<2u;mode++){
  uint32_t hash=run(engine,mode);
  if(hash!=reference[engine][mode]){
   fprintf(stderr,"FX PCM changed engine %u mode %u: %u != %u\n",
    engine,mode,hash,reference[engine][mode]);return 1;
  }
  printf("REVERB %u independent=%u PCM=%u\n",engine,mode,hash);
 }
 return 0;
}
