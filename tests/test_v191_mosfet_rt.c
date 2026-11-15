/* V1.91: the dual-channel post-mix drive must be monotone, bounded and
 * division-free at audio rate. Golden transfer uses a 64-bit host oracle;
 * native LUT has no division or 64-bit math in the sample path. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_mosfet_lut.h"

int main(void){
 uint32_t max_error=0u;
 const uint16_t gains[]={256u,384u,512u,768u,1024u};
 for(unsigned g=0u;g<sizeof(gains)/sizeof(gains[0]);g++){
  uint32_t last=0u;
  for(int32_t x=0;x<=32767;x++){
   uint32_t hot=((uint32_t)x*gains[g]+128u)>>8;
   uint32_t ideal=(uint32_t)(((uint64_t)hot*32767u)/(32767u+hot));
   uint32_t actual=(uint16_t)ru_mosfet_shaper(x,gains[g]);
   assert(actual>=last && actual<=32767u);
   assert(ru_mosfet_shaper(-x,gains[g])==-(int32_t)actual);
   uint32_t delta=actual>ideal?actual-ideal:ideal-actual;
   if(delta>max_error)max_error=delta;
   last=actual;
  }
 }
 assert(max_error<=3u);
 assert(ru_mosfet_shaper(0,1024u)==0);
 assert(ru_mosfet_shaper(32767,1024u)>0);
 printf("PASS V1.91 MOSFET LUT: monotone, symmetric, bounded, max oracle error %u PCM\n",max_error);
 return 0;
}
