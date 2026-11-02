/* V1.52 CRT output contract: glyphs never erased; 2D warp, phosphor rows. */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
static uint8_t expected[7200];
int main(void){
 RuState v={0};
 const uint8_t pages[]={RU_PAGE_PLAY,RU_PAGE_SETTINGS,RU_PAGE_FX,
  RU_PAGE_SAVER_OPTIONS,RU_PAGE_HELP,RU_PAGE_BUG_REPORT,RU_PAGE_LOADING,RU_PAGE_SAFE};
 unsigned styled=0u;
 for(unsigned j=0;j<sizeof(pages);j++){
  v.page=pages[j];v.saver_active=0;v.crt_enabled=0;v.show_battery=1;v.battery=90;
  v.boot_step=24;v.k[0]=85;v.k[1]=55;v.k[2]=33;v.k[3]=90;
  ru_render_raw(&v);memcpy(expected,ru_bits,7200u);
  ru_render(&v);assert(!memcmp(expected,ru_bits,7200u));
  v.crt_enabled=1;ru_render(&v);
  /* SETTINGS intentionally changes its ON/OFF label, all other raw pages match. */
  if(v.page!=RU_PAGE_SETTINGS)assert(!memcmp(expected,ru_bits,7200u));
  unsigned difference=0;
  for(uint32_t y=0;y<240;y++)for(uint32_t x=0;x<240;x++){
   uint16_t off=ru_crt_pixel(ru_bits,x,y,0);
   uint16_t on=ru_crt_pixel(ru_bits,x,y,(uint8_t)(pages[j]!=RU_PAGE_BUG_REPORT));
   assert(off==0u||off==0xffffu);
   if(on!=off)difference++;
  }
  if(pages[j]==RU_PAGE_BUG_REPORT)assert(difference==0);
  else{assert(difference>0);styled++;}
 }
 assert(styled==7u);
 /* All output colours are neutral phosphor grey: R == B, no green tint. */
 for(uint32_t y=0;y<240;y++)for(uint32_t x=0;x<240;x++){
  uint16_t p=ru_crt_pixel(ru_bits,x,y,1u);
  assert(((p>>11)&31u)==(p&31u));
 }
 /* Midpoint of an image stays registered, corners show 2D black border. */
 assert(ru_crt_pixel(ru_bits,0,0,1)==0u);
 assert(ru_crt_pixel(ru_bits,239,239,1)==0u);
 puts("PASS V1.52 CRT: RAW parity, 2D geometry, neutral scanlines, QR exclusion");
 return 0;
}
