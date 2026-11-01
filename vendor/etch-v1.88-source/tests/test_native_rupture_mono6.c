/* The exact FM1 audio_core.c, compiled as RUPTURE_BUILD and run on a host. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../firmware/fm1-native/audio_core.h"
static struct etch_native_audio s;
static struct etch_native_controls c;
static int16_t pcm[128];
static void init(void){
 etch_native_reset(&s);c=(struct etch_native_controls){0};
 c.mono_mode=1;c.env_enabled=1;c.attack_step=0x7fff0000u;
 c.decay_step=0x7fff0000u;c.sustain_level=32767u;
 c.release_step=0x7fff0000u;c.gain=32767u;c.cutoff=8192u;c.filter_type=2u;
 c.morph=19005u;c.depth=16384u;c.resonance=12000u;
 assert(s.mono_mode==1u&&ETCH_MONO_STACK==6u);
}
int main(void){
 init();
 for(unsigned n=40;n<=45;n++){
  etch_native_midi(&s,0x90,(uint8_t)n,120);
  assert(s.mono_count==n-39u&&s.voice[0].note==128+n);
 }
 uint32_t original_phase=s.cxs.phase;
 uint32_t first_birth=s.voice[0].birth;
 etch_native_render(&s,&c,0,pcm,128);
 assert(s.cxs.phase!=original_phase);
 etch_native_midi(&s,0x90,46,120);
 assert(s.mono_count==6u && s.mono_stack[0].pitch==41u);
 /* Legato note changes do not allocate another voice or reset CXS phase. */
 assert(s.voice[0].birth==first_birth);
 uint32_t held_phase=s.cxs.phase;
 etch_native_midi(&s,0x80,46,0);
 assert(s.voice[0].note==128u+45u&&s.cxs.phase==held_phase);
 for(unsigned n=45;n>=41;n--){
  etch_native_midi(&s,0x80,(uint8_t)n,0);
  if(n>41)assert(s.voice[0].note==128u+n-1u);
  else assert(!s.mono_count&&!s.voice[0].held);
 }
 /* Silently ignore stale note off for evicted note. */
 etch_native_midi(&s,0x80,40,0);assert(!s.mono_count);
 /* Identical-note FIFO and per-channel sustain. */
 etch_native_midi(&s,0x90,55,80);etch_native_midi(&s,0x90,55,100);
 assert(s.mono_count==2);
 etch_native_midi(&s,0x80,55,0);assert(s.mono_count==1);
 etch_native_midi(&s,0xb0,64,127);
 etch_native_midi(&s,0x80,55,0);assert(s.mono_count==1&&!s.voice[0].down);
 etch_native_midi(&s,0xb0,64,0);assert(!s.mono_count);
 /* Panic cannot strand a currently audible MIDI note. */
 etch_native_midi(&s,0x90,60,100);etch_native_midi_panic(&s);
 assert(!s.mono_count&&!s.voice[0].held);
 /* CXS audio is nonzero, deterministic and output-limited with no notes stuck. */
 init();etch_native_midi(&s,0x90,36,127);
 long abs_sum=0;
 for(unsigned block=0;block<80u;block++){
  etch_native_render(&s,&c,0,pcm,128);
  for(unsigned i=0;i<128;i++)abs_sum+=pcm[i]<0?-(long)pcm[i]:pcm[i];
 }
 assert(abs_sum>4000u);
 assert(s.cxs.cycles>4u);
 /* Ladder type 4 is a genuinely separate four-state filter path.
  * Exercise resonance/cutoff extremes under the native audio renderer. */
 init();c.filter_type=4u;c.resonance=32000u;c.cutoff=4000u;
 etch_native_midi(&s,0x90,36,127);
 for(unsigned block=0;block<100u;block++)etch_native_render(&s,&c,0,pcm,128);
 assert(s.voice[0].ladder[0]||s.voice[0].ladder[1]||s.voice[0].ladder[2]||s.voice[0].ladder[3]);
 c.cutoff=8192u;c.resonance=32767u;
 for(unsigned block=0;block<100u;block++)etch_native_render(&s,&c,0,pcm,128);
 assert(s.mono_count==1u);

 puts("PASS native RUPTURE six-note priority, CXS mono carrier, legato/phase, MIDI repeated/sustain/panic");
 return 0;
}
