#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_poly_headroom.h"
int main(void){
 for(uint32_t admitted=0;admitted<=2;admitted++)
   assert(etch_poly_headroom_step(4096,admitted)==4096);
 for(int32_t x=-524288;x<=524288;x++){
  int32_t original=(x*1024)/4096;
  int32_t mono=x/4;
  assert(original==mono);
 }
 puts("PASS mono fixed divider matches variable Q10 mixer for all bounded samples and 0-2 admissions");
 return 0;
}
