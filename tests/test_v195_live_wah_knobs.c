/* V1.94 PCM oracle for moving, toggling and editing THREE WAH inserts
 * while the audio callback is running. Uses the target's own native_fx.h. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "native_fx.h"

static struct etch_native_fx fx;
int main(void){
 struct etch_fx_settings c;etch_fx_defaults(&c);
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 for(uint8_t s=0;s<3u;s++){
  assert(ru_chain_insert(&chain,s,RU_FX_WAH));
  chain.parameter[s][0]=(uint16_t)(11000u+s*6200u);
  chain.parameter[s][1]=(uint16_t)(6200u+s*5800u);
  chain.parameter[s][2]=(uint16_t)(9200u+s*6800u);
  chain.parameter[s][3]=(uint16_t)(20000u+s*4000u);
 }
 etch_fx_reset(&fx);
 uint32_t hash=2166136261u;
 for(uint32_t n=0;n<44000u;n++){
  if(n==2100u)chain.parameter[1][1]=32767u;
  if(n==5400u)chain.parameter[1][2]=32000u;
  if(n==10000u)chain.parameter[0][1]=300u;
  if(n==14500u){chain.parameter[2][1]=0u;chain.parameter[2][2]=0u;}
  if(n==19000u)assert(ru_chain_move(&chain,0u,1));
  if(n==22500u)ru_chain_toggle(&chain,0u);
  if(n==26000u)ru_chain_toggle(&chain,0u);
  if(n==29000u){chain.parameter[1][1]=16000u;chain.parameter[1][2]=13000u;}
  if(n==34000u){ru_chain_remove(&chain,2u);assert(ru_chain_insert(&chain,2u,RU_FX_WAH));}
  if(n==38500u)chain.parameter[2][2]=7000u;
  int16_t input=(int16_t)(((uint32_t)n*2003u+(n>>2u)*191u)&65535u);
  int16_t left=0,right=0;
  rupture_fx_process_ordered(&fx,input,18000u,16000u,22000u,13000u,
   &c,&chain,0u,&left,&right);
  hash=(hash^(uint16_t)left)*16777619u;
  hash=(hash^(uint16_t)right)*16777619u;
 }
 if(hash!=1529925597u){
  fprintf(stderr,"FAIL live-knob PCM parity: got %u, expected 1529925597\n",hash);
  return 1;
 }
 puts("PASS three live-editable WAH instances, knob turns and moves: V1.94 bit-exact PCM");
 return 0;
}
