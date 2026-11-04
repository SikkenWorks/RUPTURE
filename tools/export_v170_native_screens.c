/* RUPTURE V1.70: export actual native 240x240 rendered screens, not mockups.
 * Usage: gcc -O2 ... -o export && export /output/dir
 * SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static int shot(const char *folder,const char *name,uint8_t page,uint8_t hold){
 char path[1024];int n=snprintf(path,sizeof path,"%s/%s.pgm",folder,name);
 if(n<=0||n>=(int)sizeof path)return 1;
 RuState v={0};v.page=page;v.panel_frame=12u;v.panel_hold_active=1u;
 v.panel_hold_frame=hold;v.k[0]=57u;v.k[1]=50u;v.k[2]=68u;v.k[3]=75u;
 ru_render(&v);
 FILE *f=fopen(path,"wb");if(!f)return 2;
 fprintf(f,"P5\n240 240\n255\n");
 for(uint32_t i=0;i<240u*240u;i++){
  uint8_t px=ru_bits[i>>3]&(0x80u>>(i&7u))?255u:0u;
  fwrite(&px,1u,1u,f);
 }
 fclose(f);return 0;
}
int main(int argc,char **argv){
 if(argc!=2)return 3;
 if(shot(argv[1],"env-smoke-start",RU_PAGE_ENV,0u) ||
    shot(argv[1],"env-smoke-mid",RU_PAGE_ENV,3u) ||
    shot(argv[1],"env-smoke-settled",RU_PAGE_ENV,7u) ||
    shot(argv[1],"lfo-dirt-frame-01",RU_PAGE_LFO,0u) ||
    shot(argv[1],"lfo-dirt-frame-05",RU_PAGE_LFO,4u))return 4;
 return 0;
}
