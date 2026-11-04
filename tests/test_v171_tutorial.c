/* RUPTURE V1.71: first-run opt-in, safe tour navigation and actual 1-bit pages.
 * Compiles host-side with the same renderer used by native FM1 firmware. */
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
#define SEL (1u<<3)
#define HOME (1u<<8)
static unsigned ink(void){unsigned count=0;for(unsigned i=0;i<sizeof ru_bits;i++)count+=__builtin_popcount((unsigned)ru_bits[i]);return count;}
static uint32_t hash(void){uint32_t x=2166136261u;for(unsigned i=0;i<sizeof ru_bits;i++)x=(x^ru_bits[i])*16777619u;return x;}
static void next(RuWelcome *w){assert(ru_welcome_step(w,0,SEL,0,3500u)==RU_WELCOME_ACTION_NONE);}
static void test_welcome(void){
 RuWelcome w={0};w.phase=RU_WELCOME_CRT;
 ru_welcome_step(&w,-1,0,0,100u);next(&w);
 assert(w.phase==RU_WELCOME_READY && w.crt==0);
 next(&w);assert(w.phase==RU_WELCOME_TOUR_ASK && w.choice==0);
 /* NO skips the entire tour and completes setup, no formatting. */
 assert(ru_welcome_step(&w,0,SEL,0,4000u)==RU_WELCOME_ACTION_FINISH);
 assert(w.phase==RU_WELCOME_OFF);
 w=(RuWelcome){0};w.phase=RU_WELCOME_READY;
 next(&w);assert(w.phase==RU_WELCOME_TOUR_ASK);
 ru_welcome_step(&w,1,0,0,3600u);assert(w.choice==1u);
 next(&w);assert(w.phase==RU_WELCOME_TOUR && w.lesson==0 && w.overlay==1);
 next(&w);assert(w.lesson==1);
 ru_welcome_step(&w,-1,0,0,4000u);assert(w.lesson==0);
 ru_welcome_step(&w,-1,0,0,4000u);assert(w.lesson==0);
 ru_welcome_step(&w,1,0,0,4000u);assert(w.lesson==1);
 for(unsigned i=1;i<RU_TUTORIAL_COUNT-1u;i++)next(&w);
 assert(w.lesson==RU_TUTORIAL_COUNT-1u);
 assert(ru_welcome_step(&w,0,SEL,0,7000u)==RU_WELCOME_ACTION_FINISH);
 assert(w.phase==RU_WELCOME_OFF);
 w=(RuWelcome){0};w.phase=RU_WELCOME_TOUR;w.lesson=12u;w.replay=1u;
 assert(ru_welcome_step(&w,0,HOME,0,8000u)==RU_WELCOME_ACTION_FINISH);
 assert(w.phase==RU_WELCOME_OFF && w.replay==1u);
 /* SAVE and other input must never trigger a formatter/flash operation. */
 w=(RuWelcome){0};w.phase=RU_WELCOME_TOUR;
 for(int i=0;i<15;i++)assert(ru_welcome_step(&w,0,(1u<<9),1u<<9,(uint32_t)i*100)==RU_WELCOME_ACTION_NONE);
 assert(w.phase==RU_WELCOME_TOUR);
 puts("PASS: first-run opt-in YES/NO, SELECT back, SEL next, HOME exit, no tour flash actions");
}
static void test_tour_render(void){
 RuState v={0};v.page=RU_PAGE_WELCOME;v.welcome_phase=RU_WELCOME_TOUR_ASK;
 v.welcome_choice=0;ru_render(&v);uint32_t no=hash();
 v.welcome_choice=1;ru_render(&v);assert(hash()!=no);
 for(unsigned i=0;i<RU_TUTORIAL_COUNT;i++){
  const RuTutorialLesson *d=&ru_tutorial_lessons[i];
  assert(d->page<RU_PAGE_COUNT);
  assert(d->route && d->tip);
  assert(strlen(d->route)<=36 && strlen(d->tip)<=36);
  for(unsigned k=0;k<4;k++)if(d->knobs[k])assert(strlen(d->knobs[k])<=36);
  v=(RuState){0};v.page=d->page;v.fx_selected=d->fx;
  v.selected=d->selected;v.settings_open=d->settings_open;
  v.welcome_phase=RU_WELCOME_TOUR;v.welcome_lesson=i;
  v.welcome_overlay=1;v.show_battery=1;v.battery=70;
  v.seq_length=16;v.seq_bpm=120;v.seq_first_note=255;
  v.panel_frame=12;v.panel_hold_active=1;v.panel_hold_frame=7;
  v.dam_frame=11;
  for(unsigned j=0;j<4;j++)v.k[j]=50;
  ru_render(&v);uint32_t overlay=hash();assert(ink()>50);
  v.welcome_overlay=0;ru_render(&v);assert(hash()!=overlay);
  /* Full-screen underlying page must remain pixel-perfect; apart from
   * a footer the complete screen above y=226 is untouched. */
  uint8_t full[sizeof ru_bits];memcpy(full,ru_bits,sizeof full);
  v.welcome_phase=RU_WELCOME_OFF;ru_render(&v);
  assert(!memcmp(full,ru_bits,226u*30u));
 }
 puts("PASS: every one of 43 real 240x240 tour screens, overlay/full view and actual page identity");
}
int main(void){test_welcome();test_tour_render();return 0;}
