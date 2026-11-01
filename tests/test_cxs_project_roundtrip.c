#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/wave_store.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_patch_state.h"
static struct etch_wave_library original, restored;
static struct etch_native_pattern pattern, restored_pattern;
static struct etch_project_settings cfg, restored_cfg;
static uint8_t bytes[ETCH_PROJECT_MAX_BYTES];
int main(void){
 memset(&original,0,sizeof(original));memset(&cfg,0,sizeof(cfg));
 memset(&pattern,0,sizeof(pattern));
 cfg.seq_length=16;cfg.seq_gate=75;cfg.tempo=120;
 etch_fx_defaults(&cfg.global_fx);
 original.valid[0]=1u;
 original.selection[0]=original.selection[1]=original.selection[2]=0u;
 struct etch_native_patch *p=&original.patches[0];
 memcpy(p->name,"CXS BASS",8);p->seq_length=16;p->tempo=120;
 ru_patch_cxs_save(p,79,15,83,96);
 uint32_t size=0;
 assert(etch_project_encode(&original,&pattern,&cfg,bytes,sizeof(bytes),&size)==0);
 assert(size>64u && size<sizeof(bytes));
 assert(etch_project_decode(bytes,size,&restored,&restored_pattern,&restored_cfg)==0);
 uint8_t a=0,b=0,c=0,d=0;
 assert(ru_patch_cxs_load(&restored.patches[0],&a,&b,&c,&d));
 assert(a==79&&b==15&&c==83&&d==96);
 puts("PASS full PJT32 encode/decode persists RUPTURE CXS preset parameters");
}
