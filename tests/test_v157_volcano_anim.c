#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_panel_anim.h"
int main(void){
 RuPanelAnim a={0}; uint32_t ms=0;
 assert(a.frame==0 && a.phase==RU_PANEL_IDLE);
 assert(ru_panel_update(&a,ms,1,2) && a.frame==1);
 for(int i=0;i<18;i++){ms+=100;ru_panel_update(&a,ms,1,2);}
 assert(a.phase==RU_PANEL_HOLD && a.frame>=11 && a.frame<=13);
 int seen[3]={0}; for(int i=0;i<9;i++){seen[a.frame-11]++;ms+=100;ru_panel_update(&a,ms,1,2);}
 for(int i=0;i<3;i++)assert(seen[i]>0);
 assert(ru_panel_update(&a,ms+1,0,2)&&a.phase==RU_PANEL_CLOSE&&a.frame==14);
 ms+=1;for(int i=0;i<15;i++){ms+=100;ru_panel_update(&a,ms,0,2);}
 assert(a.frame==0 && a.phase==RU_PANEL_IDLE);
 RuPanelAnim b={0};ms=0;ru_panel_update(&b,ms,1,2);ms+=1;ru_panel_update(&b,ms,0,2);
 assert(b.phase==RU_PANEL_CLOSE && b.frame>=17);
 for(int i=0;i<15;i++){ms+=100;ru_panel_update(&b,ms,0,2);}assert(b.frame==0);
 RuPanelAnim l={0}; ms=0;ru_panel_update(&l,ms,1,0);for(int i=0;i<15;i++){ms+=90;ru_panel_update(&l,ms,1,0);}
 assert(l.phase==RU_PANEL_HOLD && l.frame==12);
 for(int i=0;i<16;i++){ms+=100;ru_panel_update(&l,ms,0,0);} assert(l.frame==0);
 RuPanelAnim e={0}; ms=0;ru_panel_update(&e,ms,1,1);for(int i=0;i<15;i++){ms+=90;ru_panel_update(&e,ms,1,1);}assert(e.phase==RU_PANEL_HOLD);
 for(int i=0;i<16;i++){ms+=100;ru_panel_update(&e,ms,0,1);}assert(e.frame==0);
 puts("PASS: volcano eruption 3-frame hold loop, key-release closure, short-gate closure, LFO and ENV preserved");
 return 0;
}
