/* Export exact native 240x240 1-bit UI images for visual QA. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static void export_screen(const char *filename){
 FILE *f=fopen(filename,"wb");if(!f)return;
 fputs("P5\n240 240\n255\n",f);
 for(int y=0;y<240;y++)for(int x=0;x<240;x++)fputc(ru_get(x,y)?255:0,f);
 fclose(f);
}
int main(int argc,char**argv){
 if(argc!=2)return 2;
 RuState v={0};v.page=RU_PAGE_PLAY;v.main_wave=0;v.show_battery=1;v.battery=67;
 v.k[0]=95;v.k[1]=51;v.k[2]=52;v.k[3]=68;v.project_dirty=1;v.dam_frame=0;
 ru_render(&v);char path[512];snprintf(path,sizeof path,"%s/dirty-play.pgm",argv[1]);export_screen(path);
 v.page=RU_PAGE_PROJECTS;v.project=2;v.active_project=1;v.project_mask=3;
 strncpy(v.project_names[0],"MY BASS",12);strncpy(v.project_names[1],"TIGHT SUB",12);
 v.project_status=5;ru_render(&v);snprintf(path,sizeof path,"%s/unsaved-open.pgm",argv[1]);export_screen(path);
 v.page=RU_PAGE_STORAGE_ACTIONS;v.manager_action=0;v.manager_status=4;
 ru_render(&v);snprintf(path,sizeof path,"%s/unsaved-storage.pgm",argv[1]);export_screen(path);
 return 0;
}
