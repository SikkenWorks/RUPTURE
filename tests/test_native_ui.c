#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_octave.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
int main(void){
 int8_t o=RUPTURE_BASS_OCTAVE;
 assert(o==-2);assert(rupture_octave_display(o)==0);
 assert(!rupture_octave_led(0u,o,0u) && !rupture_octave_led(1u,o,0u));
 assert(rupture_octave_led(0u,o,1u) && !rupture_octave_led(1u,o,0u));
 assert(rupture_octave_led(1u,o,2u));
 assert(53+12*(int)o==29); /* same bass-note pitch, no transposition */
 o=rupture_octave_on_press(o,2u);assert(o==-1);assert(rupture_octave_display(o)==1);assert(rupture_octave_led(1u,o,0u));
 o=rupture_octave_on_press(o,2u);assert(o==0);assert(rupture_octave_display(o)==2);
 o=rupture_octave_on_press(o,1u);assert(o==-1);assert(rupture_octave_display(o)==1);
 assert(rupture_octave_on_press(-4,1u)==-4);
 assert(rupture_octave_on_press(3,2u)==3);assert(rupture_octave_display(3)==5);assert(rupture_octave_display(-4)==-2);
 assert(rupture_octave_on_press(1,3u)==1); /* both keys do nothing */
 /* Full travel: 101 defined positions and exact endpoints. */
 unsigned int unique=0;
 for(int k=0;k<=100;k++){
  assert(ru_needle_xy[k][0] >= -15 && ru_needle_xy[k][0] <= 15);
  assert(ru_needle_xy[k][1] >= -15 && ru_needle_xy[k][1] <= 15);
  if(k && (ru_needle_xy[k][0]!=ru_needle_xy[k-1][0] || ru_needle_xy[k][1]!=ru_needle_xy[k-1][1]))unique++;
 }
 assert(unique>=30);
 assert(ru_needle_xy[0][0]<0&&ru_needle_xy[0][1]>0); /* 7:30 */
 assert(ru_needle_xy[50][0]==0&&ru_needle_xy[50][1]<0); /* 12:00 */
 assert(ru_needle_xy[100][0]>0&&ru_needle_xy[100][1]>0); /* 4:30 */ /* not the old seven-step indicator */
 RuState v={0};v.page=RU_PAGE_PLAY;v.octave=rupture_octave_display(RUPTURE_BASS_OCTAVE);v.show_battery=1;v.battery=-1;
 v.k[0]=40;v.k[1]=50;v.k[2]=60;v.k[3]=70;
 ru_render(&v);assert(ru_get(7,5));
 v.page=RU_PAGE_LOADING;v.boot_step=RU_SPLASH_STEPS;ru_render(&v);
 int pixels=0;for(int y=0;y<240;y++)for(int x=0;x<240;x++)pixels+=ru_get(x,y);
 assert(pixels>400);
 v.boot_step=1;ru_render(&v);int small=0;
 for(int y=0;y<240;y++)for(int x=0;x<240;x++)small+=ru_get(x,y);
 assert(small>0 && small<pixels);
 /* V1.20: final framebuffer must be byte-identical to approved art in ROM;
    intro remains non-empty and different until final (no false "animation"). */
 v.boot_step=RU_SPLASH_STEPS;ru_render(&v);
 for(unsigned int i=0;i<7200u;i++)assert(ru_bits[i]==ru_splash_final_bitmap[i]);
 unsigned int changed=0;unsigned int earlier=0;
 for(unsigned int f=1;f<RU_SPLASH_STEPS;f++){
  v.boot_step=(uint8_t)f;ru_render(&v);
  unsigned int visible=0;
  for(unsigned int i=0;i<7200u;i++){
   visible+=(unsigned)__builtin_popcount((unsigned)ru_bits[i]);
   if(ru_bits[i]!=ru_splash_final_bitmap[i])changed++;
  }
  if(f==1u)assert(visible>0u && visible<1000u);
  if(f==12u)earlier=visible;
 }
 assert(changed>0u && earlier>0u);
 puts("PASS: RUPTURE OCT 0 bass reference, octave LEDs, pitch unchanged, 101-position knobs and native screen");
 return 0;
}
