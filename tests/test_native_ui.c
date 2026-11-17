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
 /* V2.0.1: original art stays byte-exact in every other frame. The eleven
    transitions must also differ from both adjacent original keyframes. */
 const uint32_t expected[12]={0xd691cef7u,0x59831236u,0xe834d223u,0xc14fdc20u,
  0xfd2c317eu,0x4f9c3a98u,0xde73e8a8u,0xba60a873u,0xd6c48935u,
  0x08acfa7eu,0xbf216867u,0xdd2e6ce1u};
 assert(RU_SPLASH_STEPS==23u);
 assert(RU_V2_BREACH_DURATION_MS==1770u);
 assert(ru_v2_breach_frame_at(0u)==0u);
 assert(ru_v2_breach_frame_at(34u)==0u);
 assert(ru_v2_breach_frame_at(35u)==1u);
 assert(ru_v2_breach_frame_at(769u)==21u);
 assert(ru_v2_breach_frame_at(770u)==22u);
 assert(ru_v2_breach_frame_at(1770u)==22u);
 v.boot_step=RU_SPLASH_STEPS;ru_render(&v);
 unsigned int changed=0;unsigned int earlier=0;
 uint32_t prior_hash=0u;
 for(unsigned int f=1;f<=RU_SPLASH_STEPS;f++){
  v.boot_step=(uint8_t)f;ru_render(&v);
  uint32_t hash=2166136261u;
  unsigned int visible=0;
  for(unsigned int i=0;i<7200u;i++){
   visible+=(unsigned)__builtin_popcount((unsigned)ru_bits[i]);
   hash=(hash^ru_bits[i])*16777619u;
  }
  if(f&1u)assert(hash==expected[(f-1u)/2u]);
  else assert(hash!=expected[(f-2u)/2u] && hash!=expected[f/2u]);
  if(f>1u && hash!=prior_hash)changed++;
  prior_hash=hash;
  if(f==1u)assert(visible>0u && visible<1000u);
  if(f==11u)earlier=visible;
 }
 assert(changed==22u && earlier>0u);
 puts("PASS: RUPTURE OCT 0 bass reference, octave LEDs, pitch unchanged, 101-position knobs and native screen");
 return 0;
}
