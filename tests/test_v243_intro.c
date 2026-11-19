/* New SikkenWorks -> biblical text -> moving word -> fade, no audio changes. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"

static void draw(unsigned index){
 RuState v={0};v.page=RU_PAGE_LOADING;v.boot_step=(uint8_t)(index+1u);
 ru_render(&v);
}
static unsigned lit(void){
 unsigned sum=0u;
 for(unsigned i=0;i<7200u;i++)sum+=(unsigned)__builtin_popcount((unsigned)ru_bits[i]);
 return sum;
}
int main(void){
 assert(RU_SPLASH_STEPS==RU_INTRO_FRAMES&&RU_INTRO_FRAMES==86u&&RU_INTRO_DURATION_MS==5160u);
 assert(ru_intro_frame_at(0u)==0u);
 assert(ru_intro_frame_at(5159u)==85u);
 assert(ru_intro_frame_at(5160u)==85u);
 assert(ru_intro_frame_at(99999u)==85u);
 assert(ru_intro_frame_desc(0u).scene==RU_INTRO_SCENE_LOGO);
 assert(ru_intro_frame_desc(25u).opacity==0u);
 assert(ru_intro_frame_desc(26u).scene==RU_INTRO_SCENE_TEXT);
 assert(ru_intro_frame_desc(46u).uppercase==0u);
 assert(ru_intro_frame_desc(62u).uppercase==1u);
 assert(ru_intro_frame_desc(85u).opacity==0u);
 draw(0u);assert(lit()==0u && ru_intro_gray_pending>1u);
 draw(10u);assert(lit()==0u && ru_intro_gray_pending==17u);
 assert(RU_SIKKENWORKS_LOGO_BYTES==57600u);
 draw(25u);assert(lit()==0u && ru_intro_gray_pending==1u);
 draw(37u);unsigned text=lit();assert(text>1400u&&text<7000u);
 assert(ru_intro_gray_pending==0u);
 /* Fully opaque rest of text must not change during word tremor. */
 draw(46u);uint8_t anchor[7200u];memcpy(anchor,ru_bits,sizeof(anchor));
 for(unsigned i=47u;i<62u;i++){
  draw(i);
  for(int y=0;y<240;y++)for(int x=0;x<240;x++){
   if(y>=181&&y<=201&&x>=65&&x<142)continue;
   int idx=y*240+x;
   assert((!!(ru_bits[idx>>3]&(0x80u>>(idx&7))))==
          (!!(anchor[idx>>3]&(0x80u>>(idx&7)))));
  }
 }
 draw(74u);unsigned caps=lit();assert(caps>1400u&&caps<7000u);
 draw(85u);assert(lit()==0u);
 puts("PASS V2.0.43: supplied PNG SikkenWorks portrait, 5.16s quicker fades, fixed text and word flicker");
 return 0;
}
