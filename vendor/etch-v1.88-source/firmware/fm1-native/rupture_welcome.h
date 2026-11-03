/* RUPTURE V1.59 first-run on-device wizard: UI/control loop only.
 * No audio callbacks, storage writes or OTA calls here. */
#ifndef RUPTURE_WELCOME_H
#define RUPTURE_WELCOME_H
#include <stdint.h>
enum {RU_WELCOME_OFF=0u,RU_WELCOME_INTRO=1u,RU_WELCOME_BACKUP=2u,
 RU_WELCOME_PROCEED=3u,RU_WELCOME_SAD=4u,RU_WELCOME_FORMAT=5u,
 RU_WELCOME_ERASE=6u,RU_WELCOME_FORMAT_RESULT=7u,
 RU_WELCOME_CRT=8u,RU_WELCOME_READY=9u,
 RU_WELCOME_FORMATTING=10u};
enum {RU_WELCOME_ACTION_NONE=0u,RU_WELCOME_ACTION_FORMAT=1u,
 RU_WELCOME_ACTION_FINISH=2u};
typedef struct {
 uint32_t save_hold_since;
 uint32_t result_since_ms; /* keep actual success/error on screen until acknowledged */
 uint8_t phase,choice,crt,status; /* status 0 none,1 formatted,2 already ready,3 failed */
} RuWelcome;
static uint8_t ru_welcome_step(RuWelcome *w,int32_t turn,uint32_t pressed,
 uint32_t held,uint32_t now){
 if(!w||w->phase==RU_WELCOME_OFF)return RU_WELCOME_ACTION_NONE;
 if(w->phase==RU_WELCOME_ERASE){
  /* Selected YES + a deliberate physical SAVE hold is required for erase.
   * SEL or HOME always cancels safely, with zero flash operations. */
  if(pressed&((1u<<3)|(1u<<8))){w->phase=RU_WELCOME_FORMAT;w->choice=0;w->save_hold_since=0;return RU_WELCOME_ACTION_NONE;}
  if(held&(1u<<9)){
   if(!w->save_hold_since)w->save_hold_since=now?now:1u;
   if((uint32_t)(now-w->save_hold_since)>=3000u){
    w->save_hold_since=0u;w->phase=RU_WELCOME_FORMATTING;
    return RU_WELCOME_ACTION_FORMAT;
   }
  }else w->save_hold_since=0u;
  return RU_WELCOME_ACTION_NONE;
 }
 if(w->phase==RU_WELCOME_FORMATTING)return RU_WELCOME_ACTION_NONE;
 if(w->phase==RU_WELCOME_BACKUP||w->phase==RU_WELCOME_PROCEED||w->phase==RU_WELCOME_FORMAT||w->phase==RU_WELCOME_CRT){
  if(turn>0)w->choice=1u;
  else if(turn<0)w->choice=0u;
 }
 if(!(pressed&(1u<<3)))return RU_WELCOME_ACTION_NONE;
 switch(w->phase){
 case RU_WELCOME_INTRO:w->phase=RU_WELCOME_BACKUP;w->choice=0u;break;
 case RU_WELCOME_BACKUP:w->phase=w->choice?RU_WELCOME_FORMAT:RU_WELCOME_PROCEED;w->choice=0u;break;
 case RU_WELCOME_PROCEED:w->phase=w->choice?RU_WELCOME_FORMAT:RU_WELCOME_SAD;w->choice=0u;break;
 case RU_WELCOME_SAD:w->phase=RU_WELCOME_CRT;w->choice=w->crt;break;
 case RU_WELCOME_FORMAT:
  if(w->choice){w->phase=RU_WELCOME_ERASE;w->save_hold_since=0u;}
  else{w->phase=RU_WELCOME_CRT;w->choice=w->crt;}
  break;
 case RU_WELCOME_FORMAT_RESULT:
  /* Never advance as an accidental delayed tap from the SAVE hold. The
   * result is latched until a fresh SEL press after the minimum dwell. */
  if((uint32_t)(now-w->result_since_ms)<1500u)return RU_WELCOME_ACTION_NONE;
  w->phase=RU_WELCOME_CRT;w->choice=w->crt;break;
 case RU_WELCOME_CRT:w->crt=w->choice;w->phase=RU_WELCOME_READY;break;
 case RU_WELCOME_READY:w->phase=RU_WELCOME_OFF;return RU_WELCOME_ACTION_FINISH;
 default:break;
 }
 return RU_WELCOME_ACTION_NONE;
}
/* The flash operation returns its verified storage status. Show a dedicated
 * result until SEL acknowledges; no automatic success or time-based exit. */
static void ru_welcome_format_result(RuWelcome *w,uint8_t status,uint32_t now){
 w->status=status;w->result_since_ms=now?now:1u;
 w->phase=RU_WELCOME_FORMAT_RESULT;
}
#endif
