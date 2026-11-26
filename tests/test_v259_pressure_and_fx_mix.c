/* V2.0.59: bounded physical pressure and live MIX target invariants. */
#define RUPTURE_BUILD 1
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "rupture_key_touch.h"
int main(void){
 struct ru_touch_state st={.active_key=255u};
 uint16_t reads[20]={0},gens[20]={0};
 const uint8_t conf=ru_touch_config(RU_TOUCH_REVERB,1u);
 assert(ru_touch_update(&st,1u,reads,gens,20u,conf)==0u);
 reads[0]=600u;gens[0]++;assert(!ru_touch_update(&st,1u,reads,gens,40u,conf));
 uint8_t prior=0;
 for(uint8_t n=0;n<25u;n++){
  reads[0]=(uint16_t)(600u-n*10u);gens[0]++;
  uint8_t now=ru_touch_update(&st,1u,reads,gens,60u+n*20u,conf);
  assert(now>=prior && now-prior<=8u);prior=now;
  if(n<10u)assert(now<100u);
 }
 assert(prior>0u);
 reads[0]=600u;gens[0]++;
 uint8_t down=ru_touch_update(&st,1u,reads,gens,800u,conf);
 assert(down<prior);
 assert(ru_touch_update(&st,0u,reads,gens,820u,conf)==0u);
 struct etch_native_controls c={0};
 c.fx_chain_active=1u;ru_chain_clear(&c.fx_chain);
 ru_chain_insert(&c.fx_chain,0u,RU_FX_MOSFET);
 uint16_t old_drive=c.fx_chain.parameter[0][0];
 struct ru_touch_profile p={RU_TOUCH_MOSFET,RU_TOUCH_MODE_PRESS,1u,0u,100u,1u};
 ru_touch_apply_profile(&c,&p,0u);
 assert(c.fx_chain.parameter[0][3]==1u && c.fx_chain.parameter[0][0]==old_drive);
 ru_touch_apply_profile(&c,&p,64u);
 assert(c.fx_chain.parameter[0][3]>10000u && c.fx_chain.parameter[0][3]<30000u);
 ru_touch_apply_profile(&c,&p,127u);
 assert(c.fx_chain.parameter[0][3]==32767u);
 const uint8_t ts[]={RU_FX_REVERB,RU_FX_DISTORTION,RU_FX_WAH};
 for(uint8_t i=0;i<3u;i++){
  ru_chain_clear(&c.fx_chain);ru_chain_insert(&c.fx_chain,0u,ts[i]);
  uint16_t original_drive=c.fx_chain.parameter[0][0];
  p.target=ts[i]==RU_FX_REVERB?RU_TOUCH_REVERB:
   ts[i]==RU_FX_DISTORTION?RU_TOUCH_DISTORTION:RU_TOUCH_WAH;
  ru_touch_apply_profile(&c,&p,0u);
  assert(c.fx_chain.parameter[0][3]==0u && c.fx_chain.parameter[0][0]==original_drive);
  ru_touch_apply_profile(&c,&p,127u);
  assert(c.fx_chain.parameter[0][3]==32767u);
 }
 puts("PASS V2.0.59 physical pressure gradual, effects control MIX not DRIVE");
}
