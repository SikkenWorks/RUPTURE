/* Hash canonical 1-bit RUPTURE layouts. Run before and after an optimisation
 * against the exact same compiled renderer. Independent of target hardware. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static uint64_t fnv(const void *p, uint32_t n){const uint8_t *s=p;uint64_t h=14695981039346656037ull;
 for(uint32_t i=0;i<n;i++)h=(h^s[i])*1099511628211ull;return h;}
int main(void){
 const uint8_t pages[]={RU_PAGE_PLAY,RU_PAGE_SHAPE,RU_PAGE_FILTER,RU_PAGE_ENV,RU_PAGE_LFO,RU_PAGE_FX,RU_PAGE_SEQ,RU_PAGE_SUB1,RU_PAGE_SUB2,RU_PAGE_EQ,RU_PAGE_COMP,RU_PAGE_TILT,RU_PAGE_FUZZ,RU_PAGE_SANDPAPER};
 for(unsigned z=0;z<sizeof pages;z++)for(uint8_t m=0;m<3;m++){
  RuState v={0};v.page=pages[z];v.fx_selected=(uint8_t)((z+m)%8);v.seq_length=16;v.seq_bpm=120;v.seq_grid_mask=0x5555;
  v.k[0]=(uint16_t)(22+10*m);v.k[1]=(uint16_t)(46+5*m);v.k[2]=50;v.k[3]=76;v.sub_blend=60;
  v.knob_focus_visible=m==2;v.knob_focus=m;
  v.animate=1;for(uint8_t k=0;k<4;k++)v.knob_visual[k]=(uint8_t)((12+25*k+8*m)%101);
  v.battery=80;v.system_battery_valid=1;v.eq_mode=(uint8_t)(m%2);
  ru_render(&v);printf("%02u:%u %016llx\n",pages[z],m,(unsigned long long)fnv(ru_bits,sizeof ru_bits));
 }
 return 0;
}
