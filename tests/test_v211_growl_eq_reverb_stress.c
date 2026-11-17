/* Sustained GROWL at low EQ frequencies with two sub oscillators + HALL.
 * PCM/headroom check only: actual DMA deadlines still require the FM1. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_waves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"

static struct etch_native_fx fx;
int main(void){
 struct etch_fx_settings c;etch_fx_defaults(&c);etch_fx_reset(&fx);
 c.eq_low=910u;c.eq_mid=0u;c.eq_high=32767u;c.eq_hpf=32767u;
 c.reverb_size=27000u;c.reverb_decay=28500u;
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1u);
 for(uint8_t b=0;b<4u;b++)p.freq[b]=0u;
 p.q[0]=p.q[1]=100u;
 uint16_t g[4]={910u,0u,32767u,0u};
 struct rupture_eq2_bq coeff[4];rupture_eq2_compute(&p,g,coeff);
 rupture_eq2_publish(&fx.eq2,coeff,g);fx.eq2_enabled=1u;
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 assert(ru_chain_insert(&chain,0u,RU_FX_REVERB));
 RuSubState sub1,sub2;ru_sub_init(&sub1,5280000,301);ru_sub_init(&sub2,5280000,113);
 RuGrowlState growl={0,0};
 RuGrowlParams growl_params=ru_growl_make_params(40u,60u,65u,68u,62u,30u,24u,12u);
 uint32_t phase=0u;
 uint32_t digest=2166136261u;uint32_t peaks=0,nonzero=0,fullscale=0;
 for(uint32_t n=0;n<132300u;n++){
  if(n==44100u){ /* Live EQ edit is designed on the control thread. */
   p.freq[0]=14u;p.freq[1]=11u;rupture_eq2_compute(&p,g,coeff);
   rupture_eq2_publish(&fx.eq2,coeff,g);
  }
  int32_t s1=ru_sub_tick(&sub1,2u,1u,40u,70u,5280000,62,82,77,85);
  int32_t s2=ru_sub_tick(&sub2,1u,2u,60u,80u,5280000,62,82,77,85);
  phase+=5280000u;
  if(n%44100u==0u){
   uint8_t bright=(uint8_t)(40u+(n/44100u%2u)*35u);
   growl_params=ru_growl_make_params(bright,60u,65u,68u,62u,30u,24u,12u);
  }
  int32_t carrier=ru_growl_process(&growl,ru_growl_read(phase,5280000u),&growl_params);
  int32_t mono=ru_sub_blend(carrier,s1,s2,ru_sub_smooth_blend(&sub1,73u),
      ru_sub_smooth_blend(&sub2,68u));
  if(mono>26000)mono=26000;else if(mono< -26000)mono=-26000;
  int16_t l=0,r=0;
  rupture_fx_process_ordered(&fx,(int16_t)mono,0u,0u,31500u,0u,
      &c,&chain,0u,&l,&r);
  int32_t peak=abs(l);if(abs(r)>peak)peak=abs(r);
  if((uint32_t)peak>peaks)peaks=(uint32_t)peak;
  if(peak>=32767)fullscale++;
  if(peak>100u)nonzero++;
  digest=(digest^(uint16_t)l)*16777619u^(uint16_t)r;
 }
 assert(fullscale==0u&&peaks<=28000u&&nonzero>10000u);
 printf("PASS V2.0.11 GROWL + dual SUB + low EQ2 + HALL: peak=%u, fullscale=%u, active=%u, PCM=%u\n",
  peaks,fullscale,nonzero,digest);
 return 0;
}
