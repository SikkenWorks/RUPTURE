/* RUPTURE V1.4: shape persistence tests, including backward defaults and
 * full PJT32 encode/decode, no flash writes performed. */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/wave_store.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_patch_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_shape_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_main_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_sub_state.h"
static struct etch_wave_library lib,restored;
static struct etch_native_pattern sequence,restored_seq;
static struct etch_project_settings cfg,restored_cfg;
static struct etch_preset_dsp_bank dsp,saved_dsp;
static uint8_t data[ETCH_PROJECT_MAX_BYTES];
int main(void){
 uint8_t w,e,b,p;
 struct etch_preset_dsp v={0};
 ru_shape_restore(&v,&w,&e,&b,&p);
 assert(w==0&&e==50&&b==0&&p==0);
 ru_shape_store(&v,21,75,39,60);
 assert(ru_shape_restore(&v,&w,&e,&b,&p));
 assert(w==21&&e==75&&b==39&&p==60);
 /* Other reserved bits remain invalid in the project codec. */
 assert(v.spectral_wave==RU_SHAPE_TAG);
 ru_shape_store(&v,255,255,255,255);
 assert(ru_shape_restore(&v,&w,&e,&b,&p));
 assert(w==100&&e==100&&b==100&&p==100);
 memset(&lib,0,sizeof(lib));memset(&cfg,0,sizeof(cfg));
 memset(&sequence,0,sizeof(sequence));memset(&dsp,0,sizeof(dsp));
 memset(&saved_dsp,0,sizeof(saved_dsp));
 etch_project_bind_preset_dsp(&cfg,&dsp);
 etch_project_bind_preset_dsp(&restored_cfg,&saved_dsp);
 cfg.tempo=120;cfg.seq_length=16;cfg.seq_gate=75;cfg.source_version=32u;
 etch_fx_defaults(&cfg.global_fx);
 lib.valid[0]=1u;
 lib.selection[0]=lib.selection[1]=lib.selection[2]=0;
 ru_patch_cxs_save(&lib.patches[0],30,40,50,60);
 lib.patches[0].tempo=120;lib.patches[0].seq_length=16;
 for(uint8_t i=0;i<ETCH_PATCH_SLOTS;i++){
  dsp.slot[i].spectral_part=2u;
  dsp.slot[i].spectral_gain=50u;
  dsp.slot[i].spectral_tilt=50u;
  dsp.slot[i].blend_a=i;dsp.slot[i].blend_b=i;
 }
 etch_perf_init(&dsp.performance);
 etch_key_groups_init(&dsp.groups);
 ru_shape_store(&dsp.slot[0],27,63,49,84);
 uint8_t tt[2]={3u,4u},oo[2]={1u,0u},dd[2]={55u,45u},rr[2]={20u,30u};
 ru_sub_state_store(&dsp.slot[0],tt,oo,dd,rr);
 ru_main_state_store(&dsp.slot[0],1u);
 uint32_t len=0;
 int ec=etch_project_encode(&lib,&sequence,&cfg,data,sizeof(data),&len);
 if(ec){printf("encode error %d\n",ec);return 2;}
 ec=etch_project_decode(data,len,&restored,&restored_seq,&restored_cfg);
 if(ec){printf("decode error %d\n",ec);return 3;}
 assert(ru_patch_is_cxs(&restored.patches[0]));
 assert(ru_main_state_restore(&saved_dsp.slot[0])==1u);
 assert(ru_main_state_restore(&saved_dsp.slot[1])==0u);
 uint8_t t2[2],o2[2],d2[2],r2[2];
 assert(ru_sub_state_restore(&saved_dsp.slot[0],t2,o2,d2,r2));
 assert(t2[0]==3u&&t2[1]==4u);
 assert(ru_shape_restore(&saved_dsp.slot[0],&w,&e,&b,&p));
 assert(w==27&&e==63&&b==49&&p==84);
 assert(!ru_shape_restore(&saved_dsp.slot[1],&w,&e,&b,&p));
 assert(w==0&&e==50&&b==0&&p==0);
 /* Reserved bit5 still rejected; main SAW bit4 is accepted. */
 dsp.slot[0].spectral_wave|=0x20u;
 assert(etch_project_encode(&lib,&sequence,&cfg,data,sizeof(data),&len)!=0);
 dsp.slot[0].spectral_wave&=(uint8_t)~0x20u;
 /* A shape tag may never be grafted onto an untagged legacy preset. */
 lib.patches[0].valid=1u;
 assert(etch_project_encode(&lib,&sequence,&cfg,data,sizeof(data),&len)!=0);
 ru_patch_cxs_save(&lib.patches[0],30,40,50,60);
 /* V1.3-style project with no tag must restore neutrally. */
 dsp.slot[0].spectral_wave&=(uint8_t)~(RU_SHAPE_TAG|RU_MAIN_SAW_TAG|RU_SUB_STATE_TAG);
 ec=etch_project_encode(&lib,&sequence,&cfg,data,sizeof(data),&len);
 assert(ec==0);
 ec=etch_project_decode(data,len,&restored,&restored_seq,&restored_cfg);
 assert(ec==0);
 assert(!ru_main_state_restore(&saved_dsp.slot[0]));
 assert(!ru_shape_restore(&saved_dsp.slot[0],&w,&e,&b,&p));
 assert(w==0&&e==50&&b==0&&p==0);
 printf("PASS V1.45 CXS/SAW main + SQUARE/SAW subs PJT storage roundtrip, bit5 guard, legacy defaults\n");
 return 0;
}
