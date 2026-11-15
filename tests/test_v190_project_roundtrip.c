#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
static struct etch_wave_library lib,decoded;
static struct etch_project_settings project,loaded;
static struct etch_native_pattern seq,seq_loaded;
static struct etch_preset_dsp_bank dsp,dsp_loaded;
static uint8_t packet[ETCH_PROJECT_MAX_BYTES];
static uint32_t save(uint8_t version){
 uint32_t size=0;project.source_version=version;
 assert(etch_project_encode(&lib,&seq,&project,packet,sizeof(packet),&size)==0);
 return size;
}
int main(void){
 project.seq_length=16;project.seq_gate=75;project.tempo=120;
 etch_fx_defaults(&project.global_fx);ru_fxm_defaults(&project.rupture_fx);
 rupture_eq2_defaults(&project.eq2,1u);ru_v190_defaults(&project.v190);
 etch_project_bind_preset_dsp(&project,&dsp);etch_project_bind_preset_dsp(&loaded,&dsp_loaded);
 etch_perf_init(&dsp.performance);etch_key_groups_init(&dsp.groups);
 for(uint8_t i=0;i<ETCH_PATCH_SLOTS;i++){
  dsp.slot[i].spectral_part=2;dsp.slot[i].spectral_gain=50;
  dsp.slot[i].spectral_tilt=50;dsp.slot[i].blend_a=i;dsp.slot[i].blend_b=i;
 }
 lib.valid[0]=1u;
 uint32_t old_size=save(34);
 assert(etch_project_decode(packet,old_size,&decoded,&seq_loaded,&loaded)==0);
 assert(loaded.source_version==34 && ru_chain_valid(&loaded.v190.chain));
 assert(loaded.v190.chain.type[0]==RU_FX_EMPTY);
 assert(ru_chain_insert(&project.v190.chain,0,RU_FX_DELAY));
 assert(ru_chain_insert(&project.v190.chain,5,RU_FX_MOSFET));
 ru_chain_toggle(&project.v190.chain,5);
 project.v190.glide_patch[0][0]=199u;
 project.v190.glide_patch[0][1]=0x16u; /* always, exp, retrigger */
 project.v190.glide_patch[22][0]=17u;
 ru_slide_set(&project.v190,3u,1u);
 ru_slide_set(&project.v190,127u,1u);
 uint32_t size=save(35);
 assert(size==old_size+106u);
 assert(packet[0]=='P'&&packet[1]=='J'&&packet[2]=='T'&&packet[3]=='S');
 assert(etch_project_decode(packet,size,&decoded,&seq_loaded,&loaded)==0);
 assert(loaded.source_version==35u);
 assert(memcmp(&project.v190,&loaded.v190,sizeof(project.v190))==0);
 packet[size-106u]=RU_FX_TYPES; /* reject unknown FX id */
 assert(etch_project_decode(packet,size,&decoded,&seq_loaded,&loaded)!=0);
 assert(ru_chain_insert(&project.v190.chain,1u,RU_FX_DELAY));
 size=save(35); /* V1.93: repeated processor IDs are valid project data */
 assert(etch_project_decode(packet,size,&decoded,&seq_loaded,&loaded)==0);
 assert(loaded.v190.chain.type[0]==RU_FX_DELAY&&loaded.v190.chain.type[1]==RU_FX_DELAY);
 size=save(35);assert(etch_project_decode(packet,size-1u,&decoded,&seq_loaded,&loaded)!=0);
 puts("PASS PJT35: full FX order/enables, per-patch glide, step slide, legacy PJT34");
 return 0;
}
