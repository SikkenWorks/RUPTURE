/* A zero-mix insert must not reuse the previous same-type insert's wet signal. */
#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx state;
static int16_t reference[1024][2];

int main(void){
 const uint8_t types[]={RU_FX_TILT,RU_FX_FUZZ,RU_FX_SAND};
 struct etch_fx_settings settings;
 etch_fx_defaults(&settings);
 for(unsigned kind=0;kind<sizeof(types);kind++){
  struct rupture_fx_chain chain;
  ru_chain_clear(&chain);
  chain.independent=1;
  assert(ru_chain_insert(&chain,1,types[kind]));
  chain.parameter[1][0]=82;
  for(unsigned duplicate=0;duplicate<2;duplicate++){
   if(duplicate){
    assert(ru_chain_insert(&chain,6,types[kind]));
    chain.parameter[6][types[kind]==RU_FX_TILT?2:3]=0;
   }
   etch_fx_reset(&state);
   struct rupture_fx_dispatch dispatch=ru_chain_prepare_dispatch(&chain);
   uint32_t rng=0x12345678u;
   for(unsigned frame=0;frame<1024;frame++){
    rng=rng*1664525u+1013904223u;
    int16_t left,right,input=(int16_t)(rng>>16);
    rupture_fx_process_ordered_fast(&state,input,0,0,0,0,&settings,&chain,0,
     &left,&right,&dispatch,0,0);
    if(duplicate){
     assert(left==reference[frame][0]&&right==reference[frame][1]);
    }else{
     reference[frame][0]=left;
     reference[frame][1]=right;
    }
   }
  }
 }
 puts("PASS zero-mix duplicate TILT, FUZZ and SANDPAPER inserts");
}
