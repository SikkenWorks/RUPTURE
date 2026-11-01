#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_sustain.h"
static struct etch_native_pattern pattern;
int main(void){
 memset(&pattern,0,sizeof(pattern));
 pattern.steps[1].count=1;pattern.steps[1].notes[0]=(struct etch_seq_note){36,115,1};
 pattern.steps[6].count=1;pattern.steps[6].notes[0]=(struct etch_seq_note){40,96,1};
 assert(ru_seq_extend_length(&pattern,1,16,20)==1);
 assert(pattern.steps[1].notes[0].length==5); /* steps 1..5, stops before 6 */
 assert(pattern.steps[6].notes[0].note==40);
 assert(pattern.steps[1].notes[0].velocity==115);
 assert((ru_seq_sustain_mask(&pattern,16,0)&0x003cu)==0x003cu);
 assert(!(ru_seq_sustain_mask(&pattern,16,0)&(1u<<6)));
 assert(!ru_seq_extend_length(&pattern,1,16,1));
 assert(ru_seq_extend_length(&pattern,1,16,-3));
 assert(pattern.steps[1].notes[0].length==2);
 assert(ru_seq_extend_length(&pattern,1,16,-99));
 assert(pattern.steps[1].notes[0].length==1);
 assert(!ru_seq_extend_length(&pattern,1,16,-1));
 assert(!ru_seq_extend_length(&pattern,0,16,4));
 assert(!ru_seq_extend_length(&pattern,16,16,1));
 pattern.steps[15].count=1;pattern.steps[15].notes[0]=(struct etch_seq_note){42,127,1};
 assert(!ru_seq_extend_length(&pattern,15,16,8));
 pattern.steps[6].count=0;
 assert(ru_seq_extend_length(&pattern,1,16,100));
 assert(pattern.steps[1].notes[0].length==14); /* reaches 14, stops at 15 */
 pattern.steps[15].count=0;
 assert(ru_seq_extend_length(&pattern,1,16,50));
 assert(pattern.steps[1].notes[0].length==15); /* full sequence */
 assert(ru_seq_extend_length(&pattern,1,16,-100));
 puts("PASS V1.23: ALGORITHM duration, empty steps, next trigger boundary, backwards, velocity and pitch retained");
 return 0;
}
