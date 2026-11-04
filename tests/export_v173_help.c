/* Read-only direct FM1 1-bit HELP frame snapshot for the current release. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
int main(int argc,char **argv){
 if(argc!=2)return 2;
 uint8_t found=0u;
 RuState v={0};v.page=RU_PAGE_HELP;
 for(uint8_t i=0u;i<RU_HELP_CARD_COUNT;i++)if(!strcmp(ru_help_cards[i].title,"LIVE REC CLICK")){
  v.help_page=i;found=1u;break;
 }
 if(!found)return 3;
 ru_render(&v);
 FILE *f=fopen(argv[1],"wb");if(!f)return 4;
 fputs("P5\n240 240\n255\n",f);
 for(int y=0;y<240;y++)for(int x=0;x<240;x++){
  uint32_t z=(uint32_t)y*240u+(uint32_t)x;
  unsigned char px=(ru_bits[z>>3]&(0x80u>>(z&7)))?255u:0u;
  fwrite(&px,1,1,f);
 }
 fclose(f);return 0;
}
