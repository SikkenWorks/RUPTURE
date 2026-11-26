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
static uint32_t inject_lane,inject_mode,inject_hit;
static uint32_t stream_publications,stream_messages;
extern uint32_t ru_fx_cpu1_run(uint32_t,uint32_t,volatile uint32_t *);

int ru_cpu1_fx_available(void){return use_cpu1;}
/* Host simulates CPU1 immediately consuming each safely published window.
 * Hardware does the same asynchronously while CPU0 synthesizes later notes. */
static void run_window(uint32_t begin,uint32_t end){
 if(!use_cpu1)return;
 volatile uint32_t progress=begin;
 uint32_t stop=end;
 if(inject_mode&&begin==inject_lane){
  /* Simulated CPU1 timeout/fault while CPU0 continues, once only.
   * No corruption INSIDE an effect sample is claimed recoverable. */
  stop=inject_mode==1u?begin:begin+17u;
  inject_hit++;
  inject_mode=0u;
  use_cpu1=0u; /* retired after the in-flight job finishes/halts */
 }
 if(stop>begin)assert(ru_fx_cpu1_run(begin,stop,&progress)==stop);
 assert(progress==stop);
 last_end=stop;committed_frames+=stop-begin;
}
void ru_cpu1_fx_send(uint32_t begin,uint32_t end){
 assert(begin==0u&&end==128u);
 stream_messages++;
 last_end=begin;
 run_window(0u,64u);
}
void ru_cpu1_fx_publish(uint32_t ready){
 assert(ready==128u);
 stream_publications++;
 if(use_cpu1)run_window(ready-64u,ready);
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

#define STRESS_BLOCKS 600u
static int16_t stress_reference[STRESS_BLOCKS][256];
static uint32_t event_seed;
static uint32_t stress_random(void){
 event_seed=event_seed*1103515245u+12345u;return event_seed;
}
static uint32_t stress_events(uint32_t block){
 uint32_t rand=stress_random();
 if((block%11u)==0u)etch_native_midi(&audio,0x90u,(uint8_t)(36u+rand%36u),100u);
 if((block%13u)==0u)etch_native_midi(&audio,0x80u,(uint8_t)(36u+rand%36u),0u);
 if((block%57u)==0u)etch_native_midi(&audio,0xb0u,123u,0u);
 /* Repeated pressed/released panel keys; engine must never strand a key. */
 return block%7u==0u?(1u<<(rand%18u)):(block%7u==1u?0u:
         (block%7u==2u?(1u<<((rand>>8u)%18u)):0u));
}
static void compare_stress(uint32_t fail_lane,uint32_t fail_mode){
 const uint8_t heavy[4]={RU_FX_MOSFET,RU_FX_DISTORTION,RU_FX_REVERB,RU_FX_SAND};
 setup(heavy,4u);use_cpu1=0u;event_seed=0x14a2b37fu;
 for(uint32_t i=0u;i<STRESS_BLOCKS;i++){
  uint32_t keys=stress_events(i);
  etch_native_render_stereo(&audio,&cfg,keys,stress_reference[i],128u);
 }
 setup(heavy,4u);use_cpu1=1u;event_seed=0x14a2b37fu;
 inject_lane=fail_lane;inject_mode=0u;inject_hit=0u;
 for(uint32_t i=0u;i<STRESS_BLOCKS;i++){
  uint32_t keys=stress_events(i);
  if(i==195u)inject_mode=fail_mode;
  etch_native_render_stereo(&audio,&cfg,keys,actual,128u);
  assert(memcmp(stress_reference[i],actual,sizeof(actual))==0&&
    "CPU1 timeout/partial sample recovery changed FX/audio PCM");
 }
 if(fail_mode)assert(inject_hit==1u);
 else assert(inject_hit==0u);
 use_cpu1=0u;inject_mode=0u;
 etch_native_midi(&audio,0xb0u,123u,0u); /* all received MIDI released */
 etch_native_render_stereo(&audio,&cfg,0u,actual,128u);
 assert(audio.mono_count==0u&&!audio.voice[0].held&&
   "MIDI/physical notes must not remain latched after ALL NOTES OFF");
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
 /* 5 x 600 real 128-frame blocks = 384k samples, with random MIDI, panel
  * edge changes and heavy FX. Inject first/second-lane zero/partial workers.
  * Verify PCM bit-for-bit equal to the CPU0-only reference after recovery. */
 compare_stress(0u,0u);
 compare_stress(0u,1u);
 compare_stress(0u,2u);
 compare_stress(64u,1u);
 compare_stress(64u,2u);
 assert(stream_messages>1000u&&stream_publications==stream_messages);
 puts("PASS V2.0.58: 100 ordered FX pairs + 6 heavy 4-FX chains, 81408 stereo frames CPU0/core2 exact");
 puts("PASS V2.0.58: 384000 stereo frames, 4 injected CPU1 watermark stalls/partials sample-exact");
 return 0;
}
