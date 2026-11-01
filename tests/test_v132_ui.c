/* V1.32 Utrecht art, screensaver options, REC LED and website checks. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
int main(void){
 RuState v={0};v.page=RU_PAGE_OTHER;v.saver_enabled=1u;ru_render(&v);
 assert(ru_get(12,76) && ru_get(12,141));
 v.page=RU_PAGE_SAVER_OPTIONS;v.saver_delay_index=1u;
 ru_render(&v);assert(ru_get(12,73)&&ru_get(12,139));
 v.saver_delay_index=7;v.saver_options_selected=1;ru_render(&v);assert(ru_get(12,139));
 v.saver_active=1;v.saver_frame=0;ru_render(&v);
 int city=0;for(int y=0;y<165;y++)for(int x=0;x<240;x++)city+=ru_get(x,y);
 assert(city>1500);
 v.saver_frame=48;ru_render(&v);
 int newcity=0;for(int y=0;y<165;y++)for(int x=0;x<240;x++)newcity+=ru_get(x,y);
 assert(newcity==city); /* Dom Tower remains stable */
 assert(!ru_get(120,224)); /* canyon void */
 v.saver_frame=63;ru_render(&v);assert(!ru_get(120,224));
 puts("PASS V1.32 FM1 Utrecht art, fracturing street, 2-row saver options");
 return 0;
}
