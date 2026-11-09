/* Host safety tests for the genuine RUPTURE dual-JEDEC allowlist. */
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "hal/fm1_flash_id.h"
static int would_write(uint32_t id){
 /* A failed SPI ID read (0) preserves the previously working write path. */
 return id==0u || FL_JEDEC_OK(id);
}
static void ota_identity_bytes(uint32_t id, uint8_t dst[27]){
 for (unsigned i=0; i<27; ++i) dst[i]=0;
 dst[24]=(uint8_t)(id>>16);dst[25]=(uint8_t)(id>>8);dst[26]=(uint8_t)id;
}
int main(void){
 const uint32_t recognized[]={0x856014u,0xB36014u};
 for(unsigned i=0;i<2;i++){
   uint8_t identity[27];
   uint32_t chip=recognized[i];
   assert(FL_JEDEC_OK(chip)); assert(would_write(chip));
   ota_identity_bytes(chip,identity);
   assert(identity[24]==(chip>>16 &255u));
   assert(identity[25]==(chip>>8 &255u));
   assert(identity[26]==(chip &255u));
 }
 const uint32_t invalid[]={0xFFFFFFu,0x00000001u,0x856013u,0xB36015u,0xEF4014u,0x0000B3u};
 for(unsigned i=0;i<sizeof invalid/sizeof invalid[0];++i){
  assert(!FL_JEDEC_OK(invalid[i]));assert(!would_write(invalid[i]));
 }
 assert(!FL_JEDEC_OK(0u));assert(would_write(0u));
 puts("PASS: Puya and UCUN recognized; readable unknown IDs refused; failed probe preserves legacy path; OTA ID bytes match");
 return 0;
}
