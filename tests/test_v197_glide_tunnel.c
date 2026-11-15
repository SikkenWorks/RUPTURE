/* V1.97: GLIDE visual frames and found-footage jitter only. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"

static void assert_outside_panel_unchanged(const uint8_t *a,const uint8_t *b){
 for(unsigned row=0;row<240u;row++){
  if(row>=RU_PANEL_Y && row<RU_PANEL_Y+RU_PANEL_HEIGHT)continue;
  assert(memcmp(a+row*30u,b+row*30u,30u)==0);
 }
}

int main(void){
 assert(RU_TUNNEL_FRAMES==12u);
 assert(RU_PANEL_WIDTH==224u && RU_PANEL_HEIGHT==30u);
 RuState v={0};v.page=RU_PAGE_GLIDE;v.k[0]=30u;v.k[1]=1u;
 uint8_t original[7200],moving[7200];
 ru_render(&v);
 memcpy(original,ru_bits,sizeof(original));
 /* Exact sprite bits, no blending, interpolation or 1-bit quality loss. */
 for(unsigned y=0;y<RU_PANEL_HEIGHT;y++)for(unsigned x=0;x<RU_PANEL_WIDTH;x++){
  int actual=ru_get((int)RU_PANEL_X+(int)x,(int)RU_PANEL_Y+(int)y);
  int expected=(ru_tunnel_sprites[0][y][x>>3u]>>(7u-(x&7u)))&1u;
  assert(actual==expected);
 }
 v.glide_frame=11u;ru_render(&v);
 memcpy(moving,ru_bits,sizeof(moving));
 assert(memcmp(original,moving,sizeof(original))!=0);
 assert_outside_panel_unchanged(original,moving);
 /* A different camera origin changes the picture, never labels/knobs. */
 v.glide_frame=0u;v.glide_shake=3u;ru_render(&v);
 assert(memcmp(original,ru_bits,sizeof(original))!=0);
 assert_outside_panel_unchanged(original,ru_bits);
 /* The tunnel state must have zero visual effects on other pages. */
 v.page=RU_PAGE_PLAY;v.glide_frame=0u;v.glide_shake=0u;ru_render(&v);
 memcpy(original,ru_bits,sizeof(original));
 v.glide_frame=11u;v.glide_shake=7u;ru_render(&v);
 assert(memcmp(original,ru_bits,sizeof(original))==0);
 puts("PASS: GLIDE 224x30 tunnel, 12 ROM frames, POV shake; other pages untouched");
 return 0;
}
