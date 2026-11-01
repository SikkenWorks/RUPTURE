#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
int main(void){
 struct etch_fx_settings c;etch_fx_defaults(&c);etch_fx_reset(&fx);
 c.comp_ratio=32767u;c.comp_threshold=32767u;
 for(int i=0;i<10000;i++){int16_t l=0,r=0;etch_fx_process_stereo_ex(&fx,19000,0,0,0,&c,&l,&r);}
 assert(fx.comp_gain<32767 && fx.comp_gain>0);
 int32_t before=fx.comp_gain;
 c.comp_ratio=0;
 int16_t l=0,r=0;
 etch_fx_process_stereo_ex(&fx,19000,0,0,0,&c,&l,&r);
 assert(fx.comp_gain>before && fx.comp_gain<32767); /* no sudden unity jump */
 for(int i=0;i<2000;i++)etch_fx_process_stereo_ex(&fx,19000,0,0,0,&c,&l,&r);
 assert(fx.comp_gain==32767 && l==19000 && r==19000);
 puts("PASS compressor enabled/disabled release has no abrupt gain reset; full dry recovered");
 return 0;
}
