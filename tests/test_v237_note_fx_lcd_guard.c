/* V2.0.37: note edges cannot force competing LCD DMA under heavy FX. */
#include <assert.h>
#include <stdio.h>
#include "rupture_note_paint_budget.h"

int main(void){
 struct ru_note_paint_budget budget={0};
 struct rupture_fx_chain chain={0};
 assert(ru_note_fx_active_count(&chain,1u)==0u);
 for(uint8_t i=0u;i<3u;i++){
  chain.type[i]=RU_FX_REVERB;
  chain.enabled|=(uint8_t)(1u<<i);
 }
 assert(ru_note_fx_active_count(&chain,1u)==3u);
 assert(ru_note_fx_active_count(&chain,0u)==0u);
 assert(ru_note_paint_allow_fx(&budget,200u,100u,0u,0u,3u));
 /* The old burst-only guard always painted the first note. The new
  * heavy-FX guard defers it, including note release and single MIDI notes. */
 ru_note_paint_changed(&budget,201u);
 assert(budget.burst==1u);
 assert(!ru_note_paint_allow_fx(&budget,201u,0u,0u,0u,3u));
 assert(!ru_note_paint_allow_fx(&budget,264u,0u,0u,0u,3u));
 assert(ru_note_paint_allow_fx(&budget,265u,0u,0u,0u,3u));
 /* Never delay navigation, explicit editing, or an alert/forced UI. */
 assert(ru_note_paint_allow_fx(&budget,202u,200u,1u,32u,3u));
 assert(ru_note_paint_allow_fx(&budget,202u,0u,0u,0u,2u));
 ru_note_paint_committed(&budget);
 assert(ru_note_paint_allow_fx(&budget,205u,0u,0u,0u,3u));
 assert(!ru_note_paint_allow_fx_sustain(&budget,205u,200u,0u,0u,3u,1u));
 assert(ru_note_paint_allow_fx_sustain(&budget,320u,200u,0u,0u,3u,1u));
 assert(ru_note_paint_allow_fx_sustain(&budget,210u,200u,1u,0u,3u,1u));
 assert(ru_note_paint_allow_fx_sustain(&budget,210u,200u,0u,0u,2u,1u));
 assert(ru_note_paint_allow_fx_sustain(&budget,210u,200u,0u,0u,3u,0u));
 /* Exact edge timestamps, wrap-around and the normal burst policy. */
 ru_note_paint_changed(&budget,0xfffffff0u);
 assert(!ru_note_paint_allow_fx(&budget,0x0000000eu,0u,0u,0u,4u));
 assert(ru_note_paint_allow_fx(&budget,0x00000030u,0u,0u,0u,4u));
 chain.enabled&=(uint8_t)~(1u<<2u);
 assert(ru_note_fx_active_count(&chain,1u)==2u);
 chain.enabled|=(uint8_t)(1u<<2u);
 chain.type[2]=RU_FX_EMPTY;
 assert(ru_note_fx_active_count(&chain,1u)==2u);
 puts("PASS V2.0.37 3+ FX: no note-edge cosmetic LCD DMA, no sound/command gating");
 return 0;
}
