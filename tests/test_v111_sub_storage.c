/* RUPTURE V1.11: per-preset dual SUB persistence and strict codec validation. */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_patch_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_shape_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_sub_state.h"
static struct etch_wave_library lib,restored;
static struct etch_native_pattern pattern,restored_pattern;
static struct etch_project_settings cfg,got;
static struct etch_preset_dsp_bank dsp,loaded;
static uint8_t bytes[ETCH_PROJECT_MAX_BYTES];
static void initialize(void){
 memset(&lib,0,sizeof(lib));memset(&restored,0,sizeof(restored));
 memset(&cfg,0,sizeof(cfg));memset(&got,0,sizeof(got));
 memset(&dsp,0,sizeof(dsp));memset(&loaded,0,sizeof(loaded));
 memset(&pattern,0,sizeof(pattern));memset(&restored_pattern,0,sizeof(restored_pattern));
 etch_project_bind_preset_dsp(&cfg,&dsp);
 etch_project_bind_preset_dsp(&got,&loaded);
 cfg.tempo=120;cfg.seq_length=16;cfg.seq_gate=75;cfg.source_version=32u;
 etch_fx_defaults(&cfg.global_fx);
 lib.valid[0]=1u;lib.selection[0]=lib.selection[1]=lib.selection[2]=0;
 for(uint8_t i=0;i<ETCH_PATCH_SLOTS;i++){
  dsp.slot[i].spectral_part=2u;
  dsp.slot[i].spectral_gain=50u;dsp.slot[i].spectral_tilt=50u;
  dsp.slot[i].blend_a=i;dsp.slot[i].blend_b=i;
 }
 etch_perf_init(&dsp.performance);etch_key_groups_init(&dsp.groups);
}
static void check(uint8_t index,const uint8_t type[2],const uint8_t oct[2],const uint8_t detune[2],const uint8_t drift[2]){
 uint8_t t[2],o[2],d[2],r[2];
 if(!ru_sub_state_restore(&loaded.slot[index],t,o,d,r)){
 const struct etch_preset_dsp *v=&loaded.slot[index];
 fprintf(stderr,"FAIL idx=%u tag=%02X matrix=%u,%u,%u %u,%u,%u %u,%u,%u %u,%u,%u living=%u,%u,%u,%u evolution=%u,%u,%u,%u\n",index,v->spectral_wave,
 v->matrix.route[0].source,v->matrix.route[0].target,v->matrix.route[0].amount,
 v->matrix.route[1].source,v->matrix.route[1].target,v->matrix.route[1].amount,
 v->matrix.route[2].source,v->matrix.route[2].target,v->matrix.route[2].amount,
 v->matrix.route[3].source,v->matrix.route[3].target,v->matrix.route[3].amount,
 v->living.depth,v->living.rate,v->living.color,v->living.vary,
 v->evolution.depth,v->evolution.rate,v->evolution.mode,v->evolution.sync);
 assert(0);
}
 for(uint8_t i=0;i<2u;i++){
  assert(t[i]==type[i] && o[i]==oct[i]);
  assert(d[i]==detune[i] && r[i]==drift[i]);
 }
 assert(!etch_matrix_active(&loaded.slot[index].matrix));
 assert(!loaded.slot[index].living.depth && !loaded.slot[index].evolution.depth);
}
int main(void){
 initialize();
 uint8_t t0[2]={1,2},o0[2]={0,3},d0[2]={25,84},r0[2]={33,92};
 uint8_t t1[2]={2,1},o1[2]={1,2},d1[2]={50,54},r1[2]={0,100};
 ru_patch_cxs_save(&lib.patches[0],35,48,60,70);
 ru_patch_cxs_save(&lib.patches[1],45,58,50,80);
 ru_shape_store(&dsp.slot[0],10,50,28,31);
 ru_sub_state_store(&dsp.slot[0],t0,o0,d0,r0);
 ru_shape_store(&dsp.slot[1],20,50,12,36);
 ru_sub_state_store(&dsp.slot[1],t1,o1,d1,r1);
 uint32_t len=0;
 assert(etch_project_encode(&lib,&pattern,&cfg,bytes,sizeof(bytes),&len)==0);
 assert(etch_project_decode(bytes,len,&restored,&restored_pattern,&got)==0);
 check(0,t0,o0,d0,r0);check(1,t1,o1,d1,r1);
 assert(sizeof(struct etch_preset_dsp)==33u);
 assert((loaded.slot[0].spectral_wave&0xc0u)==0xc0u);
 uint8_t w,e,b,p;
 assert(ru_shape_restore(&loaded.slot[0],&w,&e,&b,&p));
 assert(w==10&&e==50&&b==28&&p==31);
 /* A project saved before V1.11 keeps SUBs OFF even after other projects. */
 dsp.slot[0].spectral_wave&=(uint8_t)~RU_SUB_STATE_TAG;
 assert(etch_project_encode(&lib,&pattern,&cfg,bytes,sizeof(bytes),&len)==0);
 assert(etch_project_decode(bytes,len,&restored,&restored_pattern,&got)==0);
 uint8_t t[2]={2,2},o[2]={3,3},d[2]={100,100},r[2]={100,100};
 assert(!ru_sub_state_restore(&loaded.slot[0],t,o,d,r));
 assert(t[0]==0&&t[1]==0&&o[0]==1&&o[1]==1&&d[0]==50&&d[1]==54);
 /* Tagged records with a malformed sub state must be rejected. */
 dsp.slot[0].spectral_wave|=RU_SUB_STATE_TAG;
 dsp.slot[0].matrix.route[0].source=2;
 assert(etch_project_encode(&lib,&pattern,&cfg,bytes,sizeof(bytes),&len)==-1);
 dsp.slot[0].matrix.route[0].source=0;
 /* Can't place the sub marker on old/foreign firmware presets. */
 lib.patches[0].valid=1u;
 assert(etch_project_encode(&lib,&pattern,&cfg,bytes,sizeof(bytes),&len)==-1);
 puts("PASS dual SUB1/SUB2 per-preset roundtrip, codec validation, defaults and sound shape coexistence");
 return 0;
}
