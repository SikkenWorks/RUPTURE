/* RUPTURE V2.0.4: exact PCM oracle, including repeated effects and
 * per-slot parameters. Every unchanged engine retains its V1.94 hash. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static uint32_t render(uint8_t type,uint8_t count){
 struct etch_fx_settings c;etch_fx_defaults(&c);
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 for(uint8_t s=0u;s<count;s++){
  assert(ru_chain_insert(&chain,s,type));
  uint16_t *p=chain.parameter[s];
  switch(type){
   case RU_FX_REVERB:p[0]=(uint16_t)(s%3u);p[1]=(uint16_t)(4000u+s*2700u);
    p[2]=(uint16_t)(10000u+s*2700u);p[3]=(uint16_t)(9000u+s*2450u);break;
   case RU_FX_DELAY:p[0]=(uint16_t)(1800u+s*3700u);p[1]=(uint16_t)(3000u+s*3800u);
    p[3]=(uint16_t)(10000u+s*2000u);break;
   case RU_FX_CHORUS:p[0]=(uint16_t)(1000u+s*4000u);p[1]=(uint16_t)(5000u+s*3200u);
    p[3]=(uint16_t)(12000u+s*2250u);break;
   case RU_FX_MOSFET:p[0]=(uint16_t)(3200u+s*3700u);break;
   case RU_FX_COMP:p[0]=(uint16_t)(3000u+s*3400u);p[1]=(uint16_t)(9000u+s*3000u);break;
   case RU_FX_TILT:case RU_FX_FUZZ:case RU_FX_SAND:
    p[0]=(uint16_t)(13u+s*10u);p[1]=(uint16_t)(17u+s*11u);
    p[2]=(uint16_t)(27u+s*9u);p[3]=(uint16_t)(38u+s*8u);break;
   case RU_FX_DISTORTION:p[0]=(uint16_t)(3200u+s*3800u);p[1]=(uint16_t)(8000u+s*2500u);
    p[2]=(uint16_t)(15000u+s*1800u);p[3]=(uint16_t)(11000u+s*2500u);break;
   case RU_FX_WAH:p[0]=(uint16_t)(4000u+s*3600u);p[1]=(uint16_t)(6000u+s*2900u);
    p[2]=(uint16_t)(9000u+s*2600u);p[3]=(uint16_t)(8000u+s*2650u);break;
  }
 }
 etch_fx_reset(&fx);
 uint32_t hash=2166136261u;
 for(uint32_t n=0u;n<35000u;n++){
  if(count==8u&&n==13500u)ru_chain_toggle(&chain,3u);
  if(count==8u&&n==18500u)ru_chain_toggle(&chain,3u);
  if(count==8u&&n==23000u)ru_chain_move(&chain,1u,1);
  if(count==8u&&n==28000u)ru_chain_move(&chain,2u,-1);
  int16_t input=(int16_t)((n*3701u+(n>>3u)*17u)&65535u);
  int16_t left=0,right=0;
  rupture_fx_process_ordered(&fx,input,21000u,19500u,17500u,14500u,
    &c,&chain,0u,&left,&right);
  hash=(hash^(uint16_t)left)*16777619u;
  hash=(hash^(uint16_t)right)*16777619u;
 }
 return hash;
}
int main(void){
 /* Golden hashes produced by V1.94 native_fx.c (-O3), except DISTORTION:
  * V2.0.4 deliberately replaces its overdrive-style clipping with metal
  * distortion. Its original hashes were 3229841929 / 910584237.
  * The unchanged engines remain byte-identical to their historic oracle. */
 static const uint32_t reference[RU_FX_TYPES][2]={
  {2485899402u,140647317u},
  {3397896071u,2368806137u},
  {305421467u,2026017452u},
  {1175263665u,2082340857u},
  {1465874023u,1602007509u},
  {388186517u,1941994965u},
  {489272983u,994852517u},
  {1874503083u,1003962987u},
  {2948157681u,471580635u},
  {1886883071u,3979176575u},
 };
 for(uint8_t t=0u;t<RU_FX_TYPES;t++){
  uint32_t single=render(t,1u),eight=render(t,8u);
  if(single!=reference[t][0]||eight!=reference[t][1]){
   fprintf(stderr,"PCM MISMATCH type %u: %u/%u (reference %u/%u)\n",
    t,single,eight,reference[t][0],reference[t][1]);
   return 1;
  }
 }
 puts("PASS V2.0.4 all ten FX, historic PCM parity except intentional metal distortion");
 return 0;
}
