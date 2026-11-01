#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
static int count_pixels(int x0,int y0,int w,int h){int n=0;for(int y=y0;y<y0+h;y++)for(int x=x0;x<x0+w;x++)n+=ru_get(x,y);return n;}
int main(void){
 RuState s={0};s.page=RU_PAGE_SEQ;s.seq_length=4;s.seq_bpm=120;s.seq_current_step=0;s.k[0]=4;
 ru_render(&s);
 assert(count_pixels(66,69,8,24)>0); /* active 4-step row is centered */
 assert(count_pixels(12,69,23,24)==0); /* unused left region empty */
 assert(count_pixels(12,99,216,23)==0); /* no second row */
 s.seq_length=8;s.k[0]=8;ru_render(&s);
 assert(count_pixels(12,70,24,23)>0);assert(count_pixels(12,99,216,23)==0);
 s.seq_length=16;s.k[0]=16;ru_render(&s);
 assert(count_pixels(12,99,216,23)>0); /* full two rows */
 s.page=RU_PAGE_SETTINGS;s.selected=0;s.settings_open=0;s.master=100;s.auto_save=0;
 ru_render(&s);assert(count_pixels(12,69,216,64)>0);
 s.selected=1;s.auto_save=1;ru_render(&s);
 s.settings_open=1;ru_render(&s);assert(count_pixels(12,72,216,111)>0);
 s.settings_open=0;s.auto_save=1;ru_render(&s);assert(count_pixels(166,110,14,14)>0); /* direct checkbox; legacy modal removed */
 s.page=RU_PAGE_GLO;s.settings_open=0;s.selected=1;ru_render(&s);
 assert(ru_get(12,48)&&ru_get(12,82)&&ru_get(12,116)&&ru_get(12,150)&&ru_get(12,184));
 s.saver_active=1;s.saver_frame=0;ru_render(&s);
 assert(count_pixels(0,0,240,240)>4000);
 s.saver_frame=10;ru_render(&s);int few=count_pixels(0,0,240,240);assert(few>0);
 s.saver_frame=48;ru_render(&s);int more=count_pixels(0,0,240,240);assert(more<few);
 s.saver_active=0;s.page=RU_PAGE_PLAY;ru_render(&s);assert(ru_get(7,5));
 puts("PASS V1.27: hidden 4/8 steps, card navigation, idle animation and wake screen");
 return 0;
}
