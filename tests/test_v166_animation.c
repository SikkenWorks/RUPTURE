/* V1.66: actual 1-bit native renderer and note-gated animation scheduler.
 * Regression: a one-tick sprite change MUST stay pending until painted. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_panel_anim.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_lcd_budget.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static uint32_t hash(void){uint32_t h=2166136261u;for(unsigned n=0;n<sizeof(ru_bits);n++)h=(h^ru_bits[n])*16777619u;return h;}
static void test_play(void){
 RuState v={0};v.page=RU_PAGE_PLAY;
 ru_render(&v);
 /* Idle sprite must still be pixel-identical to original packed PNG. */
 for(unsigned y=0;y<RU_DAM_H;y++){
  const uint8_t *from=&ru_dam_sprites[0][y][0];
  const uint8_t *to=&ru_bits[(RU_DAM_Y+y)*(RU_W/8)+RU_DAM_X/8];
  assert(!memcmp(from,to,RU_DAM_ROW_BYTES));
 }
 v.dam_frame=10;ru_render(&v);uint32_t ten=hash();
 v.dam_frame=11;ru_render(&v);uint32_t eleven=hash();
 v.dam_frame=12;ru_render(&v);uint32_t twelve=hash();
 assert(ten!=eleven&&eleven!=twelve&&ten!=twelve);
 RuDamAnim a={0};uint8_t last=0,had_open=0,had_loop=0;
 unsigned paint=0,flow_transitions=0;
 uint8_t pending=0;
 uint32_t lastpaint=0;
 for(uint32_t ms=0;ms<6000;ms+=25){
  const uint8_t changed=ru_dam_update(&a,ms,1,5); // fastest octave
  if(changed)pending=1;
  if(a.phase==RU_DAM_OPEN)had_open=1;
  if(a.phase==RU_DAM_LEAK){had_loop=1;if(a.frame!=last)flow_transitions++;}
  if(ru_lcd_should_paint(ms,lastpaint,ms==0u,pending,0u,0u,0u)){
   v.dam_frame=a.frame;ru_render(&v);lastpaint=ms;pending=0;paint++;
  }
  last=a.frame;
 }
 assert(had_open&&had_loop&&flow_transitions>20&&paint>40);
 assert(a.phase==RU_DAM_LEAK && a.frame>=10&&a.frame<=12);
 for(uint32_t ms=6000;ms<7800;ms+=25)ru_dam_update(&a,ms,0,5);
 assert(a.phase==RU_DAM_IDLE&&a.frame==0);
 printf("PLAY hold: %u sprite transitions, %u painted screens, note-off closed\n",flow_transitions,paint);
}
static void test_panel(uint8_t mode,uint8_t page){
 RuPanelAnim a={0};uint8_t held=0;uint16_t seen=0;uint8_t shakes=0;
 for(uint32_t ms=0;ms<4000;ms+=25){
  if(ru_panel_update(&a,ms,1,mode))seen++;
  if(a.phase==RU_PANEL_HOLD)held=1;
  if(a.shake)shakes=1;
 }
 assert(held&&seen>15);
 if(mode==2){assert(a.frame>=11&&a.frame<=13);}else{assert(a.frame==12&&shakes);}
 RuState v={0};v.page=page;v.panel_frame=a.frame;v.panel_shake=0;
 ru_render(&v);uint32_t orig=hash();
 if(mode!=2){v.panel_shake=1;ru_render(&v);assert(hash()!=orig);}
 for(uint32_t ms=4000;ms<6000;ms+=25)ru_panel_update(&a,ms,0,mode);
 assert(a.phase==RU_PANEL_IDLE&&a.frame==0);
}
static void test_latched_frame(void){
 uint8_t pending=0;uint32_t last_paint=0;uint32_t paint=0;
 // V1.69 intentionally shortened the real-DMA-miss guard to 100ms.
 // An animation frame arriving earlier stays pending, then gets painted
 // at the first permitted 25ms service tick. Historical 180ms assert was
 // for the older V1.66 rate limiter and is no longer a valid expectation.
 for(uint32_t t=0;t<=400;t+=25){
  uint8_t new_frame=(t==25);
  if(new_frame)pending=1;
  if(ru_lcd_should_paint(t,last_paint,0,pending,0,20,3)){
   pending=0;last_paint=t;paint++;
  }
 }
 assert(paint>=1&&last_paint>=100u&&!pending);
 assert(!ru_lcd_should_paint(5,0,0,0,0,0,0));
 assert(ru_lcd_should_paint(25,0,0,1,0,0,0));
}
int main(void){test_play();test_panel(0,RU_PAGE_LFO);test_panel(1,RU_PAGE_ENV);test_panel(2,RU_PAGE_SHAPE);test_latched_frame();puts("PASS V1.66: PLAY, ENV, LFO, SHAPE loops, shake, release, sprite latch, 1-bit source parity");return 0;}
