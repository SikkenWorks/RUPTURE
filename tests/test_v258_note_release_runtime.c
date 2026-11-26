/* Actual native mono renderer: panel/MIDI release and overflow recovery. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "audio_core.h"

#if ETCH_CPU1_FX_LIVE_LAB
/* Keep the real CORE2 audio renderer in fallback mode for note-only tests.
 * The separate CORE2 PCM test verifies the actual FX worker/hand-offs. */
int ru_cpu1_fx_available(void){return 0;}
void ru_cpu1_fx_send(uint32_t begin,uint32_t end){(void)begin;(void)end;}
void ru_cpu1_fx_publish(uint32_t ready){(void)ready;}
uint32_t ru_cpu1_fx_collect(void){return 0u;}
#endif

static struct etch_native_audio s;
static struct etch_native_controls c;
static int16_t stereo[ETCH_NATIVE_FRAMES*2];

static void render(uint32_t keys){
 etch_native_render_stereo(&s,&c,keys,stereo,ETCH_NATIVE_FRAMES);
}
static void setup(void){
 memset(&c,0,sizeof(c));
 etch_fx_defaults(&c.fx_settings);
 c.mono_mode=1u;
 c.gain=16000u;c.cutoff=18000u;c.sustain_level=20000u;
 c.attack_step=0x40000000u;c.decay_step=0x00008800u;
 c.release_step=0x00008800u;
 etch_native_reset(&s);
}
int main(void){
 setup();
 render(1u<<5); /* physical key */
 assert(s.mono_count==1u&&s.mono_stack[0].id==5u);
 assert(s.voice[0].held);
 etch_native_midi(&s,0x90u,60u,110u); /* USB/TRS note over held panel */
 assert(s.mono_count==2u&&s.mono_stack[1].id==188u);
 etch_native_midi_panic(&s); /* lost MIDI packet or unplug */
 assert(s.mono_count==1u&&s.mono_stack[0].id==5u);
 assert(s.voice[0].held&&s.voice[0].note==5u);
 render(0u); /* real key released after MIDI overflow */
 assert(s.mono_count==0u&&!s.voice[0].held);

 setup();
 etch_native_midi(&s,0x90u,63u,100u);
 assert(s.mono_count==1u);
 etch_native_midi(&s,0x90u,63u,0u); /* velocity-zero Note OFF */
 assert(s.mono_count==0u&&!s.voice[0].held);
 etch_native_midi(&s,0x90u,63u,100u);
 etch_native_midi(&s,0x80u,63u,50u); /* explicit Note OFF */
 assert(s.mono_count==0u&&!s.voice[0].held);

 setup();
 etch_native_midi(&s,0x90u,65u,100u);
 etch_native_midi(&s,0xb0u,64u,127u); /* sustain pressed */
 etch_native_midi(&s,0x80u,65u,0u);
 assert(s.mono_count==1u&&!s.mono_stack[0].down);
 assert(s.voice[0].held);
 etch_native_midi(&s,0xb0u,64u,0u); /* pedal released */
 assert(s.mono_count==0u&&!s.voice[0].held);

 setup();
 render(1u<<3);
 assert(s.mono_count==1u&&s.voice[0].held);
 /* Simulate a lost panel-edge tracking update: physical key is already up,
  * yet software still remembers its prior key bit. Level reconciliation
  * must drop the orphan before checking the edge bitmap. */
 s.keys=0u;
 render(0u);
 assert(s.mono_count==0u&&!s.voice[0].held);

 /* A corrupted note-stack count can arrive before the next render block.
  * MIDI CC120/CC121 must never loop past the six-note allocation, even
  * when the physical-key reconciliation has not run yet. */
 setup();
 s.mono_count=ETCH_MONO_STACK+1u;
 s.voice[0].active=s.voice[0].held=s.voice[0].down=1u;
 etch_native_midi(&s,0xb0u,120u,0u);
 assert(s.mono_count==0u&&!s.voice[0].held&&!s.voice[0].down);
 setup();
 s.mono_count=255u;
 s.voice[0].active=s.voice[0].held=s.voice[0].down=1u;
 etch_native_midi(&s,0xb0u,121u,0u);
 assert(s.mono_count==0u&&!s.voice[0].held&&!s.voice[0].down);

 puts("PASS V2.0.58 actual native renderer: panel+MIDI panic, sustain, missed edges and corrupt-count CC recovery");
 return 0;
}
