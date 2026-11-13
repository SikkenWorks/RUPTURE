/* RUPTURE BEND UI screenshot generator, host-only, same native 1-bit renderer.
 * Usage: render_bend_ui target_index strength releasing crt_on OUTPUT.ppm */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "rupture_ui.c"
#include "rupture_eq2.c"
int main(int argc,char **argv){
 if(argc!=6)return 2;
 RuState v={0};
 int ti=atoi(argv[1]),strength=atoi(argv[2]),rel=atoi(argv[3]);
 if(ti<0||ti>4||strength<0||strength>100)return 3;
 v.page=RU_PAGE_BEND;v.show_battery=1u;v.battery=80u;
 v.crt_enabled=(uint8_t)!!atoi(argv[4]);
 v.bend_target=(uint8_t)ti;v.bend_live=(uint8_t)strength;
 v.bend_releasing=(uint8_t)!!rel;
 v.bend_frame=(uint8_t)(strength?((unsigned)strength*(RU_BEND_FRAMES-1u)+99u)/100u:0u);
 v.k[0]=(uint16_t)strength;v.k[1]=(uint16_t)ti;
 ru_render(&v);
 FILE *f=fopen(argv[5],"wb");if(!f)return 4;
 fprintf(f,"P6\n240 240\n255\n");
 for(uint32_t y=0;y<240u;y++)for(uint32_t x=0;x<240u;x++){
  uint16_t c=ru_crt_pixel(ru_bits,x,y,v.crt_enabled);
  unsigned char rgb[3]={
   (unsigned char)(((c>>11)&31u)*255u/31u),
   (unsigned char)(((c>>5)&63u)*255u/63u),
   (unsigned char)((c&31u)*255u/31u)};
  fwrite(rgb,1u,3u,f);
 }
 fclose(f);return 0;
}
