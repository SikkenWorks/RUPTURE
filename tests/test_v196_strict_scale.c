/* RUPTURE V1.96: STRICT plays only pitches in the selected scale.
 * Run the real audio core, not just text/UI assertions. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "audio_core.h"
#include "native_led_policy.h"
#include "native_music_labels.h"

static struct etch_native_audio audio;
static struct etch_native_controls controls;
static int16_t pcm[256];
static int16_t guide_pcm[256];

static void setup(uint8_t mode){
 memset(&controls,0,sizeof(controls));
 etch_fx_defaults(&controls.fx_settings);
 controls.mono_mode=1u;
 controls.gain=19000u;controls.cutoff=17000u;
 controls.sustain_level=25000u;
 controls.attack_step=0x40000000u;
 controls.decay_step=0x00008800u;
 controls.release_step=0x00008800u;
 controls.cxs_pressure=74u;controls.cxs_balance=50u;
 controls.cxs_memory=55u;controls.cxs_density=63u;
 controls.harmony.root=0u;controls.harmony.mode=0u;
 controls.harmony.keylock=mode;controls.harmony.chord=0u;
 etch_native_reset(&audio);
}
static uint64_t render(uint32_t keys){
 uint64_t energy=0u;
 for(unsigned b=0u;b<5u;b++){
  etch_native_render_stereo(&audio,&controls,keys,pcm,128u);
  for(unsigned n=0u;n<256u;n++)energy+=(int64_t)pcm[n]*pcm[n];
 }
 return energy;
}
int main(void){
 struct etch_harmony_settings h={0u,0u,3u,0u};
 /* At OCT=0, key 0 is F, key 1 is F# (forbidden), key 2 is G. */
 assert(etch_harmony_note_in_scale(53u,&h));
 assert(!etch_harmony_note_in_scale(54u,&h));
 assert(etch_harmony_note_in_scale(55u,&h));
 uint32_t playable=etch_harmony_live_key_mask(0,&h);
 assert((playable&1u)&&!(playable&2u)&&(playable&4u));
 assert(etch_led_scale_guide_targets(0,&h)==playable);
 h.keylock=1u;
 assert(etch_led_scale_guide_targets(0,&h)==playable);
 h.keylock=2u;
 assert(!etch_led_scale_guide_targets(0,&h));
 assert(etch_lock_step(0u,1)==1u);
 assert(etch_lock_step(1u,1)==3u);
 assert(etch_lock_step(3u,1)==2u);
 assert(etch_lock_step(2u,-1)==3u);
 assert(etch_lock_step(3u,-1)==1u);
 assert(!strcmp(etch_lock_name(3u),"STRICT"));

 setup(3u);
 assert(render(1u<<1)==0u); /* F# physically pressed: exactly silent */
 assert(audio.mono_count==0u);
 assert(render(1u<<2)>0u); /* G is in the scale: real oscillator sounds */
 assert(audio.mono_count==1u);
 render(0u);assert(audio.mono_count==0u);

 setup(1u);assert(render(1u<<1)>0u); /* GUIDE still plays F# */
 assert(audio.mono_count==1u);
 setup(2u);assert(render(1u<<1)>0u); /* Historic LOCK still quantizes */
 assert(audio.mono_count==1u);
 /* An allowed note must produce exactly the same stereo PCM in GUIDE and
  * STRICT. This verifies that neither oscillator nor FX sound was altered. */
 setup(1u);uint64_t guide_energy=render(1u<<2);
 memcpy(guide_pcm,pcm,sizeof(pcm));
 setup(3u);uint64_t strict_energy=render(1u<<2);
 assert(strict_energy==guide_energy);
 assert(memcmp(guide_pcm,pcm,sizeof(pcm))==0);
 setup(3u);
 render(0u); /* publish harmony before an external MIDI event */
 etch_native_midi(&audio,0x90u,54u,111u);
 assert(audio.mono_count==0u); /* F# MIDI note-on is silent too */
 etch_native_midi(&audio,0x80u,54u,0u); /* note-off remains safe */
 etch_native_midi(&audio,0x90u,55u,111u);
 assert(audio.mono_count==1u);
 etch_native_midi(&audio,0x80u,55u,0u);
 assert(audio.mono_count==0u);
 puts("PASS V1.96 OFF/GUIDE/STRICT/LOCK order, LEDs, real mono PCM and MIDI note gating");
 return 0;
}
