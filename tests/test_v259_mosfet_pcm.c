/* Verifies audible MOSFET wet/dry route without altering legacy presets. */
#define RUPTURE_BUILD 1
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "native_fx.h"
static struct etch_native_fx fx;
static int16_t process(uint16_t p3){
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 ru_chain_insert(&chain,0u,RU_FX_MOSFET);
 chain.parameter[0][0]=20000u;
 chain.parameter[0][3]=p3;
 struct etch_fx_settings cfg;etch_fx_defaults(&cfg);
 etch_fx_reset(&fx);
 int16_t l=0,r=0;
 rupture_fx_process_ordered(&fx,12000,0,0,0,0,&cfg,&chain,0u,&l,&r);
 assert(l==r);return l;
}
int main(void){
 const int16_t full_legacy=process(0u);
 const int16_t dry=process(1u);
 const int16_t half=process(16384u);
 const int16_t full_new=process(32767u);
 assert(full_legacy==full_new);
 assert(dry==12000);
 assert((half>dry&&half<full_new)||(half<dry&&half>full_new));
 printf("PASS V2.0.59 MOSFET PCM: DRY=%d MID=%d WET=%d legacy=%d\n",dry,half,full_new,full_legacy);
}
