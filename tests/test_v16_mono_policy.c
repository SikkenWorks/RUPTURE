#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_mono_policy.h"
int main(void){
 uint8_t notes[6]={48,60,36,67,60,52};
 assert(rupture_choose_pitch(notes,6,0)==5);
 assert(rupture_choose_pitch(notes,6,1)==3);
 assert(rupture_choose_pitch(notes,6,2)==2);
 /* Newest wins ties in high and low modes. */
 uint8_t ties[4]={60,48,60,48};
 assert(rupture_choose_pitch(ties,4,1)==2);
 assert(rupture_choose_pitch(ties,4,2)==3);
 /* One audible oscillator: glide moves towards its target without overshoot. */
 uint32_t now=25000,goal=62000;
 for(uint32_t left=5000;left;left--){
  uint32_t next=rupture_glide_next(now,goal,left);
  assert(next>=now && next<=goal);now=next;
 }
 assert(now==goal);
 now=goal;goal=25000;
 for(uint32_t left=5000;left;left--){
  uint32_t next=rupture_glide_next(now,goal,left);
  assert(next<=now && next>=goal);now=next;
 }
 assert(now==goal);
 assert(rupture_glide_next(100,200,0)==200);
 puts("PASS V1.6 native mono LAST/HIGH/LOW, ties, bounded forward/reverse glide");
}
