#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_v190_project.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_mono_policy.h"

static uint32_t play_curve(uint8_t curve){
 uint32_t pitch=60000u;
 for(uint32_t left=500u;left>0u;left--){
  uint32_t next=rupture_glide_next_curve(pitch,120000u,left,curve);
  assert(next>=pitch&&next<=120000u);
  pitch=next;
 }
 return pitch;
}
int main(void){
 static const uint8_t expected[RU_FX_TYPES]={
  RU_FX_REVERB,RU_FX_DELAY,RU_FX_CHORUS,RU_FX_WAH,
  RU_FX_MOSFET,RU_FX_DISTORTION,RU_FX_FUZZ,RU_FX_SAND,
  RU_FX_COMP,RU_FX_TILT};
 struct rupture_v190_project p;ru_v190_defaults(&p);
 for(uint8_t n=0u;n<RU_FX_TYPES;n++){
  assert(ru_fx_library_type(n)==expected[n]);
  assert(ru_fx_library_index(expected[n])==n);
 }
 for(uint8_t curve=0u;curve<6u;curve++){
  assert(ru_v190_glide_stage(&p,curve,320u,2u,curve,1u));
  assert(ru_v190_valid(&p));
  uint8_t flags=p.glide_patch[curve][1];
  uint8_t loaded=(uint8_t)(((flags>>2u)&3u)|((flags>>3u)&4u));
  assert(loaded==curve);
  assert(play_curve(curve)==120000u);
 }
 assert((p.glide_patch[0][1]&0x17u)==0x12u); /* old LINEAR flags */
 assert((p.glide_patch[1][1]&0x17u)==0x16u); /* old EXP flags */
 uint32_t bend=65536u;
 uint32_t next=rupture_bend_slew_q16(bend,100000u);
 assert(next>bend&&next<100000u); /* no pitch jump on knob change */
 bend=next;
 for(uint32_t i=0u;i<3000u;i++)bend=rupture_bend_slew_q16(bend,100000u);
 assert(bend==100000u);
 next=rupture_bend_slew_q16(bend,65536u);
 assert(next<bend&&next>65536u); /* smooth bend release too */
 bend=next;
 for(uint32_t i=0u;i<3000u;i++)bend=rupture_bend_slew_q16(bend,65536u);
 assert(bend==65536u);
 puts("PASS V2.0.4: 6 persisted GLIDE curves, continuous BEND, correct FX browser order");
 return 0;
}
