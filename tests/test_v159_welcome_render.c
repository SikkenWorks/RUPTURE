#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
int main(int argc,char **argv){
 if(argc!=3)return 2;
 int phase=atoi(argv[1]);if(phase<1||phase>9)return 2;
 RuState v={0};v.page=RU_PAGE_WELCOME;v.welcome_phase=(uint8_t)phase;
 v.welcome_choice=(uint8_t)(phase%2);v.welcome_crt=1;v.welcome_store_ready=1;
 v.welcome_hold_pct=45;v.welcome_status=1;
 ru_render(&v);
 FILE *f=fopen(argv[2],"wb");if(!f)return 3;
 fputs("P5\n240 240\n255\n",f);
 for(int y=0;y<240;y++)for(int x=0;x<240;x++){
  uint32_t i=(uint32_t)y*240u+(uint32_t)x;
  uint8_t px=(ru_bits[i>>3]&(0x80u>>(i&7)))?255:0;
  fwrite(&px,1,1,f);
 }
 fclose(f);return 0;
}
