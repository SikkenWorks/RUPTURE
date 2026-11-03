/* Native RUPTURE V1.61 UI + encoder regression. No hardware or audio calls. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_save_route.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_encoder_feel.h"
static uint32_t h(void){uint32_t result=2166136261u;
 for(unsigned i=0;i<sizeof(ru_bits);i++)result=(result^ru_bits[i])*16777619u;
 return result;}
int main(void){
 RuEncoderFeel e={0};
 assert(ru_encoder_feel(&e,1000,1,1)==1);
 assert(ru_encoder_feel(&e,1400,1,1)==1);
 assert(ru_encoder_feel(&e,1430,1,1)==1);
 assert(ru_encoder_feel(&e,1460,1,1)==2);
 assert(ru_encoder_feel(&e,1490,-1,1)==-1);
 assert(ru_encoder_feel(&e,1510,-1,1)==-1);
 assert(ru_encoder_feel(&e,1530,-1,1)==-2);
 assert(ru_encoder_feel(&e,1550,3,0)==3);
 assert(ru_encoder_feel(&e,1560,1,0)==1);
 assert(ru_encoder_feel(&e,1570,-1,0)==-1);
 assert(ru_save_destination(RU_PAGE_SYS_INFO)==RU_SAVE_NO_MENU);
 RuState v={0};v.page=RU_PAGE_SYS_INFO;v.system_storage_ok=1;
 v.system_projects_used=4;v.system_usb=1;v.battery=73;v.system_battery_valid=1;
 v.diag_dsp=36;v.diag_late=0;ru_render(&v);uint32_t good=h();
 v.system_storage_ok=0;v.system_storage_empty=1;ru_render(&v);uint32_t empty=h();
 assert(good!=empty);
 v.system_storage_empty=0;v.diag_late=19;ru_render(&v);assert(h()!=empty);
 v=(RuState){0};v.page=RU_PAGE_GLO;v.selected=1u;ru_render(&v);uint32_t nav1=h();
 v.selected=5u;ru_render(&v);assert(h()!=nav1);
 v=(RuState){0};v.page=RU_PAGE_PLAY;v.k[0]=50;v.k[1]=50;v.k[2]=50;v.k[3]=50;
 ru_render(&v);uint32_t idle=h();v.knob_focus_visible=1;v.knob_focus=0;ru_render(&v);
 assert(h()!=idle);
 puts("PASS V1.61: calibrated encoder acceleration, discrete no-skip, system status variants, GLO 6 cards, focus marker, SAVE blocked on info");
 return 0;
}
