#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_patch_state.h"
int main(void){
 struct etch_native_patch p={0};uint8_t a=0,b=0,c=0,d=0;
 p.valid=1;p.morph=16384;p.depth=20000;
 assert(!ru_patch_cxs_load(&p,&a,&b,&c,&d));
 for(int x=0;x<=100;x++){
  ru_patch_cxs_save(&p,x,100-x,(x*37)%101,(x*53)%101);
  assert(ru_patch_cxs_load(&p,&a,&b,&c,&d));
  assert(a==x&&b==100-x&&c==(x*37)%101&&d==(x*53)%101);
 }
 p.valid=0x81;p.morph=0x3fff;
 assert(!ru_patch_cxs_load(&p,&a,&b,&c,&d));
 puts("PASS CXS four oscillator parameters persist in tagged legacy-sized patch records");return 0;
}
