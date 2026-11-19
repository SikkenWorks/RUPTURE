/* V2.0.39: note edges and sustained 3+ FX notes prioritize audio over LCD. */
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
 ru_note_paint_changed(&budget,201u);
 assert(budget.burst==1u);
 assert(!ru_note_paint_allow_fx(&budget,201u,0u,0u,0u,3u));
 assert(!ru_note_paint_allow_fx(&budget,264u,0u,0u,0u,3u));
 assert(ru_note_paint_allow_fx(&budget,265u,0u,0u,0u,3u));
 assert(ru_note_paint_allow_fx(&budget,202u,200u,1u,32u,3u));
 assert(ru_note_paint_allow_fx(&budget,202u,0u,0u,0u,2u));
 ru_note_paint_committed(&budget);
 assert(ru_note_paint_allow_fx(&budget,205u,0u,0u,0u,3u));
 assert(!ru_note_paint_allow_fx_sustain(&budget,205u,200u,0u,0u,3u,1u));
 assert(ru_note_paint_allow_fx_sustain(&budget,320u,200u,0u,0u,3u,1u));
 assert(ru_note_paint_allow_fx_sustain(&budget,210u,200u,1u,0u,3u,1u));
 assert(ru_note_paint_allow_fx_sustain(&budget,210u,200u,0u,0u,2u,1u));
 assert(ru_note_paint_allow_fx_sustain(&budget,210u,200u,0u,0u,3u,0u));
 ru_note_paint_changed(&budget,0xfffffff0u);
 assert(!ru_note_paint_allow_fx(&budget,0x0000000eu,0u,0u,0u,4u));
 assert(ru_note_paint_allow_fx(&budget,0x00000030u,0u,0u,0u,4u));
 chain.enabled&=(uint8_t)~(1u<<2u);
 assert(ru_note_fx_active_count(&chain,1u)==2u);
 chain.enabled|=(uint8_t)(1u<<2u);
 chain.type[2]=RU_FX_EMPTY;
 assert(ru_note_fx_active_count(&chain,1u)==2u);
 puts("PASS V2.0.39 3+ FX edge+sustain LCD guard, no sound/command gating");
 return 0;
}
