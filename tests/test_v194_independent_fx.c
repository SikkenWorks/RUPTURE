/* V1.94: repeated FX slots own their controls and small DSP state.
 * Heavy time effects keep the fixed, single existing PCM history. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx fx;
static struct etch_fx_settings global;
static uint32_t render(const struct rupture_fx_chain *chain){
 etch_fx_reset(&fx);etch_fx_defaults(&global);
 uint32_t hash=2166136261u;
 for(uint32_t n=0u;n<19000u;n++){
  int16_t l=0,r=0;
  int16_t x=(int16_t)(((n*271u+763u)&65535u)-32768);
  rupture_fx_process_ordered(&fx,x,18000u,19000u,20000u,14000u,
    &global,chain,0u,&l,&r);
  hash=(hash^(uint16_t)l)*16777619u;
  hash=(hash^(uint16_t)r)*16777619u;
 }
 return hash;
}
static void exercise(uint8_t type,uint8_t knob,uint16_t new_value){
 struct rupture_fx_chain c;ru_chain_clear(&c);
 assert(ru_chain_insert(&c,0u,type));
 assert(ru_chain_insert(&c,1u,type));
 assert(ru_chain_valid(&c));
 uint16_t before=c.parameter[0][knob];
 c.parameter[1][knob]=new_value;
 assert(ru_chain_valid(&c));
 assert(c.parameter[0][knob]==before);
 uint32_t different=render(&c);
 c.parameter[1][knob]=before;
 uint32_t equal=render(&c);
 assert(different!=equal);
 c.parameter[1][knob]=new_value;
 assert(ru_chain_move(&c,1u,1));
 assert(c.parameter[2][knob]==new_value);
 assert(c.parameter[0][knob]==before);
 assert(c.type[2]==type&&c.type[1]==RU_FX_EMPTY);
 ru_chain_remove(&c,2u);
 for(uint8_t i=0u;i<4u;i++)assert(c.parameter[2][i]==0u);
 assert(ru_chain_valid(&c));
}
static void stress_all_eight(uint8_t type){
 struct rupture_fx_chain c;ru_chain_clear(&c);
 for(uint8_t i=0u;i<RU_FX_SLOTS;i++){
  assert(ru_chain_insert(&c,i,type));
  uint16_t *p=c.parameter[i];
  if(type==RU_FX_WAH){p[0]=(uint16_t)(6000u+i*3700u);p[3]=27000u;}
  if(type==RU_FX_DISTORTION){p[0]=(uint16_t)(5000u+i*3500u);p[3]=26000u;}
  if(type==RU_FX_FUZZ){p[0]=(uint16_t)(20u+i*10u);p[3]=75u;}
  if(type==RU_FX_DELAY){p[0]=(uint16_t)(2000u+i*3700u);p[1]=(uint16_t)(3000u+i*3800u);}
  if(type==RU_FX_REVERB){p[3]=(uint16_t)(6000u+i*2800u);}
 }
 assert(ru_chain_valid(&c));
 etch_fx_reset(&fx);etch_fx_defaults(&global);
 uint32_t active=0u;
 for(uint32_t n=0u;n<19000u;n++){
  if(n%2111u==0u)ru_chain_toggle(&c,(uint8_t)((n/2111u)&7u));
  int16_t l=0,r=0;
  int16_t in=(n%111u)<50u?32767:-32768;
  rupture_fx_process_ordered(&fx,in,30000u,30000u,30000u,30000u,
    &global,&c,0u,&l,&r);
  assert((int32_t)l>=-32768&&(int32_t)l<=32767);
  assert((int32_t)r>=-32768&&(int32_t)r<=32767);
  if(l||r)active++;
 }
 assert(active>10000u);
}
int main(void){
 exercise(RU_FX_MOSFET,0u,25000u);
 exercise(RU_FX_COMP,1u,31000u);
 exercise(RU_FX_TILT,0u,90u);
 exercise(RU_FX_FUZZ,0u,90u);
 exercise(RU_FX_SAND,0u,90u);
 exercise(RU_FX_DISTORTION,0u,31000u);
 exercise(RU_FX_WAH,0u,31000u);
 exercise(RU_FX_CHORUS,3u,29000u);
 exercise(RU_FX_DELAY,0u,31000u);
 exercise(RU_FX_DELAY,1u,30000u); /* each feedback contributes to shared history */
 exercise(RU_FX_REVERB,3u,30000u);
 stress_all_eight(RU_FX_WAH);
 stress_all_eight(RU_FX_DISTORTION);
 stress_all_eight(RU_FX_FUZZ);
 stress_all_eight(RU_FX_DELAY);
 stress_all_eight(RU_FX_REVERB);
 puts("PASS V1.94 independent FX control banks, small DSP states, ordered moves");
 return 0;
}
