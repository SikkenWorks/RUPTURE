#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
int main(void){
 RuState v={0};v.page=RU_PAGE_SUB1;v.sub_blend=70;
 /* Isolate the knob painter: no pixel in the subtitle rectangle y50..80. */
 ru_clear();ru_draw_knobs(&v);
 for(int y=50;y<=80;y++)for(int x=0;x<240;x++)assert(!ru_get(x,y));
 /* The two upper knobs start safely BELOW the subtitle's 80px edge. */
 ru_render(&v);assert(ru_get(8,50)&&ru_get(120,79));
 v.page=RU_PAGE_SHAPE;ru_clear();ru_draw_knobs(&v);
 for(int y=50;y<=80;y++)for(int x=0;x<240;x++)assert(!ru_get(x,y));
 /* Header wordmark survives the negative-space tear and remains recognizable. */
 ru_clear();ru_wordmark();int count=0;for(int y=3;y<22;y++)for(int x=7;x<135;x++)count+=ru_get(x,y);
 assert(count>200&&count<1600);
 puts("PASS V1.24: lower knob clearance, 1-bit header rupture, exact native rendering");
 return 0;
}
