/* Specific hardware-reported AUDIO STOP rack: MOSFET, DISTORTION, REVERB.
 * Host test checks sound parity, NOT processor deadline feasibility on FM1. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_sub_realtime.h"

static struct etch_native_fx fx;
static uint64_t hash=UINT64_C(14695981039346656037);
static uint32_t frames;

static void exercise(const uint8_t order[3],uint8_t reverb_type,
                     uint8_t independent,uint8_t sub_on){
 struct rupture_fx_chain chain;
 struct etch_fx_settings settings;
 ru_chain_clear(&chain);etch_fx_defaults(&settings);
 etch_fx_set_reverb_type(&settings,reverb_type);
 settings.reverb_decay=31200u;settings.reverb_size=30900u;
 settings.folder_mix=31000u;settings.folder_fold=32767u;
 settings.folder_tone=28000u;
 chain.independent=independent;
 for(uint8_t slot=0u;slot<3u;slot++){
  uint8_t kind=order[slot];
  assert(ru_chain_insert(&chain,slot,kind));
  if(kind==RU_FX_REVERB){
   chain.parameter[slot][0]=reverb_type;
   chain.parameter[slot][1]=30000u;
   chain.parameter[slot][2]=30200u;
   chain.parameter[slot][3]=31000u;
  }
  if(kind==RU_FX_DISTORTION){
   chain.parameter[slot][0]=32767u;
   chain.parameter[slot][1]=19000u;
   chain.parameter[slot][2]=28000u;
   chain.parameter[slot][3]=32767u;
  }
  if(kind==RU_FX_MOSFET)chain.parameter[slot][0]=30000u;
 }
 etch_fx_reset(&fx);
 struct rupture_fx_dispatch dispatch=ru_chain_prepare_dispatch(&chain);
 struct rupture_fx_distortion_cache coeffs={0};
 RuSubState sub;
 RuSubRt rt={0};
 ru_sub_init(&sub,0x12001200u,0x30123012u);
 uint8_t checked=0;
 for(uint32_t i=0u;i<4096u;i++){
  if(!(i&31u)){checked=0u;coeffs.valid_mask=0u;}
  /* First and second note onsets, note-off and long sustained third note. */
  uint8_t held=(uint8_t)((i<256u)||(i>=512u&&i<768u)||(i>=1024u&&i<3584u));
  int32_t main=held?((i&1u)?-18000:18000):0;
  if(sub_on){
   int32_t sub_sample=ru_sub_rt_tick(&rt,&sub,RU_SUB_CXS,1u,52u,25u,
         0x013579bdu,68u,57u,62u,66u);
   main+=held?(sub_sample>>2u):0;
  }
  if(main>32767)main=32767;
  if(main<-32768)main=-32768;
  int16_t l,r;
  rupture_fx_process_ordered_fast(&fx,(int16_t)main,20000u,22000u,
      32000u,30000u,&settings,&chain,0u,&l,&r,&dispatch,&checked,&coeffs);
  hash^=(uint16_t)l|((uint32_t)(uint16_t)r<<16u);
  hash*=UINT64_C(1099511628211);
 }
 frames+=4096u;
}

int main(void){
 const uint8_t names[]={RU_FX_MOSFET,RU_FX_DISTORTION,RU_FX_REVERB};
 unsigned cases=0u;
 for(uint8_t a=0u;a<3u;a++)for(uint8_t b=0u;b<3u;b++){
  if(a==b)continue;
  uint8_t c=(uint8_t)(3u-a-b);
  uint8_t order[3]={names[a],names[b],names[c]};
  for(uint8_t reverb=0u;reverb<3u;reverb++)
  for(uint8_t independent=0u;independent<2u;independent++)
  for(uint8_t sub=0u;sub<2u;sub++){
   exercise(order,reverb,independent,sub);cases++;
  }
 }
 assert(cases==72u && frames==294912u);
 printf("MOSFET_DIST_REVERB cases=%u stereo_frames=%u hash=%016llx\n",
        cases,frames,(unsigned long long)hash);
 return 0;
}
