/* Export real 1-bit native tutorial screenshots (host-only; no device calls). */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
int main(int argc,char **argv){
 if(argc!=3)return 2;
 int lesson=atoi(argv[1]);
 if(lesson<-2||(lesson>=0 && lesson>=(int)RU_TUTORIAL_COUNT))return 2;
 RuState v={0};v.show_battery=1;v.battery=83;
 v.welcome_phase=lesson==-1?RU_WELCOME_TOUR_ASK:lesson>=0?RU_WELCOME_TOUR:RU_WELCOME_OFF;
 v.welcome_overlay=1;v.welcome_choice=1;
 if(lesson>=0){
  const RuTutorialLesson *t=&ru_tutorial_lessons[lesson];
  v.page=t->page;v.fx_selected=t->fx;v.selected=t->selected;
  v.settings_open=t->settings_open;v.welcome_lesson=(uint8_t)lesson;
  v.seq_length=16;v.seq_bpm=120;v.seq_first_note=255;
  v.panel_frame=12;v.panel_hold_active=1;v.panel_hold_frame=7;
  v.dam_frame=11;
  for(int i=0;i<4;i++)v.k[i]=60;
 }else v.page=lesson==-1?RU_PAGE_WELCOME:RU_PAGE_GLO;
 ru_render(&v);
 FILE *f=fopen(argv[2],"wb");if(!f)return 3;
 fputs("P5\n240 240\n255\n",f);
 for(int y=0;y<240;y++)for(int x=0;x<240;x++){
  uint32_t z=(uint32_t)y*240u+(uint32_t)x;
  unsigned char p=(ru_bits[z>>3]&(0x80u>>(z&7)))?255u:0u;
  fwrite(&p,1,1,f);
 }
 fclose(f);return 0;
}
