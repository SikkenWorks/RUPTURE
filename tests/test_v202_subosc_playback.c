#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc_fear_playback.h"

int main(void){
 uint8_t frame=0u,active=0u;
 uint32_t last=0u;
 assert(!ru_sub_fear_play_step(0u,0u,&frame,&active,&last));
 assert(frame==0u);
 assert(ru_sub_fear_play_step(1000u,1u,&frame,&active,&last));
 assert(frame==1u && active);
 assert(!ru_sub_fear_play_step(1074u,1u,&frame,&active,&last));
 assert(ru_sub_fear_play_step(1075u,1u,&frame,&active,&last));
 assert(frame==2u);
 for(uint32_t n=3u;n<=15u;n++){
  assert(ru_sub_fear_play_step(1000u+75u*(n-1u),1u,&frame,&active,&last));
  assert(frame==n);
 }
 assert(ru_sub_fear_play_step(2125u,1u,&frame,&active,&last));
 assert(frame==12u); /* eye tremor loops only frames 13-16 */
 assert(ru_sub_fear_play_step(2200u,1u,&frame,&active,&last));
 assert(frame==13u);
 assert(ru_sub_fear_play_step(2230u,0u,&frame,&active,&last));
 assert(frame==0u && !active);
 assert(!ru_sub_fear_play_step(2240u,0u,&frame,&active,&last));
 assert(ru_sub_fear_play_step(2500u,1u,&frame,&active,&last));
 assert(frame==1u);
 puts("PASS V2.0.2: fear turn, eyes tremor loop, true note-off reset, re-trigger");
 return 0;
}
