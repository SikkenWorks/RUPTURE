/* RUPTURE V1.70: ENV one-shot settling and LFO endless falling dirt.
 * Runs the ACTUAL native 1-bit renderer plus actual note-gated scheduler.
 * No FM1 hardware or DSP needed; hardware validation remains separate. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_panel_anim.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static uint32_t hash(void){uint32_t h=2166136261u;
 for(size_t i=0;i<sizeof(ru_bits);++i)h=(h^ru_bits[i])*16777619u;
 return h;}
static void render_panel(const RuPanelAnim *a,uint8_t page){
 RuState v={0};v.page=page;v.panel_frame=a->frame;
 v.panel_shake=a->shake;v.panel_hold_frame=a->hold_frame;
 v.panel_hold_active=(uint8_t)(a->phase==RU_PANEL_HOLD);
 ru_render(&v);
}
static void check_rom_sprites(void){
 assert(RU_HOLD_FRAMES==RU_PANEL_HOLD_FRAMES);
 assert(sizeof(ru_env_hold_sprites)==RU_HOLD_FRAMES*RU_PANEL_HEIGHT*RU_PANEL_BYTES);
 assert(sizeof(ru_lfo_hold_sprites)==RU_HOLD_FRAMES*RU_PANEL_HEIGHT*RU_PANEL_BYTES);
 assert(!memcmp(ru_env_hold_sprites[7],ru_env_sprites[12],
                RU_PANEL_HEIGHT*RU_PANEL_BYTES));
 for(unsigned n=0;n<7;n++){
  assert(memcmp(ru_env_hold_sprites[n],ru_env_sprites[12],
                RU_PANEL_HEIGHT*RU_PANEL_BYTES)!=0);
  assert(memcmp(ru_env_hold_sprites[n],ru_env_hold_sprites[n+1],
                RU_PANEL_HEIGHT*RU_PANEL_BYTES)!=0);
 }
 for(unsigned n=0;n<RU_HOLD_FRAMES;n++){
  assert(memcmp(ru_lfo_hold_sprites[n],ru_lfo_hold_sprites[(n+1)%RU_HOLD_FRAMES],
                RU_PANEL_HEIGHT*RU_PANEL_BYTES)!=0);
 }
}
static void test_env(void){
 RuPanelAnim a={0};uint32_t t=100u;
 assert(ru_panel_update(&a,t,1u,1u));
 while(a.phase!=RU_PANEL_HOLD){t+=25u;ru_panel_update(&a,t,1u,1u);assert(t<3000u);}
 assert(a.frame==12u && a.hold_frame==0u);
 uint32_t hashes[8]={0};
 render_panel(&a,RU_PAGE_ENV);hashes[0]=hash();
 for(unsigned n=1;n<8;n++){
  /* Fixed 1-bit sprite sequence settles for just over a second. */
  t+=150u;ru_panel_update(&a,t,1u,1u);
  assert(a.phase==RU_PANEL_HOLD && a.frame==12u && a.hold_frame==n);
  render_panel(&a,RU_PAGE_ENV);hashes[n]=hash();
  assert(hashes[n]!=hashes[n-1]);
 }
 assert(a.next_ms!=0u);
 /* All smoke/loose rubble has settled, and it never comes back. */
 uint8_t hit[3]={0};unsigned moved=0;uint32_t prior=hashes[7];
 for(unsigned n=0;n<140;n++){
  t+=25u;
  (void)ru_panel_update(&a,t,1u,1u);
  assert(a.hold_frame==7u && a.frame==12u);
  assert(a.next_ms!=0u);
  hit[a.shake]=1u;
  render_panel(&a,RU_PAGE_ENV);
  if(hash()!=prior)moved++;
  prior=hash();
 }
 assert(hit[0]&&hit[1]&&hit[2]&&moved>5u);
 assert(ru_panel_update(&a,t+25u,0u,1u));
 assert(a.phase==RU_PANEL_CLOSE && a.frame==13u && a.hold_frame==0u);
 for(unsigned n=0;n<15;n++)ru_panel_update(&a,t+50u+n*90u,0u,1u);
 assert(a.phase==RU_PANEL_IDLE && a.frame==0u);
 /* Early Note OFF must still take original 13..19 closing frames. */
 a=(RuPanelAnim){0};t=200u;ru_panel_update(&a,t,1u,1u);
 for(unsigned n=0;n<7;n++){t+=80u;ru_panel_update(&a,t,1u,1u);}
 assert(a.phase==RU_PANEL_OPEN);
 ru_panel_update(&a,t+25u,0u,1u);
 assert(a.phase==RU_PANEL_CLOSE && a.frame>=13u && a.frame<=19u);
 puts("PASS ENV: eight unique settling sprites -> subtly vibrating static rubble -> release closes");
}
static void test_lfo(void){
 RuPanelAnim a={0};uint32_t t=100u;
 ru_panel_update(&a,t,1u,0u);
 while(a.phase!=RU_PANEL_HOLD){t+=25u;ru_panel_update(&a,t,1u,0u);assert(t<3000u);}
 assert(a.frame==12u && a.hold_frame==0u);
 unsigned seen=0,transitions=0;uint32_t last=0;
 for(unsigned n=0;n<48;n++){
  render_panel(&a,RU_PAGE_LFO);uint32_t now=hash();
  if(n&&now!=last)transitions++;
  last=now;
  seen|=1u<<a.hold_frame;
  t+=115u;
  assert(ru_panel_update(&a,t,1u,0u));
  assert(a.phase==RU_PANEL_HOLD && a.frame==12u);
  assert(a.hold_frame==(uint8_t)((n+1u)%RU_HOLD_FRAMES));
 }
 assert(seen==0xffu && transitions>=40u);
 ru_panel_update(&a,t+25u,0u,0u);
 assert(a.phase==RU_PANEL_CLOSE && a.frame==12u && a.hold_frame==0u);
 for(unsigned n=0;n<15;n++)ru_panel_update(&a,t+50u+n*80u,0u,0u);
 assert(a.phase==RU_PANEL_IDLE && a.frame==0u);
 puts("PASS LFO: all 8 debris sprites loop continuously (48 ticks); original reverse closes");
}
int main(void){check_rom_sprites();test_env();test_lfo();puts("PASS V1.70: native render, original sprite parity, release and note-gate state");return 0;}
