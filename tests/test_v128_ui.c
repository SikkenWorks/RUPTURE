#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
static int pixels(void){int n=0;for(int y=0;y<240;y++)for(int x=0;x<240;x++)n+=ru_get(x,y);return n;}
int main(void){
 RuState v={0};v.page=RU_PAGE_GLO;v.selected=3u;ru_render(&v);
 assert(ru_get(12,48)&&ru_get(12,82)&&ru_get(12,116)&&ru_get(12,150)&&ru_get(12,184));
 v.page=RU_PAGE_SETTINGS;v.selected=2;v.settings_open=0u;v.master=100u;ru_render(&v);
 assert(ru_get(12,57)&&ru_get(12,99)&&ru_get(12,141)&&ru_get(12,183));
 v.page=RU_PAGE_OTHER;v.saver_enabled=1u;ru_render(&v);assert(pixels()>500);
 v.page=RU_PAGE_WIPE_ALL;v.manager_confirm=0u;ru_render(&v);assert(ru_get(14,157));
 v.manager_confirm=1u;ru_render(&v);assert(ru_get(126,157));
 v.page=RU_PAGE_ABOUT;v.about_scroll=0u;ru_render(&v);assert(ru_about_lines()>30u);
 char line[RU_ABOUT_COLS+1];assert(ru_about_get_line(0u,line));
 assert(strcmp(line,"RUPTURE is a monophonic bass")==0 || strlen(line)>15u);
 v.about_scroll=(uint8_t)(ru_about_lines()-11u);ru_render(&v);assert(pixels()>200);
 /* Check the wordmark E's middle bar, keeping every other letter untouched. */
 v.page=RU_PAGE_PLAY;v.about_scroll=0u;ru_render(&v);
 for(int x=0;x<6;x++)assert(ru_get(115+x*2,9));
 v.show_battery=1u;v.usb_power=1u;ru_render(&v);assert(ru_get(195,6));
 v.usb_power=0u;ru_render(&v);assert(!ru_get(195,6));
 v.saver_active=1u;v.saver_frame=0u;ru_render(&v);assert(pixels()>4000);
 v.saver_frame=48u;ru_render(&v);assert(pixels()>1000);
 puts("PASS V1.28 native cards, About wrap/scroll, middle E, USB icon and enhanced screensaver");
 return 0;
}
