/* Render the REAL FM1 native 240x240 expression SETTINGS page to PGM.
 * No mockup fonts, scaling, invented icons or modified LCD driver. */
#include <stdio.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
static int write_frame(const char *path){
 FILE *f=fopen(path,"wb");if(!f)return 1;
 fprintf(f,"P5\n240 240\n255\n");
 for(uint32_t i=0u;i<240u*240u;i++){
  uint8_t px=(ru_bits[i>>3]&(0x80u>>(i&7u)))?255u:0u;
  if(fwrite(&px,1u,1u,f)!=1u){fclose(f);return 2;}
 }
 return fclose(f)!=0;
}
static int shot(const char *path,uint8_t engine){
 RuState v={0};v.page=RU_PAGE_SETTINGS;v.settings_open=6u;
 v.touch_engine=engine;v.touch_field=engine?2u:3u;
 v.touch_target=5u; /* KEY PRESSURE target REVERB MIX, not FX type ID. */
 v.touch_mode=engine?2u:0u;
 v.touch_min=engine?10u:25u;v.touch_max=engine?90u:80u;
 v.touch_sensitivity=engine?3u:1u;v.touch_rate=engine?2u:1u;
 v.touch_live=engine?67u:31u;v.touch_enabled_mask=3u;
 v.touch_view_active=1u;v.touch_raw=engine?0u:492u;
 ru_render(&v);
 return write_frame(path);
}
static int reverb_shot(const char *path){
 RuState v={0};v.page=RU_PAGE_REVERB;v.fx_selected=0u;
 v.k[0]=1u;v.k[1]=67u;v.k[2]=56u;v.k[3]=0u;
 v.fx_mod_active=1u;v.fx_mod_mode=RU_TOUCH_MODE_PRESS;
 v.fx_mod_min=0u;v.fx_mod_max=80u;v.fx_mod_live=54u;
 ru_render(&v);return write_frame(path);
}
static int midi_velocity_shot(const char *path){
 RuState v={0};v.page=RU_PAGE_SETTINGS;v.settings_open=6u;
 v.touch_engine=0u;v.touch_field=2u;v.touch_target=5u;
 v.touch_mode=RU_TOUCH_MODE_VELOCITY;v.touch_sensitivity=1u;
 v.touch_min=0u;v.touch_max=80u;v.touch_live=44u;
 v.touch_enabled_mask=3u;v.touch_view_active=1u;
 v.touch_midi_source=1u;v.touch_midi_raw=70u;
 ru_render(&v);return write_frame(path);
}
int main(int argc,char **argv){
 if(argc!=5)return 3;
 return shot(argv[1],0u)||shot(argv[2],1u)||reverb_shot(argv[3])||
  midi_velocity_shot(argv[4])?4:0;
}
