#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rupture_ui.c"
#include "rupture_eq2.c"
#include "rupture_octave.h"
int main(int argc,char **argv){if(argc!=3 && argc!=4)return 2;RuState v={0};v.page=(uint8_t)atoi(argv[1]);v.octave=rupture_octave_display(RUPTURE_BASS_OCTAVE);v.show_battery=1;v.battery=75;v.usb_power=1;v.fx_selected=0;v.fx_on=1;v.sub_blend=75;v.eq_on=1;v.project=1;v.seq_slot=1;v.seq_present=1;v.seq_status=0;v.root=9;v.mode=1;v.keylock=1;v.filter=4;v.master=100;v.macro[0]=57;v.macro[1]=40;v.k[0]=57;v.k[1]=50;v.k[2]=68;v.k[3]=75;if(v.page==RU_PAGE_SEQ){v.seq_length=16;v.seq_bpm=120;v.seq_current_step=3;v.seq_grid_mask=0x25a9;v.seq_note_count=1;v.seq_first_note=36;v.k[0]=16;v.k[1]=120;v.k[2]=110;v.k[3]=75;}if(v.page==RU_PAGE_ARP){v.k[0]=0;v.k[1]=2;v.k[2]=1;v.k[3]=75;}if(v.page==RU_PAGE_SETTINGS)v.k[0]=100;
 if(v.page==RU_PAGE_OTHER||v.page==RU_PAGE_SAVER_OPTIONS){v.saver_enabled=1u;v.saver_delay_index=1u;}
 if(v.page==RU_PAGE_STORAGE_PROJECTS||v.page==RU_PAGE_STORAGE_ACTIONS){
  v.project_mask=7u;v.manager_project=1u;v.manager_action=1u;
  for(int i=0;i<3;i++){const char *names[]={"FIRST LIGHT","DARK BASS","RUPTURE"};
   for(int j=0;j<12;j++)v.project_names[i][j]=j<(int)strlen(names[i])?names[i][j]:' ';
   v.project_names[i][12]=0;
  }
 }
 if(v.page==RU_PAGE_STORAGE_SEQUENCES||v.page==RU_PAGE_STORAGE_SEQ_ACTIONS){
  v.manager_seq_mask=0x0bu;v.manager_seq=1u;v.manager_action=2u;
  for(int i=0;i<4;i++){const char *names[]={"VERSE","CHORUS","BRIDGE","OUTRO"};
   for(int j=0;j<12;j++)v.manager_seq_names[i][j]=j<(int)strlen(names[i])?names[i][j]:' ';
   v.manager_seq_names[i][12]=0;
  }
 }if(v.page==RU_PAGE_EQ){v.k[0]=125;v.k[1]=75;v.k[2]=110;v.k[3]=20;}if(v.page==RU_PAGE_FX){v.k[0]=1;v.k[1]=61;v.k[2]=80;v.k[3]=35;}if(v.page==RU_PAGE_GUIDE||v.page==RU_PAGE_KEYLOCK){v.k[0]=9;v.k[1]=1;v.k[2]=1;}if(v.page==RU_PAGE_SUB1){v.k[0]=0;v.k[1]=1;v.k[2]=50;v.k[3]=0;}
if(v.page==RU_PAGE_SUB2){v.k[0]=0;v.k[1]=1;v.k[2]=54;v.k[3]=0;}
if(v.page==RU_PAGE_SHAPE){v.k[0]=72;v.k[1]=64;v.k[2]=30;v.k[3]=42;}if(v.page==RU_PAGE_COMP){v.k[0]=74;v.k[1]=65;v.k[2]=30;v.k[3]=60;}
if(v.page==RU_PAGE_FOLDER){v.k[0]=62;v.k[1]=50;v.k[2]=84;v.k[3]=72;}
if(v.page==RU_PAGE_TRANSIENT){v.k[0]=72;v.k[1]=52;v.k[2]=63;v.k[3]=70;}
if(v.page==RU_PAGE_TILT){v.k[0]=67;v.k[1]=50;v.k[2]=71;v.k[3]=50;}
if(v.page==RU_PAGE_FUZZ){v.k[0]=75;v.k[1]=90;v.k[2]=75;v.k[3]=78;}
if(v.page==RU_PAGE_SANDPAPER){v.k[0]=65;v.k[1]=40;v.k[2]=50;v.k[3]=80;}
if(v.page==RU_PAGE_NOTE_MEMORY){v.k[0]=0;v.k[1]=1;v.k[2]=0;v.k[3]=2;}if(v.page==RU_PAGE_PROJECTS){v.selected=1;v.project_mask=0x0007u;for(int i=0;i<3;i++){const char *n=i==0?"BASS HOUSE":i==1?"HEAVY LEAD":"DUB MONO";for(int j=0;j<12;j++)v.project_names[i][j]=j<(int)strlen(n)?n[j]:' ';v.project_names[i][12]=0;}}
if(v.page==RU_PAGE_SEQ_SLOTS){v.selected=1;v.seq_mask=0x0bu;for(int i=0;i<4;i++){const char *n=i==0?"INTRO":i==1?"VERSE":i==2?"BREAK": "OUTRO";for(int j=0;j<12;j++)v.seq_names[i][j]=j<(int)strlen(n)?n[j]:' ';v.seq_names[i][12]=0;}}
if(v.page==RU_PAGE_PROJECTS){v.project=2u;v.active_project=1u;}
if(v.page==RU_PAGE_SEQ_SLOTS){v.seq_slot=2u;v.active_seq_slot=1u;}
if(v.page==RU_PAGE_NAME){const char *n="MIDNIGHT BASS";for(int j=0;j<12;j++)v.edit_name[j]=n[j];v.edit_name[12]=0;v.edit_cursor=4;}
if(v.page==RU_PAGE_DIAG){v.diag_wall=62;v.diag_dsp=58;v.diag_render=52;v.diag_copy=9;v.diag_slack=38;v.diag_late=0;v.diag_guard=0;v.diag_scope=137;}
if(v.page==RU_PAGE_LOADING){v.show_battery=0;v.boot_step=argc>=4?(uint8_t)atoi(argv[3]):10;}if(strcmp(argv[1],"storage-save")==0)ru_storage_popup(1u,2u,65u,0u);
else if(strcmp(argv[1],"storage-load")==0)ru_storage_popup(2u,4u,65u,0u);
else if(strncmp(argv[1],"eq2",3)==0){
 v.page=RU_PAGE_EQ;v.eq_mode=1u;
 v.eq_tab=strstr(argv[1],"freq")?1u:strstr(argv[1],"width")?2u:0u;
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1);
 v.eq_freq[0]=30u;v.eq_freq[1]=50u;v.eq_freq[2]=50u;v.eq_freq[3]=50u;
 v.eq_q[0]=9u;v.eq_q[1]=47u;
 for(int i=0;i<4;i++)p.freq[i]=v.eq_freq[i];
 for(int i=0;i<2;i++)p.q[i]=v.eq_q[i];
 uint16_t gains[4]={25486u,11000u,29900u,20000u};
 for(int i=0;i<4;i++)v.eq_gain_index[i]=(uint8_t)rupture_eq2_gain_index(gains[i]);
 struct rupture_eq2_bq coeff[4];rupture_eq2_compute(&p,gains,coeff);
 rupture_eq2_response_curve(coeff,v.eq_curve);
 ru_render(&v);
}
else if(strcmp(argv[1],"audio-fault")==0){v.page=RU_PAGE_SAFE;v.audio_fault=1u;ru_render(&v);}
else if(strcmp(argv[1],"saver")==0){v.saver_active=1u;v.saver_frame=argc>=4?(uint8_t)atoi(argv[3]):32u;ru_render(&v);}
else ru_render(&v);FILE *f=fopen(argv[2],"wb");if(!f)return 3;fprintf(f,"P5\n240 240\n255\n");for(int y=0;y<240;y++)for(int x=0;x<240;x++)fputc(ru_get(x,y)?255:0,f);fclose(f);return 0;}
