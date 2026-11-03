#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_panel_anim.h"
static void check_hold_and_release(uint8_t mode){
 RuPanelAnim a={0};uint32_t now=0;
 assert(ru_panel_update(&a,now,1u,mode));
 for(int i=0;i<15;i++){now+=90u;ru_panel_update(&a,now,1u,mode);}
 assert(a.phase==RU_PANEL_HOLD && a.frame==12u);
 uint8_t first=a.shake;int updates=0;uint8_t seen[3]={0};
 for(int i=0;i<9;i++){now+=120u;if(ru_panel_update(&a,now,1u,mode))updates++;seen[a.shake]=1u;}
 assert(updates>3 && seen[0] && seen[1] && seen[2] && a.frame==12u);
 assert(ru_panel_update(&a,now+1u,0u,mode));assert(a.phase==RU_PANEL_CLOSE);
 now++;for(int i=0;i<20;i++){now+=120u;ru_panel_update(&a,now,0u,mode);}
 assert(a.phase==RU_PANEL_IDLE && a.frame==0u && a.shake==0u);
 (void)first;
}
int main(void){
 check_hold_and_release(0u);check_hold_and_release(1u);
 RuPanelAnim a={0};uint32_t now=0;assert(ru_panel_update(&a,now,1u,2u));
 for(int i=0;i<20;i++){now+=100u;ru_panel_update(&a,now,1u,2u);}
 assert(a.phase==RU_PANEL_HOLD&&a.frame>=11u&&a.frame<=13u);
 uint8_t seen[3]={0};for(int i=0;i<15;i++){seen[a.frame-11u]=1u;now+=100u;ru_panel_update(&a,now,1u,2u);}
 assert(seen[0]&&seen[1]&&seen[2]);
 assert(ru_panel_update(&a,now+1u,0u,2u));
 for(int i=0;i<20;i++){now+=100u;ru_panel_update(&a,now,0u,2u);}
 assert(a.phase==RU_PANEL_IDLE && a.frame==0u);
 puts("PASS: LFO/ENV sustained 3-phase shake, note release, volcano lava loop and close");
 return 0;
}
