from pathlib import Path
R=Path('/mnt/data/rupture-v1861')
S=R/'vendor/etch-v1.88-source/firmware/fm1-native'

def change(path,old,new,limit=1):
    p=S/path; s=p.read_text(); n=s.count(old)
    if n<limit:
        raise RuntimeError(f'{path} expected >= {limit} matches, got {n}: {old[:100]!r}')
    p.write_text(s.replace(old,new,limit))

(S/'rupture_bend_curves.h').write_text('''/* RUPTURE V1.86.1: K3 momentary BEND response curve presets.
 * The control thread computes these bounded integer curves, not the audio ISR.
 * Keeping this independent of patch/storage/OTA avoids a project format change.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_BEND_CURVES_H
#define RUPTURE_BEND_CURVES_H
#include <stdint.h>
#define RU_BEND_CURVE_COUNT 5u
enum {RU_BEND_LINEAR=0u,RU_BEND_SOFT=1u,RU_BEND_HARD=2u,
      RU_BEND_SCURVE=3u,RU_BEND_EXP=4u};
static uint8_t ru_bend_curve_percent(uint8_t curve,uint8_t raw){
 uint32_t x=raw;
 if(x>=100u)return 100u;
 const uint32_t x2=x*x;
 uint32_t y=x;
 switch(curve){
  case RU_BEND_SOFT: y=(200u*x-x2+50u)/100u;break;        /* ease out */
  case RU_BEND_HARD: y=(x2+50u)/100u;break;              /* ease in */
  case RU_BEND_SCURVE: y=(3u*x2*100u-2u*x2*x+5000u)/10000u;break;
  case RU_BEND_EXP: y=(x2*x+5000u)/10000u;break;         /* cubic */
  default:break;                                         /* LINEAR */
 }
 return (uint8_t)(y>100u?100u:y);
}
static const char *const ru_bend_curve_names[RU_BEND_CURVE_COUNT]={
 "LINEAR","SOFT","HARD","S-CURVE","EXP"};
#endif
''')
change('bringup.c','#include "rupture_bend_sprites.h"\n','#include "rupture_bend_sprites.h"\n#include "rupture_bend_curves.h"\n')
change('rupture_ui.c','#include "rupture_bend_sprites.h"\n','#include "rupture_bend_sprites.h"\n#include "rupture_bend_curves.h"\n')
change('bringup.c','static uint8_t ru_bend_target=2u,ru_bend_live=0u,ru_bend_last_visual_frame=0u;',
     'static uint8_t ru_bend_target=2u,ru_bend_live=0u,ru_bend_curve=RU_BEND_LINEAR,ru_bend_last_visual_frame=0u;')
change('bringup.c',' uint8_t idx=ru_bend_target<5u?ru_bend_target:0u;\n uint32_t target=target_q16[idx];\n uint32_t delta=target>65536u?target-65536u:0u;\n return 65536u + (uint32_t)((delta*(uint32_t)ru_bend_live + 50u)/100u);',
     ' uint8_t idx=ru_bend_target<5u?ru_bend_target:0u;\n uint32_t target=target_q16[idx];\n uint32_t delta=target>65536u?target-65536u:0u;\n uint8_t shaped=ru_bend_curve_percent(ru_bend_curve,ru_bend_live);\n return 65536u + (uint32_t)((delta*(uint32_t)shaped + 50u)/100u);')
change('bringup.c',' if(!ru_bend_live)return 0u;\n uint32_t frame=((uint32_t)ru_bend_live*(RU_BEND_FRAMES-1u)+99u)/100u;',
     ' uint8_t shaped=ru_bend_curve_percent(ru_bend_curve,ru_bend_live);\n if(!shaped)return 0u;\n uint32_t frame=((uint32_t)shaped*(RU_BEND_FRAMES-1u)+99u)/100u;')
change('bringup.c','v.bend_live=ru_bend_live;v.bend_target=ru_bend_target;v.bend_frame=ru_bend_frame_from_live();',
     'v.bend_live=ru_bend_live;v.bend_target=ru_bend_target;v.bend_curve=ru_bend_curve;v.bend_shaped=ru_bend_curve_percent(ru_bend_curve,ru_bend_live);v.bend_frame=ru_bend_frame_from_live();')
change('bringup.c','case RU_PAGE_BEND:v.k[0]=ru_bend_live;v.k[1]=ru_bend_target;v.k[2]=0u;v.k[3]=0u;break;',
     'case RU_PAGE_BEND:v.k[0]=ru_bend_live;v.k[1]=ru_bend_target;v.k[2]=ru_bend_curve;v.k[3]=0u;break;')
change('bringup.c',
'''  if(control==1u)ru_bend_target=(uint8_t)ui_bound((int32_t)ru_bend_target+amount,0,4);
 }
 else if(ru_page==RU_PAGE_TILT)''',
'''  if(control==1u)ru_bend_target=(uint8_t)ui_bound((int32_t)ru_bend_target+amount,0,4);
  if(control==2u)ru_bend_curve=(uint8_t)ui_bound((int32_t)ru_bend_curve+amount,0,RU_BEND_CURVE_COUNT-1u);
 }
 else if(ru_page==RU_PAGE_TILT)''')
# A play gesture must not mark the project as unsaved.
change('bringup.c','ru_page!=RU_PAGE_GLO && ru_page!=RU_PAGE_HELP && ru_page!=RU_PAGE_MENU && ru_page!=RU_PAGE_SETTINGS && ru_page!=RU_PAGE_SAVER_OPTIONS && ru_page!=RU_PAGE_AUTOLOAD)ru_mark_dirty();',
     'ru_page!=RU_PAGE_GLO && ru_page!=RU_PAGE_HELP && ru_page!=RU_PAGE_MENU && ru_page!=RU_PAGE_SETTINGS && ru_page!=RU_PAGE_SAVER_OPTIONS && ru_page!=RU_PAGE_AUTOLOAD && ru_page!=RU_PAGE_BEND)ru_mark_dirty();')
change('rupture_ui.c','uint8_t bend_frame,bend_target,bend_live,bend_releasing;',
     'uint8_t bend_frame,bend_target,bend_live,bend_shaped,bend_curve,bend_releasing;')
change('rupture_ui.c','case RU_PAGE_BEND:if(i==1)val=val*25u;break;',
     'case RU_PAGE_BEND:if(i==1||i==2)val=val*25u;break;')
change('rupture_ui.c','{"BEND","TARGET","",""},','{"BEND","TARGET","CURVE",""},')
# Native screen: three functional knobs, real shaped offset + tiny plot.
start=(S/'rupture_ui.c').read_text()
i=start.index(' if(page==RU_PAGE_BEND){',start.index('static void ru_render_raw'))
j=start.index(' if(page==RU_PAGE_SUB1||page==RU_PAGE_SUB2){',i)
new=''' if(page==RU_PAGE_BEND){
  /* On-device 240x240 view. The mountain ALWAYS comes from the unchanged
   * prepacked 1-bit sprite frames. The K3 curve preview is a tiny static
   * response graph calculated only during a UI redraw, never in the ISR. */
  const uint8_t frame=v->bend_frame<RU_BEND_FRAMES?v->bend_frame:0u;
  const uint8_t target=v->bend_target<5u?v->bend_target:2u;
  const uint8_t curve=v->bend_curve<RU_BEND_CURVE_COUNT?v->bend_curve:RU_BEND_LINEAR;
  const uint32_t strength=v->bend_live<=100u?v->bend_live:100u;
  const uint32_t shaped=ru_bend_curve_percent(curve,(uint8_t)strength);
  static const char *const range_names[5]={"1/4","1/2","FULL","1 1/2","DOUBLE"};
  static const uint16_t bend_cents[5]={25u,50u,100u,150u,200u};
  ru_panel_blit_shaken(ru_bend_sprites[frame],0,0);
  ru_center("K2 / BEND RANGE",120,88,1);
  for(int i=0;i<5;i++){
   const int x=10+i*44;
   if(i==(int)target){ru_frame(x,98,42,24);ru_rect(x+3,117,36,3);}
   else ru_frame(x,100,42,20);
   ru_center(range_names[i],x+21,105,1);
  }
  /* The live readout uses the curved pitch, not the raw K1 position. */
  uint32_t cents=(shaped*bend_cents[target]+50u)/100u;
  char pitch[16],whole[8];
  ru_uint(cents/100u,whole);
  int w=0;pitch[w++]='+';
  for(int k=0;whole[k]&&w<7;k++)pitch[w++]=whole[k];
  pitch[w++]='.';
  pitch[w++]=(char)('0'+(cents%100u)/10u);
  pitch[w++]=(char)('0'+cents%10u);
  pitch[w++]=' ';pitch[w++]='S';pitch[w++]='T';pitch[w]=0;
  /* Read-only miniature curve graph: X = raw K1 turn; Y = actual bend.
   * The marker shows the current instant on the selected preset curve. */
  ru_frame(10,124,41,21);
  ru_line(14,140,46,140);ru_line(14,140,14,128);
  for(int q=1;q<=16;q++){
   int x0=14+(q-1)*2,x1=14+q*2;
   uint8_t prev=ru_bend_curve_percent(curve,(uint8_t)((q-1)*100/16));
   uint8_t next=ru_bend_curve_percent(curve,(uint8_t)(q*100/16));
   ru_line(x0,140-(int)prev*12/100,x1,140-(int)next*12/100);
  }
  ru_rect(14+(int)strength*32/100,140-(int)shaped*12/100,2,2);
  ru_text("LIVE",56,131,1);
  ru_text(pitch,227-ru_strlen(pitch)*6,131,1);
  ru_frame(11,147,218,12);
  if(shaped)ru_rect(14,150,(int)((shaped*212u+50u)/100u),6);
  for(int k=1;k<4;k++)ru_rect(14+k*53,148,1,3);
  ru_text("0",12,161,1);ru_text("100%",203,161,1);
  const char *state=strength?(v->bend_releasing?"RETURN":"BENDING"):"IDLE";
  ru_center(state,120,162,1);
  /* Three compact controls: K1 momentary, K2 distance, K3 curve. */
  static const int bx[3]={39,120,201};
  for(int k=0;k<3;k++){
   ru_knob(bx[k],191,v->animate?v->knob_visual[k]:ru_knob_normal(v,k));
   ru_knob_focus_marker(v,bx[k],191,(uint8_t)k);
  }
  ru_center("K1 BEND",39,209,1);
  char percent[12];ru_uint(strength,percent);
  int plen=ru_strlen(percent);if(plen<9){percent[plen]='%';percent[plen+1]=0;}
  ru_center(percent,39,218,1);
  ru_center("K2 RANGE",120,209,1);
  ru_center(range_names[target],120,218,1);
  ru_center("K3 CURVE",201,209,1);
  ru_center(ru_bend_curve_names[curve],201,218,1);
  ru_footer("STOP K1 = RELEASE");return;
 }
'''
(S/'rupture_ui.c').write_text(start[:i]+new+start[j:])
# Host exporter variants, still same renderer.
change('rupture_host_export.c',
'''if(v.page==RU_PAGE_BEND){v.bend_target=2u;v.bend_live=62u;v.bend_frame=5u;v.bend_releasing=0u;v.k[0]=62u;v.k[1]=2u;v.k[2]=0u;v.k[3]=0u;}''',
'''if(v.page==RU_PAGE_BEND){
 v.bend_target=2u;v.bend_live=62u;v.bend_curve=RU_BEND_LINEAR;
 if(argc>=4 && strncmp(argv[3],"bend",4)==0){
  int mode=atoi(argv[3]+4);if(mode>=0&&mode<(int)RU_BEND_CURVE_COUNT)v.bend_curve=(uint8_t)mode;
 }
 v.bend_shaped=ru_bend_curve_percent(v.bend_curve,v.bend_live);
 v.bend_frame=(uint8_t)((v.bend_shaped*(RU_BEND_FRAMES-1u)+99u)/100u);
 v.bend_releasing=0u;v.k[0]=62u;v.k[1]=2u;v.k[2]=v.bend_curve;v.k[3]=0u;
}''')
# Version reporting: only true release-relevant identifiers, not historical docs.
change('rupture_ui.c','{"V1.86","FM-1_968",','{"V1.86.1","FM-1_968",')
change('build.py',"'version':'RUPTURE V1.86'","'version':'RUPTURE V1.86.1'")
print('K3 curve patch applied to',S)
