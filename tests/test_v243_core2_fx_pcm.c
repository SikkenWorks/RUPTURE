/* Real native renderer: same samples with CPU0 FX and the CPU1 FX worker.
 * Host invokes each worker chunk immediately, without hardware concurrency.
 * This verifies AUDIO/FX PCM routing, not shared-bus or DMA timing. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "audio_core.h"

static struct etch_native_audio audio;
static struct etch_native_controls cfg;
static int16_t reference[6][256],actual[256];
static uint8_t use_cpu1;
static uint32_t last_end;
static uint32_t committed_frames;
extern uint32_t ru_fx_cpu1_run(uint32_t,uint32_t,volatile uint32_t *);

int ru_cpu1_fx_available(void){return use_cpu1;}
void ru_cpu1_fx_send(uint32_t begin,uint32_t end){
 volatile uint32_t progress=begin;
 assert(ru_fx_cpu1_run(begin,end,&progress)==end&&progress==end);
 last_end=end;committed_frames+=end-begin;
}
uint32_t ru_cpu1_fx_collect(void){return last_end;}

static void setup(const uint8_t types[4],uint32_t count){
 memset(&cfg,0,sizeof(cfg));
 etch_fx_defaults(&cfg.fx_settings);
 cfg.mono_mode=1u;cfg.gain=18000u;
 cfg.cutoff=18500u;cfg.sustain_level=26000u;
 cfg.attack_step=0x40000000u;cfg.decay_step=0x00008800u;
 cfg.release_step=0x00008800u;
 cfg.cxs_pressure=74u;cfg.cxs_balance=50u;
 cfg.cxs_memory=55u;cfg.cxs_density=63u;
 cfg.sub_type[0]=0u;cfg.sub_blend[0]=45u;
 cfg.chorus=17000u;cfg.delay=19000u;cfg.reverb=30000u;
 cfg.fx_chain_drive=21000u;cfg.fx_chain_active=1u;
 ru_chain_clear(&cfg.fx_chain);
 for(uint32_t i=0u;i<count;i++)assert(ru_chain_insert(&cfg.fx_chain,(uint8_t)i,types[i]));
 etch_native_reset(&audio);
}

static void compare(const uint8_t types[4],uint32_t count){
 const uint32_t note_keys[6]={1u,1u,1u,0u,1u,0u};
 setup(types,count);use_cpu1=0u;
 for(uint32_t b=0;b<6u;b++)
  etch_native_render_stereo(&audio,&cfg,note_keys[b],reference[b],128u);
 setup(types,count);use_cpu1=1u;committed_frames=0u;
 for(uint32_t b=0;b<6u;b++){
  etch_native_render_stereo(&audio,&cfg,note_keys[b],actual,128u);
  if(memcmp(reference[b],actual,sizeof(actual))){
   for(uint32_t f=0u;f<256u;f++)if(reference[b][f]!=actual[f]){
    fprintf(stderr,"FX mismatch on block %u sample %u: %d != %d\n",
      b,f,reference[b][f],actual[f]);break;
   }
   assert(0&&"full core2 FX must preserve CPU0 sample output");
  }
 }
 assert(committed_frames==6u*128u);
 use_cpu1=0u;
}

int main(void){
 const uint8_t sets[][4]={
  {RU_FX_MOSFET,RU_FX_DISTORTION,RU_FX_REVERB,RU_FX_SAND},
  {RU_FX_REVERB,RU_FX_DELAY,RU_FX_CHORUS,RU_FX_MOSFET},
  {RU_FX_SAND,RU_FX_FUZZ,RU_FX_WAH,RU_FX_REVERB},
  {RU_FX_COMP,RU_FX_TILT,RU_FX_DISTORTION,RU_FX_CHORUS},
  {RU_FX_DELAY,RU_FX_REVERB,RU_FX_DELAY,RU_FX_REVERB},
  {RU_FX_DISTORTION,RU_FX_SAND,RU_FX_MOSFET,RU_FX_REVERB},
 };
 for(uint32_t n=0u;n<sizeof(sets)/sizeof(sets[0]);n++)compare(sets[n],4u);
 /* Every effect as the first and second insert, including duplicate types.
  * Source-order matters because the rack has shared FX tanks and state. */
 for(uint8_t first=0u;first<RU_FX_TYPES;first++)
  for(uint8_t second=0u;second<RU_FX_TYPES;second++){
   const uint8_t pair[4]={first,second,0u,0u};
   compare(pair,2u);
  }
 puts("PASS V2.0.43: 100 ordered FX pairs + 6 heavy 4-FX chains, 81408 stereo frames CPU0/core2 exact");
 return 0;
}
