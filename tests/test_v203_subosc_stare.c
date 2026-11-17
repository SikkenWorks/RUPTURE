#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc_fear_playback.h"

int main(void){
 uint8_t frame=0u,active=0u,shake=0u;
 uint32_t last=0u;
 assert(!ru_sub1_stare_step(0u,0u,&frame,&active,&last,&shake));
 assert(ru_sub1_stare_step(1000u,1u,&frame,&active,&last,&shake));
 assert(frame==1u && active && shake==0u);
 for(uint32_t n=2u;n<=RU_SUB1_STARE_FRAME;n++){
  assert(!ru_sub1_stare_step(1000u+75u*(n-1u)-1u,1u,
    &frame,&active,&last,&shake));
  assert(ru_sub1_stare_step(1000u+75u*(n-1u),1u,
    &frame,&active,&last,&shake));
  assert(frame==n && shake==0u);
 }
 for(uint32_t n=1u;n<=50u;n++){
  assert(ru_sub1_stare_step(1000u+75u*(RU_SUB1_STARE_FRAME-1u+n),
    1u,&frame,&active,&last,&shake));
  assert(frame==RU_SUB1_STARE_FRAME); /* no more facial frames */
  assert(shake==(n&7u)); /* only camera jitter while the note is held */
 }
 assert(ru_sub1_stare_step(8000u,0u,&frame,&active,&last,&shake));
 assert(frame==0u && shake==0u && !active);
 assert(!ru_sub1_stare_step(8050u,0u,&frame,&active,&last,&shake));
 assert(ru_sub1_stare_step(8100u,1u,&frame,&active,&last,&shake));
 assert(frame==1u && shake==0u && active);
 /* SUB OSC 2 keeps the legacy 13..16 eye-loop during sustain. */
 frame=0u;active=0u;last=0u;
 assert(ru_sub_fear_play_step(1000u,1u,&frame,&active,&last));
 for(uint32_t n=2u;n<=15u;n++){
  assert(ru_sub_fear_play_step(1000u+75u*(n-1u),1u,&frame,&active,&last));
  assert(frame==n);
 }
 assert(ru_sub_fear_play_step(2125u,1u,&frame,&active,&last));
 assert(frame==12u);
 puts("PASS V2.0.3: SUB1 fixed stare with camera tremor; SUB2 legacy loop unchanged");
 return 0;
}
