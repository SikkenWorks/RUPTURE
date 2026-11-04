/* V1.74 test: deterministic live hold capture + lossless circular shift. */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_live.h"
int main(void){
 static struct etch_native_pattern p,q,back;
 struct RuLiveCapture rec={0};
 assert(etch_seq_put(&p,2,55,100,1)==1);
 ru_live_note_on(&rec,3,2,55,8);
 assert(rec.active==(1u<<3));
 assert(!ru_live_advance(&p,&rec,2,8,(1u<<3)));
 assert(ru_live_advance(&p,&rec,3,8,(1u<<3)));
 assert(p.steps[2].notes[0].length==2);
 assert(ru_live_advance(&p,&rec,5,8,(1u<<3)));
 assert(p.steps[2].notes[0].length==4);
 assert(ru_live_advance(&p,&rec,6,8,0));
 assert(p.steps[2].notes[0].length==5 && !rec.active);
 /* REC overdub can extend but never shrink a long existing note. */
 ru_live_reset(&rec);ru_live_note_on(&rec,3,2,55,8);
 assert(!ru_live_advance(&p,&rec,2,8,0));
 assert(p.steps[2].notes[0].length==5);
 /* Pattern wrap in recorded duration (origin last cell, next beat at zero). */
 assert(etch_seq_put(&p,7,60,94,1)==1);
 ru_live_reset(&rec);ru_live_note_on(&rec,9,7,60,8);
 ru_live_advance(&p,&rec,0,8,(1u<<9));
 assert(p.steps[7].notes[0].length==2);
 ru_live_advance(&p,&rec,1,8,0);
 assert(p.steps[7].notes[0].length==3);
 /* Multiple pitches, holds and all parameter-lock data travel as a unit. */
 p.steps[2].lock_mask=0x25u;p.steps[2].locks[0]=440;
 assert(etch_seq_hold_link(&p,8,3,55,2)==1);
 assert(etch_seq_hold_link(&p,8,4,55,2)==1);
 assert(etch_seq_hold_span(&p,8,2,55)>=3u);
 back=p;
 assert(ru_seq_rotate(&p,&q,8,-1)); /* step2 -> step1; step7 -> step6 */
 assert(q.steps[1].notes[0].note==55 && q.steps[1].lock_mask==0x25u);
 assert(q.steps[1].locks[0]==440);
 assert(q.steps[6].count==1 && q.steps[6].notes[0].note==60);
 assert(etch_seq_hold_owner(&q,2,55)==1);
 assert(etch_seq_hold_span(&q,8,1,55)>=3u);
 /* Shift across boundary, native HOLD origin re-encoded (old 2 -> new 7). */
 assert(ru_seq_rotate(&back,&q,8,-3));
 assert(q.steps[7].notes[0].note==55);
 assert(etch_seq_hold_owner(&q,0,55)==7);
 assert(etch_seq_hold_owner(&q,1,55)==7);
 assert(etch_seq_hold_span(&q,8,7,55)>=3u);
 assert(ru_seq_rotate(&q,&p,8,3));
 assert(!memcmp(&p,&back,sizeof p));
 assert(!ru_seq_rotate(&p,&p,8,1));
 assert(!ru_seq_rotate(&p,&q,0,1));
 assert(!ru_seq_rotate(&p,&q,8,0));
 assert(!ru_seq_rotate(&p,&q,8,8));
 /* Eight simultaneous physical piano keys may be captured independently. */
 etch_pattern_clear(&p);ru_live_reset(&rec);
 for(unsigned k=0;k<8;k++){assert(etch_seq_put(&p,0,(uint8_t)(60+k),90,1));ru_live_note_on(&rec,k,0,(uint8_t)(60+k),16);}
 assert(ru_live_advance(&p,&rec,4,16,0xff));
 for(unsigned k=0;k<8;k++)assert(p.steps[0].notes[k].length==5);
 ru_live_finish(&p,&rec);assert(!rec.active);
 puts("PASS V1.74 sustained Note ON/OFF recording, audio playhead wrap, overdub preservation, lossless circular shift, linked HOLD origins, parameter locks, poly key capture");
 return 0;
}
