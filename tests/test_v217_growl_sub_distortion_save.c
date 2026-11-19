/* RUPTURE V2.0.17: a saved GROWL/SUB/Distortion preset must still use
 * the exact same PJT35 FX chain after project/preset reload at cold boot. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_patch_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_main_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_sub_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_waves.h"

static struct etch_wave_library before, after;
static struct etch_native_pattern pattern, pattern_after;
static struct etch_project_settings cfg, restored;
static struct etch_preset_dsp_bank dsp, dsp_after;
static uint8_t encoded[ETCH_PROJECT_MAX_BYTES];

static uint32_t render_growl_sub_distortion(const struct etch_project_settings *project){
 /* Host-only deterministic heavy bass preview. Fresh DSP state models
  * cold boot; this asserts sound identity, NOT hardware DMA capacity. */
 static struct etch_native_fx fx;
 etch_fx_reset(&fx);
 RuGrowlState growl={0};
 RuGrowlParams tone=ru_growl_make_params(58u,50u,52u,68u,50u,60u,45u,40u);
 ru_growl_apply_expression(&tone,50u);
 uint32_t main_phase=0u,s1=0u,s2=0u;
 uint32_t hash=2166136261u;
 int32_t maximum=0;
 for(uint32_t n=0;n<25000u;n++){
  main_phase+=3575397u;s1+=1787698u;s2+=893849u;
  int32_t main=ru_growl_process(&growl,ru_growl_read(main_phase,3575397u),&tone);
  int32_t sub1=(int16_t)(s1>>16u),sub2=(int16_t)(s2>>16u);
  int32_t combined=main/2+sub1/6+sub2/6;
  if(combined>30000)combined=30000;
  if(combined< -30000)combined=-30000;
  int16_t left=0,right=0;
  rupture_fx_process_ordered(&fx,(int16_t)combined,0u,0u,0u,0u,
    &project->global_fx,&project->v190.chain,project->rupture_fx.bypass,&left,&right);
  int32_t abs_left=left<0?-(int32_t)left:left;
  if(abs_left>maximum)maximum=abs_left;
  hash=(hash^(uint16_t)left)*16777619u;
  hash=(hash^(uint16_t)right)*16777619u;
 }
 assert(maximum>1000&&maximum<=32767);
 return hash;
}

static uint32_t save_and_reboot(uint8_t version){
 uint32_t length=0;
 cfg.source_version=version;
 assert(etch_project_encode(&before,&pattern,&cfg,encoded,sizeof(encoded),&length)==0);
 assert(etch_project_decode(encoded,length,&after,&pattern_after,&restored)==0);
 return length;
}

int main(void){
 cfg.seq_length=16u;cfg.seq_gate=75u;cfg.tempo=120u;
 etch_fx_defaults(&cfg.global_fx);
 ru_fxm_defaults(&cfg.rupture_fx);
 rupture_eq2_defaults(&cfg.eq2,1u);
 ru_v190_defaults(&cfg.v190);
 etch_project_bind_preset_dsp(&cfg,&dsp);
 etch_project_bind_preset_dsp(&restored,&dsp_after);
 etch_perf_init(&dsp.performance);
 etch_key_groups_init(&dsp.groups);
 for(uint8_t i=0u;i<ETCH_PATCH_SLOTS;i++){
  dsp.slot[i].spectral_part=2u;dsp.slot[i].spectral_gain=50u;
  dsp.slot[i].spectral_tilt=50u;dsp.slot[i].blend_a=i;dsp.slot[i].blend_b=i;
 }
 before.valid[0]=1u;before.patches[0].valid=1u;
 before.patches[0].seq_length=16u;before.patches[0].tempo=120u;
 ru_patch_cxs_save(&before.patches[0],58u,50u,52u,68u);
 ru_main_state_store(&dsp.slot[0],1u); /* GROWL */
 const uint8_t type[2]={2u,1u},oct[2]={1u,2u};
 const uint8_t detune[2]={52u,47u},drift[2]={3u,5u};
 ru_sub_state_store(&dsp.slot[0],type,oct,detune,drift);
 /* PRESET SAVE has to capture SUB BLEND even without PROJECT SAVE. */
 cfg.rupture_fx.knobs[0][0]=83u;
 cfg.rupture_fx.knobs[0][1]=71u;
 cfg.rupture_fx.enabled=0x30u;
 cfg.rupture_fx.bypass=0x10u;
 assert(ru_chain_insert(&cfg.v190.chain,0u,RU_FX_DISTORTION));
 cfg.v190.chain.parameter[0][0]=25800u;
 cfg.v190.chain.parameter[0][1]=14500u;
 cfg.v190.chain.parameter[0][2]=20000u;
 cfg.v190.chain.parameter[0][3]=29000u;
 cfg.v190.legacy_chain=0u;
 cfg.global_fx.folder_fold=25800u;
 cfg.global_fx.folder_tone=20000u;
 /* This was the real pre-fix failure: legacy PJT34 accepted the save but
  * dropped the active distortion slot and all independent FX parameters. */
 uint32_t old_size=save_and_reboot(34u);
 assert(restored.source_version==34u);
 assert(restored.v190.chain.type[0]==RU_FX_EMPTY);
 uint32_t new_size=save_and_reboot(35u);
 assert(new_size>old_size);
 assert(restored.source_version==35u);
 assert(restored.v190.legacy_chain==0u);
 assert(!memcmp(&cfg.v190,&restored.v190,sizeof(cfg.v190)));
 assert(restored.v190.chain.type[0]==RU_FX_DISTORTION);
 assert(restored.v190.chain.enabled==1u);
 assert(restored.v190.chain.parameter[0][3]==29000u);
 assert(restored.global_fx.folder_fold==25800u);
 assert(restored.rupture_fx.enabled==0x30u);
 assert(restored.rupture_fx.knobs[0][0]==83u);
 assert(restored.rupture_fx.knobs[0][1]==71u);
 assert(restored.rupture_fx.bypass==0x10u);
 assert(ru_main_state_restore(&dsp_after.slot[0])==1u);
 uint8_t rt[2],ro[2],rd[2],rr[2];
 assert(ru_sub_state_restore(&dsp_after.slot[0],rt,ro,rd,rr));
 assert(!memcmp(type,rt,2u)&&!memcmp(oct,ro,2u));
 assert(!memcmp(detune,rd,2u)&&!memcmp(drift,rr,2u));
 uint32_t live_pcm=render_growl_sub_distortion(&cfg);
 uint32_t boot_pcm=render_growl_sub_distortion(&restored);
 assert(live_pcm==boot_pcm); /* saved/reopened sound must be bit-identical */
 puts("PASS V2.0.17: PJT34 regression reproduced; PJT35 retains GROWL, dual SUB, distortion and levels after reboot");
 return 0;
}
