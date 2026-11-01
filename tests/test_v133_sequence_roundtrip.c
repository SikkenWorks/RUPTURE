/* RUPTURE V1.33: sequence archive and per-slot BPM/length survive the unchanged
 * project serializer. Legacy factory placeholders never mask real user notes. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_patch_state.h"
static struct etch_wave_library l,out;
static struct etch_project_settings cfg,oldcfg;
static struct etch_native_pattern live,loaded,top;
static struct etch_preset_dsp_bank dsp,rdsp;
static uint8_t blob[ETCH_PROJECT_MAX_BYTES];
static void note(uint8_t key,uint8_t vel){
 etch_pattern_clear(&live);
 live.steps[0].count=1;
 live.steps[0].notes[0].note=key;
 live.steps[0].notes[0].velocity=vel;
 live.steps[0].notes[0].length=2;
 live.steps[3].count=1;
 live.steps[3].notes[0].note=(uint8_t)(key+5);
 live.steps[3].notes[0].velocity=100;
 live.steps[3].notes[0].length=1;
}
int main(void){
 memset(&l,0,sizeof(l));memset(&cfg,0,sizeof(cfg));
 cfg.source_version=33;cfg.seq_length=16;cfg.seq_gate=75;cfg.tempo=120;
 etch_fx_defaults(&cfg.global_fx);
 etch_project_bind_preset_dsp(&cfg,&dsp);
 etch_project_bind_preset_dsp(&oldcfg,&rdsp);
 for(unsigned k=0;k<ETCH_PATCH_SLOTS;k++){dsp.slot[k].spectral_part=2;dsp.slot[k].spectral_gain=50;dsp.slot[k].spectral_tilt=50;}
 etch_perf_init(&dsp.performance);etch_key_groups_init(&dsp.groups);
 l.valid[0]=1u;l.reserved=1u;
 ru_patch_cxs_save(&l.patches[0],35,40,55,60);
 memcpy(l.patches[0].name,"BASS ONE",8);
 note(36,111);
 assert(etch_pattern_bank_capture(&cfg.patterns,24,&live)==0);
 l.patches[24]=l.patches[0];memcpy(l.patches[24].name,"FIRST",5);
 l.patches[24].valid|=0x40u;
 l.patches[24].seq_length=4;l.patches[24].tempo=92;
 note(42,99);
 assert(etch_pattern_bank_capture(&cfg.patterns,25,&live)==0);
 l.patches[25]=l.patches[0];memcpy(l.patches[25].name,"SECOND",6);
 l.patches[25].valid|=0x40u;
 l.patches[25].seq_length=8;l.patches[25].tempo=167;
 /* Last live pattern is different from both archived sequences. */
 etch_pattern_clear(&top);
 uint32_t bytes=0;
 int erc=etch_project_encode(&l,&top,&cfg,blob,sizeof(blob),&bytes);
 if(erc)printf("encode rc=%d\n",erc);fflush(stdout);assert(erc==0);
 assert(etch_project_decode(blob,bytes,&out,&loaded,&oldcfg)==0);
 assert(etch_pattern_bank_fetch(&oldcfg.patterns,24,&loaded)==1);
 assert(loaded.steps[0].count==1 && loaded.steps[0].notes[0].note==36);
 assert(loaded.steps[3].notes[0].note==41);
 assert((out.patches[24].valid&0x40u) && out.patches[24].seq_length==4 && out.patches[24].tempo==92);
 assert(etch_pattern_bank_fetch(&oldcfg.patterns,25,&loaded)==1);
 assert(loaded.steps[0].count==1 && loaded.steps[0].notes[0].note==42);
 assert(loaded.steps[3].notes[0].note==47);
 assert((out.patches[25].valid&0x40u) && out.patches[25].seq_length==8 && out.patches[25].tempo==167);
 /* Duplicating an archive must retain the metadata marker and pattern. */
 assert(etch_pattern_bank_copy_slot(&oldcfg.patterns,24,26)==0);
 out.patches[26]=out.patches[24];
 assert(etch_pattern_bank_fetch(&oldcfg.patterns,26,&loaded)==1);
 assert(loaded.steps[0].notes[0].note==36 && (out.patches[26].valid&0x40u));
 puts("PASS V1.33 two independent patterns, 4/8 steps, 92/167 BPM, project codec and duplicate");
 return 0;
}
