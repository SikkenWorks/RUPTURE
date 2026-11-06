/* V1.85.1 regression: exercise the real audio sequencer, not the UI playhead.
 * Compile with -DRUPTURE_BUILD=1 against the native audio engine. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/audio_core.h"
static struct etch_native_audio s;
static struct etch_native_controls c;
static struct etch_native_pattern p;
static int16_t buf[2];

static void tick_step(uint16_t expected){
 s.perf_clock=0u; /* force next step boundary while exercising the real DSP */
 etch_native_render_stereo(&s,&c,0u,buf,1u);
 if(s.perf_current_step!=expected){
  fprintf(stderr,"played %u, expected %u (LENGTH=%u)\n",s.perf_current_step,expected,c.seq_length);
  assert(s.perf_current_step==expected);
 }
 assert(s.mono_count>0u);
 assert(s.mono_stack[s.mono_count-1u].pitch == 36u+expected);
 assert(s.perf_step==(uint8_t)((expected+1u)%c.seq_length));
}
int main(void){
 memset(&c,0,sizeof c);memset(&p,0,sizeof p);
 etch_native_reset(&s);etch_fx_defaults(&c.fx_settings);
 c.mono_mode=1u;c.tempo=240u;c.seq_rate=3u;c.seq_gate=75u;
 c.gain=20000u;c.cutoff=15000u;c.sustain_level=20000u;
 c.attack_step=0x7fff0000u;c.decay_step=0x00008000u;c.release_step=0x00008000u;
 c.pattern=&p;c.seq_run=1u;c.seq_length=16u;
 for(int i=0;i<16;i++){
  p.steps[i].count=1u;
  p.steps[i].notes[0].note=(uint8_t)(36+i);
  p.steps[i].notes[0].velocity=120u;
  p.steps[i].notes[0].length=1u;
 }
 /* 16-step baseline with a unique pitch on each step. */
 for(uint16_t i=0;i<11u;i++)tick_step(i);
 assert(s.perf_step==11u);
 /* Shrink while playing: next audible step must be in 0..3 immediately. */
 c.seq_length=4u;
 for(uint16_t i=0;i<16u;i++)tick_step((uint16_t)((11u+i)%4u));
 /* Grow back: all 16 original notes are still stored, not erased. */
 c.seq_length=16u;
 for(uint16_t i=0;i<20u;i++)tick_step((uint16_t)((3u+i)%16u));
 /* A fresh 4-step pattern must never reach any of steps 4..15. */
 c.seq_run=0u;etch_native_render_stereo(&s,&c,0u,buf,1u);
 c.seq_run=1u;c.seq_length=4u;
 for(uint16_t i=0;i<24u;i++)tick_step((uint16_t)(i%4u));
 /* Other supported lengths preserve their ordinary wrap semantics. */
 for(uint16_t length=8u;length<=128u;length=(uint16_t)(length*2u)){
  c.seq_run=0u;etch_native_render_stereo(&s,&c,0u,buf,1u);
  c.seq_run=1u;c.seq_length=length;
  for(uint16_t i=0;i<24u;i++){
   s.perf_clock=0u;
   etch_native_render_stereo(&s,&c,0u,buf,1u);
   assert(s.perf_current_step==(i%length));
  }
 }
 puts("PASS V1.85.1 real DSP: LENGTH 16->4 while playing, 4->16 notes retained, fresh length=4, 8..128 unaffected");
 return 0;
}
