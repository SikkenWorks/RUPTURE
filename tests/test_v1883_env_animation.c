/* V1.88.3 ENV: exact release reversal, no frozen smoke or healed wall.
 * Native state machine and the real compiled ROM frame arrays. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_panel_anim.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_panel_sprites.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_hold_sprites.h"

static uint8_t at(const uint8_t data[30][28], unsigned x, unsigned y){
 return (uint8_t)((data[y][x/8] & (0x80u >> (x%8))) != 0u);
}

static void all_frames_and_static_hole(void){
 assert(sizeof(ru_env_sprites)==20u*30u*28u);
 assert(sizeof(ru_env_hold_sprites)==8u*30u*28u);
 for(unsigned k=0;k<8;k++)
  assert(memcmp(ru_env_hold_sprites[k],ru_env_sprites[12+k],840u)==0);
 for(unsigned k=0;k<7;k++)
  assert(memcmp(ru_env_hold_sprites[k],ru_env_hold_sprites[k+1],840u)!=0);
 /* The cavity is black, surrounded by masonry. It does not repair itself. */
 for(unsigned x=107;x<120;x++)assert(!at(ru_env_sprites[19],x,20));
 assert(at(ru_env_sprites[19],90,18));
 assert(memcmp(ru_env_sprites[19],ru_env_sprites[0],840u)!=0);
}

static void reverse_from_full_hold(void){
 RuPanelAnim a={0};uint32_t now=1000;
 assert(ru_panel_update(&a,now,1,1));
 assert(a.frame==1 && a.phase==RU_PANEL_OPEN);
 for(unsigned n=0;n<11;n++){
  now+=80;assert(ru_panel_update(&a,now,1,1));
  assert(a.frame==n+2 && a.phase==RU_PANEL_OPEN);
 }
 now+=80;assert(ru_panel_update(&a,now,1,1));
 assert(a.phase==RU_PANEL_HOLD && a.frame==12 && a.hold_frame==0);
 for(unsigned n=1;n<8;n++){
  now+=150;assert(ru_panel_update(&a,now,1,1));
  assert(a.phase==RU_PANEL_HOLD && a.hold_frame==n);
 }
 for(unsigned n=0;n<40;n++){
  now+=220;ru_panel_update(&a,now,1,1);
  assert(a.phase==RU_PANEL_HOLD && a.hold_frame==7);
 }
 assert(ru_panel_update(&a,now+1,0,1));now++;
 assert(a.phase==RU_PANEL_CLOSE && a.frame==19);
 for(unsigned expected=18;expected>0;expected--){
  now+=90;assert(ru_panel_update(&a,now,0,1));
  assert(a.phase==RU_PANEL_CLOSE && a.frame==expected);
 }
 now+=90;assert(ru_panel_update(&a,now,0,1));
 assert(a.phase==RU_PANEL_IDLE && a.frame==0);
}

static void release_from_each_hold_step(void){
 for(unsigned release_step=0;release_step<8;release_step++){
  RuPanelAnim a={0};uint32_t now=0;
  ru_panel_update(&a,now,1,1);
  for(unsigned n=0;n<12;n++){now+=80;ru_panel_update(&a,now,1,1);}
  assert(a.phase==RU_PANEL_HOLD);
  for(unsigned n=0;n<release_step;n++){now+=150;ru_panel_update(&a,now,1,1);}
  assert(a.hold_frame==release_step);
  now+=1;ru_panel_update(&a,now,0,1);
  assert(a.phase==RU_PANEL_CLOSE && a.frame==12+release_step);
  /* An immediate retrigger resumes the same artwork, not a smoky restart. */
  now+=1;ru_panel_update(&a,now,1,1);
  assert(a.phase==RU_PANEL_HOLD && a.frame==12 && a.hold_frame==release_step);
 }
}

static void release_while_opening(void){
 for(unsigned open_step=1;open_step<12;open_step++){
  RuPanelAnim a={0};uint32_t now=0;
  ru_panel_update(&a,now,1,1);
  for(unsigned n=1;n<open_step;n++){now+=80;ru_panel_update(&a,now,1,1);}
  assert(a.frame==open_step);
  now+=1;ru_panel_update(&a,now,0,1);
  assert(a.phase==RU_PANEL_CLOSE && a.frame==open_step);
  now+=90;ru_panel_update(&a,now,0,1);
  assert(a.frame==open_step-1);
 }
}

int main(void){
 all_frames_and_static_hole();
 reverse_from_full_hold();
 release_from_each_hold_step();
 release_while_opening();
 puts("PASS V1.88.3 ENV: 20-frame native ROM; 8 hold parity; persistent hole; exact reverse and retrigger");
 return 0;
}
