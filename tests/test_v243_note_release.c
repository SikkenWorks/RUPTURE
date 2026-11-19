/* An actual release must win over stale mono-stack data; no MIDI sustain loss. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "audio_core.h"
#include "rupture_note_release_guard.h"

static struct etch_native_audio state;
static void put(uint8_t slot,uint16_t id){
 state.mono_stack[slot].id=id;
 state.mono_stack[slot].down=1u;
 state.mono_stack[slot].pitch=(uint8_t)(id&127u);
 state.mono_stack[slot].gate_left=0u;
}
int main(void){
 assert(ru_midi_capture_attack(1u,0x90u,100u));
 assert(!ru_midi_capture_attack(1u,0x90u,0u)); /* NOTE ON 0 = OFF */
 assert(!ru_midi_capture_attack(1u,0x80u,20u)); /* explicit OFF */
 assert(!ru_midi_capture_attack(1u,0xb0u,123u)); /* ALL NOTES OFF */
 assert(!ru_midi_capture_attack(0u,0x90u,100u));

 memset(&state,0,sizeof(state));
 state.mono_count=3u;
 put(0,4u);put(1,128u+60u);put(2,4096u+48u);
 assert(!ru_mono_panel_reconcile(&state,1u<<4));
 assert(state.mono_count==3u);
 assert(ru_mono_panel_reconcile(&state,0u));
 assert(state.mono_count==2u);
 assert(state.mono_stack[0].id==188u); /* live/sustained MIDI */
 assert(state.mono_stack[1].id==4144u); /* sequencer gate */
 assert(!ru_mono_panel_reconcile(&state,0u));

 memset(&state,0,sizeof(state));
 state.mono_count=3u;put(0,7u);put(1,7u);put(2,8u);
 assert(ru_mono_panel_reconcile(&state,(1u<<7)|(1u<<8)));
 assert(state.mono_count==2u);
 assert(state.mono_stack[0].id==7u&&state.mono_stack[1].id==8u);

 /* A USB/TRS overflow is NOT a panic of the physical keyboard or sequencer.
  * MIDI keys are dropped even if sustain was pressed; real keys survive. */
 memset(&state,0,sizeof(state));
 state.mono_count=4u;
 put(0,6u);put(1,128u+60u);put(2,4096u+45u);put(3,128u+128u+64u);
 ru_mono_midi_panic(&state);
 assert(state.mono_count==2u);
 assert(state.mono_stack[0].id==6u);
 assert(state.mono_stack[1].id==4141u);
 ru_mono_midi_panic(&state);
 assert(state.mono_count==2u);
 assert(ru_mono_panel_reconcile(&state,0u));
 assert(state.mono_count==1u&&state.mono_stack[0].id==4141u);

 memset(&state,0,sizeof(state));
 state.mono_count=2u;put(0,128u+59u);put(1,128u+60u);
 ru_mono_midi_panic(&state);
 assert(state.mono_count==0u);

 /* An invalid stack count must never index past the six-note buffer. */
 memset(&state,0,sizeof(state));
 state.mono_count=255u;state.voice[0].active=1u;
 state.voice[0].held=state.voice[0].down=1u;
 assert(ru_mono_panel_reconcile(&state,0u));
 assert(state.mono_count==0u&&!state.voice[0].held&&!state.voice[0].down);

 /* All real physical keys can remain held indefinitely. No age timeout. */
 memset(&state,0,sizeof(state));
 state.mono_count=6u;
 uint32_t keys=0u;
 for(uint8_t i=0u;i<6u;i++){put(i,(uint16_t)i);keys|=1u<<i;}
 for(uint32_t block=0u;block<150000u;block++)
  assert(!ru_mono_panel_reconcile(&state,keys));
 assert(state.mono_count==6u);
 puts("PASS V2.0.43: MIDI capture releases never dropped; orphan panel notes/duplicates repaired; sustain and long holds retained");
 return 0;
}
