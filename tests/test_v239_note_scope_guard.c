/* V2.0.39: only optional scope capture may be shed at heavy FX note edges. */
#include <assert.h>
#include <stdio.h>
#include "rupture_note_scope_guard.h"

int main(void){
 ru_note_scope_guard_t s={0};
 const uint32_t budget=128u*544u;
 assert(ru_note_scope_capture_allow(&s,1u,0u,2u,budget,budget));
 assert(ru_note_scope_capture_allow(&s,1u,1u,2u,budget,budget));
 assert(s.quiet_halves==0u);
 assert(!ru_note_scope_capture_allow(&s,1u,1u,3u,budget,budget));
 assert(s.quiet_halves==23u);
 for(unsigned i=0;i<23u;i++)
  assert(!ru_note_scope_capture_allow(&s,1u,0u,3u,budget,budget));
 assert(s.quiet_halves==0u);
 assert(ru_note_scope_capture_allow(&s,1u,0u,3u,budget,budget));
 assert(!ru_note_scope_capture_allow(&s,1u,0u,3u,budget/5u,budget));
 assert(ru_note_scope_capture_allow(&s,1u,0u,3u,budget/5u+1u,budget));
 assert(!ru_note_scope_capture_allow(&s,1u,0u,2u,0u,budget));
 assert(!ru_note_scope_capture_allow(&s,1u,0u,0u,0u,budget));
 assert(!ru_note_scope_capture_allow(&s,0u,1u,3u,budget,budget));
 assert(s.quiet_halves==23u);
 assert(ru_note_scope_capture_allow(&s,1u,0u,2u,budget,budget));
 assert(!ru_note_scope_capture_allow(&s,0u,0u,3u,budget,budget));
 ru_note_scope_guard_t snap=s;
 assert(ru_note_scope_capture_allow(0,1u,1u,8u,0u,budget));
 assert(s.quiet_halves==snap.quiet_halves);
 puts("PASS V2.0.39 3+ FX note-edge scope suppression, low-slack policy, exact cooldown");
 return 0;
}
