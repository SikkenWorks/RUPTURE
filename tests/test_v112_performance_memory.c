/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_performance_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_shape_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_sub_state.h"
static struct etch_wave_library lib,restored;
static struct etch_native_pattern pattern,got_pattern;
static struct etch_project_settings cfg,got;
static struct etch_preset_dsp_bank bank,loaded;
static uint8_t archive[ETCH_PROJECT_MAX_BYTES];
static void init(void){
 memset(&lib,0,sizeof(lib));memset(&restored,0,sizeof(restored));
 memset(&cfg,0,sizeof(cfg));memset(&got,0,sizeof(got));
 memset(&bank,0,sizeof(bank));memset(&loaded,0,sizeof(loaded));
 memset(&pattern,0,sizeof(pattern));memset(&got_pattern,0,sizeof(got_pattern));
 cfg.tempo=120;cfg.seq_length=16;cfg.seq_gate=75;cfg.source_version=32u;
 etch_fx_defaults(&cfg.global_fx);
 etch_project_bind_preset_dsp(&cfg,&bank);
 etch_project_bind_preset_dsp(&got,&loaded);
 lib.valid[0]=1u;
 for(unsigned i=0;i<ETCH_PATCH_SLOTS;i++){
  bank.slot[i].spectral_gain=50u;bank.slot[i].spectral_tilt=50u;
  bank.slot[i].spectral_part=2u;
  bank.slot[i].blend_a=(uint8_t)i;bank.slot[i].blend_b=(uint8_t)i;
 }
 etch_perf_init(&bank.performance);etch_key_groups_init(&bank.groups);
}
static void roundtrip(unsigned idx,uint8_t mv,uint8_t pr,uint8_t leg,uint16_t glide,uint8_t bend){
 ru_patch_cxs_save(&lib.patches[idx],58,51,47,66);
 ru_perf_store(&bank.slot[idx],mv,pr,leg,glide,bend);
 uint32_t size=0;
 assert(etch_project_encode(&lib,&pattern,&cfg,archive,sizeof(archive),&size)==0);
 assert(etch_project_decode(archive,size,&restored,&got_pattern,&got)==0);
 uint8_t v=0,p=9,l=9,b=9;uint16_t g=999;
 assert(ru_perf_restore(&loaded.slot[idx],&restored.patches[idx],&v,&p,&l,&g,&b));
 assert(v==mv&&p==pr&&l==leg&&g==glide&&b==bend);
}
int main(void){
 init();
 roundtrip(0,200,2,0,400,12);
 roundtrip(0,0,0,1,0,0);
 roundtrip(0,100,1,0,235,6);
 uint8_t one[2]={1,2},two[2]={0,3},dt[2]={55,78},dr[2]={32,94};
 ru_shape_store(&bank.slot[0],33,44,55,66);
 ru_sub_state_store(&bank.slot[0],one,two,dt,dr);
 roundtrip(0,155,2,1,175,9);
 uint8_t w,e,b,p;
 assert(ru_shape_restore(&loaded.slot[0],&w,&e,&b,&p));
 assert(w==33&&e==44&&b==55&&p==66);
 uint8_t tt[2],oo[2],dd[2],rr[2];
 assert(ru_sub_state_restore(&loaded.slot[0],tt,oo,dd,rr));
 assert(tt[0]==1&&tt[1]==2&&rr[1]==94);
 assert(!etch_matrix_active(&loaded.slot[0].matrix));
 /* Old saved RUPTURE project without marker: hard defaults. */
 bank.slot[0].blend_on=0;
 uint32_t size=0;
 assert(etch_project_encode(&lib,&pattern,&cfg,archive,sizeof(archive),&size)==0);
 assert(etch_project_decode(archive,size,&restored,&got_pattern,&got)==0);
 uint8_t m=0,pr=2,lg=0,bd=0;uint16_t gl=400;
 assert(!ru_perf_restore(&loaded.slot[0],&restored.patches[0],&m,&pr,&lg,&gl,&bd));
 assert(m==100&&pr==0&&lg==1&&gl==0&&bd==2);
 /* Corrupt tag payload must NOT change runtime sound or load stale values. */
 ru_perf_store(&bank.slot[0],200,1,0,300,10);
 bank.slot[0].spectral_gain=99;
 m=0;pr=2;lg=0;gl=400;bd=12;
 assert(!ru_perf_restore(&bank.slot[0],&lib.patches[0],&m,&pr,&lg,&gl,&bd));
 assert(m==100&&pr==0&&lg==1&&gl==0&&bd==2);
 puts("PASS V1.12 PJT32 Note Memory/Master 0..200, extremes, invalid defaults, CXS+SUB+SHAPE coexistence");
 return 0;
}
