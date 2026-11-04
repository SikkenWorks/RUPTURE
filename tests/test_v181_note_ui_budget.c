#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_note_paint_budget.h"
int main(void){
 struct ru_note_paint_budget b={0};
 ru_note_paint_changed(&b,1000);assert(b.burst==1);
 assert(ru_note_paint_allow(&b,1000,990,0,0));
 ru_note_paint_committed(&b);
 ru_note_paint_changed(&b,1025);assert(b.burst==2);
 assert(ru_note_paint_allow(&b,1025,1000,0,0));
 ru_note_paint_committed(&b);
 ru_note_paint_changed(&b,1050);assert(b.burst==3);
 assert(!ru_note_paint_allow(&b,1050,1025,0,0));
 assert(ru_note_paint_allow(&b,1050,1025,1,0)); /* menus never blocked */
 assert(ru_note_paint_allow(&b,1100,1025,0,0));
 assert(!ru_note_paint_allow(&b,1100,1025,0,12)); /* pressure -> 100ms */
 assert(ru_note_paint_allow(&b,1130,1025,0,12));
 assert(ru_note_paint_allow(&b,1220,1200,0,12)); /* burst timed out */
 ru_note_paint_changed(&b,1500);assert(b.burst==1);
 puts("PASS V1.81 note burst LCD budget: coalesces visuals, does not block commands");
 return 0;
}
