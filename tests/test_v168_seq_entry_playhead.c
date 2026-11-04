/* RUPTURE V1.68: step-1 deletion, HOLD ownership, record playhead LCD sync.
 * Test actual native pattern code and actual RUPTURE LCD scheduler policy.
 * No firmware flash or hardware interaction. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_performance.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_entry.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_redraw.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_lcd_budget.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_seq_cursor.h"

int main(void){
 static struct etch_native_pattern p;
 /* Step 1 is zero-based step 0. Create/remove/re-create with the same key. */
 assert(ru_seq_key_toggle(&p,0u,53u,110u)==1u);
 assert(p.steps[0].count==1u);
 assert(ru_seq_key_toggle(&p,0u,53u,110u)==2u);
 assert(p.steps[0].count==0u);
 assert(ru_seq_key_toggle(&p,0u,53u,110u)==1u);
 assert(ru_seq_delete_selected(&p,0u)==1u);
 assert(!p.steps[0].count);
 assert(!ru_seq_delete_selected(&p,0u));
 /* Every visible step uses the same add/remove implementation. */
 for(uint16_t step=0u;step<128u;step++){
  assert(ru_seq_key_toggle(&p,step,60u,97u)==1u);
  assert(ru_seq_key_toggle(&p,step,60u,97u)==2u);
  assert(p.steps[step].count==0u);
 }
 /* Regression: old HOLD referencing the first trigger blocks legacy toggle.
  * Remove only dependents when the RUPTURE UI explicitly deletes a note. */
 assert(etch_seq_put(&p,0u,53u,90u,3u));
 assert(etch_seq_put(&p,8u,53u,90u,1u));
 assert(etch_seq_put(&p,0u,54u,80u,2u));
 assert(etch_seq_hold_link(&p,16u,1u,53u,0u)==1u);
 assert(etch_seq_hold_link(&p,16u,2u,53u,0u)==1u);
 assert(etch_seq_hold_link(&p,16u,9u,53u,8u)==1u);
 assert(etch_seq_hold_toggle(&p,4u,53u)==1u); /* legacy unbound */
 p.steps[0].lock_mask=0x01u;p.steps[0].locks[0]=999u;
 assert(etch_seq_toggle(&p,0u,53u,110u,1u)==0u); /* confirms original bug */
 assert(ru_seq_key_toggle(&p,0u,53u,110u)==2u);
 assert(p.steps[0].count==1u && p.steps[0].notes[0].note==54u);
 assert(p.steps[1].hold_count==0u && p.steps[2].hold_count==0u);
 assert(p.steps[9].hold_count==1u && p.steps[9].hold_ago[0]==1u);
 assert(p.steps[4].hold_count==1u && p.steps[4].hold_ago[0]==0u);
 assert(p.steps[0].lock_mask==1u && p.steps[0].locks[0]==999u);
 assert(ru_seq_delete_selected(&p,0u));
 assert(p.steps[0].count==0u && p.steps[0].lock_mask==1u);
 /* Preserve all pattern data on invalid/corrupted input. */
 static struct etch_native_pattern snapshot;
 snapshot=p;
 p.steps[7].hold_count=ETCH_SEQ_NOTES+1u;snapshot=p;
 assert(!ru_seq_note_delete(&p,8u,53u));
 assert(!memcmp(&p,&snapshot,sizeof p));
 p.steps[7].hold_count=0u;

 /* The audio clock reports step 0 on the first tick, then 1..15 and 0.
  * The LED and LCD must display the same actual step. Previously LCD
  * only used idle (350-750 ms) refresh, losing multiple 125 ms steps. */
 uint16_t last_painted=0xffffu;
 uint32_t last_paint_ms=0;
 unsigned paints=0u;
 for(uint32_t ms=0;ms<=2000u;ms+=25u){
  const uint16_t next_step=(uint16_t)((ms/125u)%16u);
  uint16_t audible=etch_seq_actual_step(1u,1u,1u,(uint8_t)next_step,16u,4u);
  assert(audible==next_step);
  const uint8_t due=ru_seq_playhead_needs_paint(1u,1u,0u,audible,last_painted);
  if(ru_lcd_should_paint(ms,last_paint_ms,(uint8_t)(ms==0u),due,0u,0u,0u)){
   last_paint_ms=ms;last_painted=audible;paints++;
  }
  if(ms%125u==0u)assert(last_painted==audible);
 }
 assert(paints>=17u); /* includes correct wraparound return to step 1 */
 assert(!ru_seq_playhead_needs_paint(1u,0u,0u,8u,7u));
 assert(!ru_seq_playhead_needs_paint(0u,1u,0u,8u,7u));
 /* Under repeated DMA misses, the LCD caps redrawing but NEVER changes
  * the audio step or the LED step. Pending redraw is not lost. */
 last_painted=2u;last_paint_ms=100u;
 assert(ru_seq_playhead_needs_paint(1u,1u,0u,3u,last_painted));
 assert(!ru_lcd_should_paint(150u,last_paint_ms,0u,1u,0u,25u,3u));
 assert(ru_seq_playhead_needs_paint(1u,1u,0u,3u,last_painted));
 assert(ru_lcd_should_paint(280u,last_paint_ms,0u,1u,0u,25u,3u));
 puts("PASS V1.68: step-1 and 128-step deletion, directed HOLD cleanup, no lock losses, 17 live playhead redraws and loop wrap, DMA cap retained");
 return 0;
}
