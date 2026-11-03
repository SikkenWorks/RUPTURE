/* V1.67 UI focus regression: actual 240x240 packed renderer, every FX K1-K4,
 * non-FX rotary pages, special sequencer, SUB blend and EQ compact values. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static uint8_t baseline[sizeof ru_bits];
static int pixel(int x,int y){unsigned k=(unsigned)y*240u+(unsigned)x;return (ru_bits[k/8]>>(7-(k%8)))&1;}
static void verify_page(uint8_t page,uint8_t fx,uint8_t knob){
 RuState v={0};v.page=page;v.fx_selected=fx;v.seq_length=16;v.seq_bpm=120;
 v.system_storage_ok=1;v.k[0]=50;v.k[1]=50;v.k[2]=50;v.k[3]=50;
 v.eq_mode=1;v.eq_on=1;v.eq_tab=0;
 ru_render(&v);memcpy(baseline,ru_bits,sizeof baseline);
 v.knob_focus_visible=1;v.knob_focus=knob;
 ru_render(&v);
 if(memcmp(ru_bits,baseline,sizeof baseline)==0){
  fprintf(stderr,"Missing UI focus: page=%u FX=%u K%u\n",page,fx,knob+1u);assert(0);
 }
 if(page==RU_PAGE_FX){
  int x=knob%2u?176:64,y=knob>=2u?181:105;
  assert(pixel(x-29,y-5)&&pixel(x+27,y-5));
 }
 if(page==RU_PAGE_SEQ){
  int x=36+56*knob;
  assert(pixel(x-29,173)&&pixel(x+27,173));
 }
 if((page==RU_PAGE_SUB1||page==RU_PAGE_SUB2)&&knob==4u){
  assert(pixel(91,130)&&pixel(147,130));
 }
}
int main(void){
 for(unsigned fx=0;fx<8;fx++)for(uint8_t k=0;k<4;k++)verify_page(RU_PAGE_FX,(uint8_t)fx,k);
 const uint8_t pages[]={RU_PAGE_PLAY,RU_PAGE_SHAPE,RU_PAGE_ENV,RU_PAGE_LFO,RU_PAGE_FILTER,
  RU_PAGE_ARP,RU_PAGE_SEQ,RU_PAGE_NOTE_MEMORY,RU_PAGE_GUIDE,RU_PAGE_KEYLOCK,
  RU_PAGE_REVERB,RU_PAGE_DELAY,RU_PAGE_CHORUS,RU_PAGE_DRIVE,RU_PAGE_COMP,
  RU_PAGE_FOLDER,RU_PAGE_TILT,RU_PAGE_FUZZ,RU_PAGE_SANDPAPER,RU_PAGE_SUB1,RU_PAGE_SUB2};
 for(unsigned p=0;p<sizeof pages;p++)for(uint8_t k=0;k<4;k++)verify_page(pages[p],0,k);
 verify_page(RU_PAGE_SUB1,0,4u);verify_page(RU_PAGE_SUB2,0,4u);
 for(unsigned mode=0;mode<2;mode++)for(uint8_t k=0;k<4;k++){
  RuState v={0};v.page=RU_PAGE_EQ;v.eq_mode=(uint8_t)mode;v.k[k]=50;
  ru_render(&v);memcpy(baseline,ru_bits,sizeof baseline);
  v.knob_focus_visible=1;v.knob_focus=k;ru_render(&v);
  assert(memcmp(ru_bits,baseline,sizeof baseline)!=0);
 }
 puts("PASS V1.67: all 8 FX x4, 21 rotary pages x4, both SUB blend knobs, both EQ layouts x4; packed framebuffer focus visible");
 return 0;
}
