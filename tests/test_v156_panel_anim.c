#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "rupture_panel_anim.h"
int main(void){
 RuPanelAnim lfo={0},env={0};uint32_t time=0;
 assert(ru_panel_update(&lfo,time,0,0)==0);
 assert(ru_panel_update(&lfo,time,1,0)==1);
 assert(lfo.frame==1 && lfo.phase==RU_PANEL_OPEN);
 for(int i=0;i<11;i++){time+=80;ru_panel_update(&lfo,time,1,0);}
 assert(lfo.frame==12);time+=80;ru_panel_update(&lfo,time,1,0);
 assert(lfo.phase==RU_PANEL_HOLD);
 time+=100;assert(ru_panel_update(&lfo,time,1,0)==0);
 for(int i=0;i<13;i++){time+=80;ru_panel_update(&lfo,time,0,0);}
 assert(lfo.frame==0 && lfo.phase==RU_PANEL_IDLE);
 time=0;ru_panel_update(&env,time,1,1);
 for(int i=0;i<11;i++){time+=80;ru_panel_update(&env,time,1,1);}
 assert(env.frame==12);
 time+=80;ru_panel_update(&env,time,0,1);
 assert(env.phase==RU_PANEL_CLOSE && env.frame==12);
 for(int i=0;i<13;i++){time+=90;ru_panel_update(&env,time,0,1);}
 assert(env.frame==0 && env.phase==RU_PANEL_IDLE);
 /* Short note doesn't skip ahead to full rupture */
 time=0;env=(RuPanelAnim){0};ru_panel_update(&env,time,1,1);
 time+=10;ru_panel_update(&env,time,0,1);
 assert(env.frame==1);time+=90;ru_panel_update(&env,time,0,1);
 assert(env.frame==0);
 /* Repeated keydown during LFO reverse resumes from its current state. */
 time=0;lfo=(RuPanelAnim){0};ru_panel_update(&lfo,time,1,0);
 time+=80;ru_panel_update(&lfo,time,1,0);
 time+=20;ru_panel_update(&lfo,time,0,0);
 assert(lfo.phase==RU_PANEL_CLOSE);
 time+=20;ru_panel_update(&lfo,time,1,0);
 assert(lfo.phase==RU_PANEL_OPEN);
 puts("PASS: LFO reverse loop, ENV true reverse release, short note, retrigger");return 0;
}
