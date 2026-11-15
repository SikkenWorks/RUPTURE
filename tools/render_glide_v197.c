/* Export the actual native 240x240 GLIDE framebuffer for visual inspection.
 * Run: gcc -O2 -Wno-unused-function tools/render_glide_v197.c -o /tmp/glide-preview
 *      /tmp/glide-preview /tmp/glide-preview-00.pgm 0 0
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"

int main(int argc,char **argv){
 if(argc!=4)return 2;
 RuState v={0};
 v.page=RU_PAGE_GLIDE;
 v.k[0]=35u;v.k[1]=2u;v.k[2]=0u;v.k[3]=1u;
 v.glide_frame=(uint8_t)(atoi(argv[2])%RU_TUNNEL_FRAMES);
 v.glide_shake=(uint8_t)(atoi(argv[3])&7);
 ru_render(&v);
 FILE *fp=fopen(argv[1],"wb");if(!fp)return 3;
 fputs("P5\n240 240\n255\n",fp);
 for(int y=0;y<240;y++)for(int x=0;x<240;x++){
  uint8_t pixel=ru_get(x,y)?255u:0u;
  if(fwrite(&pixel,1,1,fp)!=1u){fclose(fp);return 4;}
 }
 fclose(fp);
 return 0;
}
