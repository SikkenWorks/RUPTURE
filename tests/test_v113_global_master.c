/* SPDX-License-Identifier: GPL-3.0-only
 * Global master volume: torn writes, project migration and log compaction.
 */
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/wave_store.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_patch_state.h"
#define N (1024u*1024u)
static uint8_t flashbuf[N];
static int fail_after=-1,writes;
static struct etch_wave_library l,out;
static struct etch_native_pattern p,op;
static struct etch_project_settings cfg,ocfg;
static struct etch_preset_dsp_bank dsp,odsp;
static int rd(uint32_t a,void *d,uint32_t n){if(a>N||n>N-a)return -1;memcpy(d,flashbuf+a,n);return 0;}
static int erase(uint32_t a){if((a&4095u)||a>N-4096u)return -1;memset(flashbuf+a,255,4096u);return 0;}
static int wr(uint32_t a,const void *p,uint32_t n){
 if(fail_after>=0&&writes++>=fail_after)return -1;
 if(a>N||n>N-a||n>256||((a&255u)+n)>256)return -1;
 const uint8_t *src=p;for(uint32_t i=0;i<n;i++)if((flashbuf[a+i]&src[i])!=src[i])return -1;
 for(uint32_t i=0;i<n;i++)flashbuf[a+i]&=src[i];return 0;
}
static const struct etch_store_io io={rd,erase,wr};
static void make_project(uint8_t id){
 memset(&l,0,sizeof(l));memset(&p,0,sizeof(p));memset(&cfg,0,sizeof(cfg));memset(&dsp,0,sizeof(dsp));
 cfg.tempo=(uint8_t)(110+id);cfg.seq_length=16;cfg.seq_gate=75;cfg.source_version=32;
 etch_project_bind_preset_dsp(&cfg,&dsp);etch_fx_defaults(&cfg.global_fx);
 l.valid[0]=1;ru_patch_cxs_save(&l.patches[0],10+id,50,60,70);
 for(unsigned i=0;i<ETCH_PATCH_SLOTS;i++){dsp.slot[i].spectral_part=2;dsp.slot[i].spectral_gain=50;dsp.slot[i].spectral_tilt=50;}
 etch_perf_init(&dsp.performance);etch_key_groups_init(&dsp.groups);
}
static void save_project(uint8_t id){
 etch_store_project_select(id);make_project(id);
 char title[12]={'P','R','O','J','E','C','T',' ',(char)('0'+id/10),(char)('0'+id%10),0,0};
 etch_store_project_name(title);
 int rc=etch_store_save_project(&io,&l,&p,&cfg);if(rc)printf("save %u rc=%d\n",id,rc);assert(rc==ETCH_STORE_OK);
}
static void check_project(uint8_t id){
 memset(&out,0,sizeof(out));memset(&op,0,sizeof(op));memset(&ocfg,0,sizeof(ocfg));memset(&odsp,0,sizeof(odsp));
 etch_project_bind_preset_dsp(&ocfg,&odsp);
 assert(etch_store_load_project_slot(&io,id,&out,&op,&ocfg)==ETCH_STORE_OK);
 assert(ocfg.tempo==(uint8_t)(110+id));
}
static void volume(uint8_t target){
 assert(etch_store_global_volume_save(&io,target)==ETCH_STORE_OK);
 uint8_t check=255u;assert(etch_store_global_volume_load(&io,&check)==ETCH_STORE_OK);assert(check==target);
}
int main(void){
 memset(flashbuf,255,sizeof(flashbuf));save_project(0);save_project(1);
 uint8_t value=0;assert(etch_store_global_volume_load(&io,&value)==ETCH_STORE_OK&&value==100);
 volume(173);check_project(0);check_project(1);
 /* A project save swaps banks but must carry global volume in its metadata. */
 save_project(2);assert(etch_store_global_volume_load(&io,&value)==ETCH_STORE_OK&&value==173);
 volume(0);volume(200);
 /* A power-cut during the small record write retains the previous value. */
 fail_after=0;writes=0;assert(etch_store_global_volume_save(&io,88)==ETCH_STORE_IO);
 fail_after=-1;assert(etch_store_global_volume_load(&io,&value)==ETCH_STORE_OK&&value==200);
 volume(88);check_project(0);check_project(1);check_project(2);
 /* Fill the 28-entry journal; the next change atomically compacts the
  * existing project bank into the other bank with its payload intact. */
 for(int i=0;i<34;i++)volume((uint8_t)(30+i));
 check_project(0);check_project(1);check_project(2);
 /* Another torn global journal write is recoverable; the A/B project
  * catalog remains accessible even if a compacted bank was incomplete. */
 fail_after=0;writes=0;assert(etch_store_global_volume_save(&io,199)==ETCH_STORE_IO);
 fail_after=-1;assert(etch_store_global_volume_load(&io,&value)==ETCH_STORE_OK&&value==63);
 volume(199);save_project(3);check_project(3);
 assert(etch_store_global_volume_load(&io,&value)==ETCH_STORE_OK&&value==199);
 puts("PASS V1.13 device-global gain 0..200, project switching, torn write, 28-entry journal, verified A/B compaction");
 return 0;
}
