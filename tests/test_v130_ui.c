/* RUPTURE V1.30: native QR, GLO fifth card, meadow and settings positions. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
static int pixel_count(void){int n=0;for(int y=0;y<240;y++)for(int x=0;x<240;x++)n+=ru_get(x,y);return n;}
int main(void){
 RuState v={0};v.page=RU_PAGE_GLO;v.selected=4;ru_render(&v);
 for(int k=0;k<5;k++)assert(ru_get(12,48+34*k));
 v.page=RU_PAGE_BUG_REPORT;ru_render(&v);
 /* Every one of the 29*29 visual modules must equal the original PNG. */
 for(int row=0;row<29;row++)for(int col=0;col<29;col++){
  int expected=!!(ru_bug_qr_white[row][col>>3]&(0x80u>>(col&7)));
  int x=47+col*5,y=52+row*5;
  for(int dy=0;dy<5;dy++)for(int dx=0;dx<5;dx++)
   assert(ru_get(x+dx,y+dy)==expected);
 }
 assert(!ru_get(44,75) && !ru_get(196,75)); /* black quiet zone */
 assert(pixel_count()>10000);
 v.page=RU_PAGE_SETTINGS;v.selected=0;v.master=100;ru_render(&v);
 assert(ru_get(12,57)&&ru_get(12,99));
 v.auto_save=1;ru_render(&v);assert(pixel_count()>1000);
 v.saver_active=1;v.saver_frame=0;ru_render(&v);int meadow=pixel_count();
 assert(meadow>4000);
 v.saver_frame=48;ru_render(&v);int canyon=pixel_count();
 assert(canyon>3000 && canyon<meadow);
 v.saver_frame=63;ru_render(&v);assert(pixel_count()>2000);
 puts("PASS V1.31: full 841/841 author QR pixel parity / 5 GLO cards / settings / meadow");return 0;
}
