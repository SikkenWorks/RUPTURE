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
 w.phase=RU_WELCOME_FORMAT_RESULT;w.status=1u;
 assert(ok(&w)==0&&w.phase==RU_WELCOME_CRT);
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
 puts("PASS V1.59 welcome: NO :( continues, CRT selection, format cancellation and explicit 3s hold");
 return 0;
}
