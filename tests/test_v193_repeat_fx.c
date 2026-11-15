/* V1.93: repeatable slots do not duplicate a shared time-based ring write.
 * Compare full PCM output for repeated types; keep the old unique route. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx state;
static struct etch_fx_settings settings;

static uint32_t render(uint8_t type,uint8_t repeats,uint8_t bypass_second){
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 assert(ru_chain_insert(&chain,0u,type));
 if(repeats){
  assert(ru_chain_insert(&chain,1u,type));
  if(bypass_second)ru_chain_toggle(&chain,1u);
 }
 assert(ru_chain_valid(&chain));
 etch_fx_defaults(&settings);etch_fx_reset(&state);
 settings.comp_ratio=30000u;
 settings.comp_threshold=24000u;
 settings.folder_fold=21000u;settings.folder_mix=28000u;
 settings.wah_sens=28000u;settings.wah_mix=28000u;
 uint32_t hash=2166136261u;
 for(uint32_t n=0;n<22000u;n++){
  int16_t x=(n%77u<35u)?20000:-18000;
  int16_t l=0,r=0;
  rupture_fx_process_ordered(&state,x,30000u,25000u,28000u,16000u,
    &settings,&chain,0u,&l,&r);
  hash=(hash^(uint16_t)l)*16777619u;
  hash=(hash^(uint16_t)r)*16777619u;
 }
 if(type==RU_FX_CHORUS)assert(state.chorus_pos==22000u%ETCH_CHORUS_SIZE);
 if(type==RU_FX_DELAY)assert(state.delay_pos==22000u%ETCH_DELAY_SIZE);
 if(type==RU_FX_REVERB)assert(state.reverb_pos==22000u%ETCH_REVERB_SIZE);
 return hash;
}
static void live_control_stress(uint8_t type){
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 for(uint8_t slot=0u;slot<RU_FX_SLOTS;slot++)
  assert(ru_chain_insert(&chain,slot,type));
 etch_fx_defaults(&settings);etch_fx_reset(&state);
 settings.wah_mix=32767u;settings.wah_sens=32767u;
 settings.wah_attack=32767u;settings.wah_reso=32767u;
 settings.folder_mix=32767u;settings.folder_fold=32767u;
 settings.folder_symmetry=32767u;settings.folder_tone=32767u;
 uint32_t nonzero=0u;
 for(uint32_t n=0u;n<30000u;n++){
  if(n%127u==0u)ru_chain_toggle(&chain,(uint8_t)((n/127u)&7u));
  if(n%4096u==0u){assert(ru_chain_move(&chain,2u,1));}
  int16_t l=0,r=0;
  rupture_fx_process_ordered(&state,(n&1u)?32767:-32768,
      32767u,32767u,32767u,32767u,&settings,&chain,0u,&l,&r);
  if(l||r)nonzero++;
  assert(l>=-32768 && l<=32767 && r>=-32768 && r<=32767);
 }
 assert(nonzero>10000u && ru_chain_valid(&chain));
}

int main(void){
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 for(uint8_t i=0u;i<RU_FX_SLOTS;i++)assert(ru_chain_insert(&chain,i,RU_FX_DELAY));
 assert(ru_chain_valid(&chain)&&chain.enabled==0xffu);
 ru_chain_toggle(&chain,4u);
 assert((chain.enabled&0x10u)==0u && (chain.enabled&0x08u)!=0u);
 assert(ru_chain_move(&chain,4u,-1));
 assert((chain.enabled&0x08u)==0u && (chain.enabled&0x10u)!=0u);
 ru_chain_remove(&chain,2u);assert(ru_chain_valid(&chain));
 /* The second insertion changes the actual PCM, not only the list. */
 const uint8_t types[]={RU_FX_REVERB,RU_FX_DELAY,RU_FX_CHORUS,
     RU_FX_DISTORTION,RU_FX_WAH,RU_FX_MOSFET,RU_FX_COMP};
 for(uint8_t i=0u;i<(uint8_t)(sizeof(types)/sizeof(types[0]));i++){
  uint32_t one=render(types[i],0u,0u);
  int32_t wah_envelope=state.wah_env,wah_lp=state.wah_lp,wah_hp=state.wah_hp;
  int32_t compressor_envelope=state.comp_env,distortion_tone=state.folder_tone_state;
  uint32_t twice=render(types[i],1u,0u);
  if(types[i]==RU_FX_WAH)
   assert(state.wah_env==wah_envelope && state.wah_lp==wah_lp && state.wah_hp==wah_hp);
  if(types[i]==RU_FX_COMP)assert(state.comp_env==compressor_envelope);
  if(types[i]==RU_FX_DISTORTION)assert(state.folder_tone_state==distortion_tone);
  uint32_t one_bypassed=render(types[i],1u,1u);
  if(twice==one)fprintf(stderr,"No PCM difference for repeated type %u: %u\n",types[i],one);
  assert(twice!=one);
  assert(one_bypassed==one); /* settled bypass is bit-identical */
 }
 live_control_stress(RU_FX_WAH);
 live_control_stress(RU_FX_DISTORTION);
 puts("PASS V1.93 repeatable FX, per-slot bypass, non-advancing shared tanks, PCM layering");
 return 0;
}
