/* V1.98: GLIDE edits stage immediately and survive the same project codec
 * used for both manual SAVE and safely deferred autosave. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_policy.h"

static struct etch_wave_library lib,decoded;
static struct etch_project_settings project,loaded;
static struct etch_native_pattern seq,seq_loaded;
static struct etch_preset_dsp_bank dsp,dsp_loaded;
static uint8_t packet[ETCH_PROJECT_MAX_BYTES];

static void roundtrip(void){
 uint32_t size=0u;
 project.source_version=35u;
 assert(ru_v190_valid(&project.v190));
 assert(etch_project_encode(&lib,&seq,&project,packet,sizeof(packet),&size)==0);
 assert(etch_project_decode(packet,size,&decoded,&seq_loaded,&loaded)==0);
 assert(loaded.source_version==35u);
 assert(memcmp(project.v190.glide_patch,loaded.v190.glide_patch,
               sizeof(project.v190.glide_patch))==0);
}

int main(void){
 project.seq_length=16u;project.seq_gate=75u;project.tempo=120u;
 etch_fx_defaults(&project.global_fx);
 ru_fxm_defaults(&project.rupture_fx);
 rupture_eq2_defaults(&project.eq2,1u);
 ru_v190_defaults(&project.v190);
 etch_project_bind_preset_dsp(&project,&dsp);
 etch_project_bind_preset_dsp(&loaded,&dsp_loaded);
 etch_perf_init(&dsp.performance);
 etch_key_groups_init(&dsp.groups);
 for(uint8_t i=0u;i<ETCH_PATCH_SLOTS;i++){
  dsp.slot[i].spectral_part=2u;
  dsp.slot[i].spectral_gain=50u;dsp.slot[i].spectral_tilt=50u;
  dsp.slot[i].blend_a=i;dsp.slot[i].blend_b=i;
 }
 lib.valid[0]=1u;
 /* Edit all four GLIDE controls on preset 0. No other preset is touched. */
 assert(ru_v190_glide_stage(&project.v190,0u,750u,1u,0u,1u));
 assert(!ru_v190_glide_stage(&project.v190,0u,750u,1u,0u,1u));
 assert(project.v190.glide_patch[0][0]==75u);
 assert(project.v190.glide_patch[0][1]==0x11u);
 assert(project.v190.glide_patch[1][0]==0u);
 assert(project.v190.glide_patch[1][1]==0x10u);
 /* Edit another preset before any flash commit; slot 0 must retain edits. */
 assert(ru_v190_glide_stage(&project.v190,1u,2000u,2u,1u,0u));
 assert(project.v190.glide_patch[0][0]==75u);
 assert(project.v190.glide_patch[1][0]==200u);
 assert(project.v190.glide_patch[1][1]==0x06u);
 assert(ru_v190_glide_stage(&project.v190,31u,9000u,12u,3u,5u));
 assert(project.v190.glide_patch[31][0]==200u);
 assert(project.v190.glide_patch[31][1]==0x16u);
 assert(!ru_v190_glide_stage(&project.v190,32u,500u,1u,0u,1u));
 /* Manual SAVE: includes every edited GLIDE parameter for both presets. */
 roundtrip();
 assert(loaded.v190.glide_patch[0][0]==75u);
 assert(loaded.v190.glide_patch[1][0]==200u);
 /* Autosave trigger is deferred until safe idle; it uses the same codec. */
 assert(!rupture_autosave_due(12000u,10000u,0u,1u,1u,0u,0u,0u));
 assert(!rupture_autosave_due(40000u,10000u,0u,1u,1u,1u,0u,0u));
 assert(rupture_autosave_due(40000u,10000u,0u,1u,1u,0u,0u,0u));
 /* New four-knob edits now go through that same project save/load path. */
 assert(ru_v190_glide_stage(&project.v190,0u,1250u,2u,1u,1u));
 roundtrip();
 assert(loaded.v190.glide_patch[0][0]==125u);
 assert(loaded.v190.glide_patch[0][1]==0x16u);
 assert(loaded.v190.glide_patch[1][0]==200u);
 assert(loaded.v190.glide_patch[1][1]==0x06u);
 puts("PASS V1.98 GLIDE four-knob per-preset SAVE/autosave project roundtrip");
 return 0;
}
