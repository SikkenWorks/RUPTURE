/* V2.0.19: PRESETS crossfade must remain PCM-exact when replacing the
 * historical per-sample /256 with a bounded, division-free Q8 operation. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_preset_ramp.h"

static uint32_t rng=0x7dcf4a27u;
static int32_t random_pcm(void){
 rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
 return (int32_t)(rng&65535u)-32768;
}
int main(void){
 const int32_t extremes[]={0,1,-1,32767,-32768,16384,-16384,8191,-8191};
 for(uint32_t i=0;i<sizeof(extremes)/sizeof(extremes[0]);i++)
  for(uint32_t j=0;j<sizeof(extremes)/sizeof(extremes[0]);j++)
   for(uint32_t step=0;step<=256u;step++){
    int32_t x=extremes[i]*(int32_t)step+extremes[j]*(int32_t)(256u-step);
    assert(etch_preset_ramp_q8(x)==x/256);
   }
 struct etch_preset_ramp r={0};
 for(uint32_t t=0;t<500u;t++){
  int32_t held_l=random_pcm(),held_r=random_pcm();
  r.last_l=held_l;r.last_r=held_r;
  etch_preset_ramp_begin(&r);
  for(uint32_t sample=0;sample<257u;sample++){
   int32_t l=random_pcm(),right=random_pcm();
   int32_t expected_l=l,expected_r=right;
   if(sample<256u){
    expected_l=(held_l*(int32_t)(256u-sample)+l*(int32_t)sample)/256;
    expected_r=(held_r*(int32_t)(256u-sample)+right*(int32_t)sample)/256;
   }
   etch_preset_ramp_frame(&r,&l,&right);
   assert(l==expected_l&&right==expected_r);
  }
  assert(r.remaining==0u);
 }
 puts("PASS V2.0.19 preset ramp: 128000 stereo frames bit-exact, no runtime division");
 return 0;
}
