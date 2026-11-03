#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_welcome.h"
#define SEL (1u<<3)
#define HOME (1u<<8)
#define SAVE (1u<<9)
static void yes(RuWelcome *w){(void)ru_welcome_step(w,1,0,0,100);}
static void no(RuWelcome *w){(void)ru_welcome_step(w,-1,0,0,100);}
static uint8_t ok(RuWelcome *w){return ru_welcome_step(w,0,SEL,0,100);}
int main(void){
 RuWelcome w={0};w.phase=RU_WELCOME_INTRO;
 assert(!ok(&w)&&w.phase==RU_WELCOME_BACKUP);
 no(&w);ok(&w);assert(w.phase==RU_WELCOME_PROCEED);
 no(&w);ok(&w);assert(w.phase==RU_WELCOME_SAD); /* explicit NO :( branch */
 ok(&w);assert(w.phase==RU_WELCOME_CRT);
 yes(&w);ok(&w);assert(w.phase==RU_WELCOME_READY&&w.crt==1u);
 assert(ok(&w)==RU_WELCOME_ACTION_FINISH&&w.phase==RU_WELCOME_OFF);
 /* Backup YES and format NO: always continue, no erase action. */
 w=(RuWelcome){.phase=RU_WELCOME_INTRO};ok(&w);yes(&w);ok(&w);
 assert(w.phase==RU_WELCOME_FORMAT);no(&w);ok(&w);
 assert(w.phase==RU_WELCOME_CRT);no(&w);ok(&w);
 assert(w.phase==RU_WELCOME_READY&&w.crt==0u);
 assert(ok(&w)==RU_WELCOME_ACTION_FINISH);
 /* Select YES format, require uninterrupted SAVE hold for three seconds. */
 w=(RuWelcome){.phase=RU_WELCOME_FORMAT};yes(&w);ok(&w);
 assert(w.phase==RU_WELCOME_ERASE);
 assert(ru_welcome_step(&w,0,0,SAVE,100u)==RU_WELCOME_ACTION_NONE);
 assert(ru_welcome_step(&w,0,0,SAVE,3099u)==RU_WELCOME_ACTION_NONE);
 assert(ru_welcome_step(&w,0,0,SAVE,3100u)==RU_WELCOME_ACTION_FORMAT);
 assert(w.phase==RU_WELCOME_FORMATTING);
 /* The old V1.59 screen could be advanced without a legible confirmation.
  * A successful format now remains visible until a later SEL acknowledgement. */
 ru_welcome_format_result(&w,1u,3200u);
 assert(w.status==1u&&w.phase==RU_WELCOME_FORMAT_RESULT);
 assert(ru_welcome_step(&w,0,SEL,0,3500u)==0&&w.phase==RU_WELCOME_FORMAT_RESULT);
 assert(ru_welcome_step(&w,0,SEL,0,4699u)==0&&w.phase==RU_WELCOME_FORMAT_RESULT);
 assert(ru_welcome_step(&w,0,SEL,0,4700u)==0&&w.phase==RU_WELCOME_CRT);
 /* A failed flash operation is never displayed as a success and is also
  * latched until explicitly acknowledged. */
 w=(RuWelcome){.phase=RU_WELCOME_FORMATTING};
 ru_welcome_format_result(&w,3u,6000u);
 assert(w.status==3u&&w.phase==RU_WELCOME_FORMAT_RESULT);
 assert(ru_welcome_step(&w,0,SEL,0,6100u)==0&&w.phase==RU_WELCOME_FORMAT_RESULT);
 assert(ru_welcome_step(&w,0,SEL,0,7500u)==0&&w.phase==RU_WELCOME_CRT);
 /* Already-compatible storage must never claim a destructive format. */
 w=(RuWelcome){.phase=RU_WELCOME_FORMATTING};
 ru_welcome_format_result(&w,2u,8000u);
 assert(w.status==2u&&w.phase==RU_WELCOME_FORMAT_RESULT);
 /* Cancellation returns to format with NO selected. */
 w=(RuWelcome){.phase=RU_WELCOME_ERASE,.choice=1u};
 assert(ru_welcome_step(&w,0,HOME,0,4200u)==0);
 assert(w.phase==RU_WELCOME_FORMAT&&w.choice==0u);
 /* Save hold resets when released mid-confirm. */
 w=(RuWelcome){.phase=RU_WELCOME_ERASE};
 (void)ru_welcome_step(&w,0,0,SAVE,1000u);
 (void)ru_welcome_step(&w,0,0,0,3500u);
 assert(ru_welcome_step(&w,0,0,SAVE,3600u)==0);
 assert(ru_welcome_step(&w,0,0,SAVE,6599u)==0);
 assert(ru_welcome_step(&w,0,0,SAVE,6600u)==RU_WELCOME_ACTION_FORMAT);
 puts("PASS V1.60 welcome: NO :( continues, CRT selection, format cancellation and explicit 3s hold");
 return 0;
}
