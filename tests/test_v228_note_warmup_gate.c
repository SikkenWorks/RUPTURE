#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_warmup_gate.h"

int main(void){
 const uint32_t octaves[]={1u,0x7ffffffu,0x1000u,0x00400000u};
 for(unsigned i=0;i<sizeof(octaves)/sizeof(octaves[0]);i++){
  uint32_t keys=octaves[i];
  assert(ru_note_keys_while_output_gated(keys,0u)==0u);
  assert(ru_note_keys_while_output_gated(keys,1u)==keys);
 }
 /* Held note: no invisible early onset, then real key edge at unmute. */
 uint32_t old_keys=0u;
 uint32_t gated=ru_note_keys_while_output_gated(1u<<6,0u);
 assert(gated==old_keys);
 uint32_t audible=ru_note_keys_while_output_gated(1u<<6,1u);
 assert((audible&~old_keys)==(1u<<6));
 assert(ru_note_keys_while_output_gated(0u,1u)==0u);
 puts("PASS V2.0.28 no swallowed first held note during startup mute");
 return 0;
}
