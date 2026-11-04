/* Host regression V1.76: countdown-to-step-1 event loss and transport latch. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_countin.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_countin_bridge.h"
int main(void){
 volatile struct RuCountIn c={0}; struct RuMetroClick click={0};
 ru_countin_begin(&c);assert(!c.first_step_guard && !c.force_seq);
 for(uint32_t s=0;s<44100u*4u;s++)(void)ru_countin_sample(&c,&click,120);
 assert(c.beats==8u && c.active);
 assert(!ru_countin_qualifies_prepress(7u,200u,0u));
 assert(!ru_countin_qualifies_prepress(8u,4000u,0u));
 assert(ru_countin_qualifies_prepress(8u,300u,500u));
 assert(!ru_countin_qualifies_prepress(8u,2100u,500u));
 /* At downbeat, audio step 1 begins even when UI has not yet seen it. */
 while(c.active)ru_countin_sample(&c,&click,120);
 assert(c.force_seq && c.ready && c.first_step_guard);
 /* If a key was sensed around this boundary, it MUST land in step zero,
  * not in the last step of the previous loop (or the edit cursor). */
 assert(ru_countin_clamp_first_step(15u,0u,16u,c.first_step_guard)==0u);
 assert(ru_countin_clamp_first_step(0u,0u,16u,c.first_step_guard)==0u);
 assert(ru_countin_clamp_first_step(15u,1u,16u,c.first_step_guard)==15u);
 c.ready=0u;c.force_seq=0u; /* UI acknowledgment; allows transport STOP. */
 assert(!c.force_seq && c.first_step_guard);
 c.first_step_guard=0u;
 assert(ru_countin_clamp_first_step(15u,0u,16u,c.first_step_guard)==15u);
 ru_countin_cancel(&c);
 assert(!c.active && !c.ready && !c.force_seq && !c.first_step_guard);
 puts("PASS V1.76 count-in: pre-downbeat key capture; first-step quantization; transport no longer forced after acknowledgment");
 return 0;
}
