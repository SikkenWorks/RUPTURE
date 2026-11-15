#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
static struct etch_wave_library lib,read_lib;
static struct etch_native_pattern seq,read_seq;
static struct etch_project_settings project,decoded;
static struct etch_preset_dsp_bank bank,read_bank;
static uint8_t buf[ETCH_PROJECT_MAX_BYTES];
int main(void){
 project.seq_length=16u;project.seq_gate=75u;project.tempo=120u;
 etch_fx_defaults(&project.global_fx);
 ru_fxm_defaults(&project.rupture_fx);
 rupture_eq2_defaults(&project.eq2,1u);
 ru_v190_defaults(&project.v190);
 etch_project_bind_preset_dsp(&project,&bank);
 etch_project_bind_preset_dsp(&decoded,&read_bank);
 etch_perf_init(&bank.performance);etch_key_groups_init(&bank.groups);
 for(uint8_t i=0u;i<ETCH_PATCH_SLOTS;i++){
  bank.slot[i].spectral_part=2u;bank.slot[i].spectral_gain=50u;
  bank.slot[i].spectral_tilt=50u;bank.slot[i].blend_a=i;bank.slot[i].blend_b=i;
 }
 lib.valid[0]=1u;
 assert(ru_chain_insert(&project.v190.chain,0u,RU_FX_DISTORTION));
 assert(ru_chain_insert(&project.v190.chain,7u,RU_FX_WAH));
 project.global_fx.folder_fold=32123u;
 project.global_fx.folder_symmetry=4444u;
 project.global_fx.folder_tone=11111u;
 project.global_fx.folder_mix=23456u;
 project.global_fx.wah_sens=234u;
 project.global_fx.wah_attack=12345u;
 project.global_fx.wah_reso=32000u;
 project.global_fx.wah_mix=22222u;
 project.source_version=35u;
 uint32_t len=0u;
 assert(etch_project_encode(&lib,&seq,&project,buf,sizeof(buf),&len)==0);
 assert(etch_project_decode(buf,len,&read_lib,&read_seq,&decoded)==0);
 assert(decoded.source_version==35u);
 assert(memcmp(&decoded.v190,&project.v190,sizeof(project.v190))==0);
 assert(decoded.global_fx.folder_fold==project.global_fx.folder_fold);
 assert(decoded.global_fx.folder_symmetry==project.global_fx.folder_symmetry);
 assert(decoded.global_fx.folder_tone==project.global_fx.folder_tone);
 assert(decoded.global_fx.folder_mix==project.global_fx.folder_mix);
 assert(decoded.global_fx.wah_sens==project.global_fx.wah_sens);
 assert(decoded.global_fx.wah_attack==project.global_fx.wah_attack);
 assert(decoded.global_fx.wah_reso==project.global_fx.wah_reso);
 assert(decoded.global_fx.wah_mix==project.global_fx.wah_mix);
 buf[len-172u]=RU_FX_TYPES; /* 106-byte PJT35 core + 66-byte V1.94 tail */
 assert(etch_project_decode(buf,len,&read_lib,&read_seq,&decoded)!=0);
 puts("PASS V1.92 PJT35: distortion/wah parameters and 8-slot order roundtrip");
 return 0;
}
