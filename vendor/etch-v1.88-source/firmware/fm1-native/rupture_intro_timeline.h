/* RUPTURE V2.0.43: quicker fades, deterministic fixed-layout boot intro.
 * The scene is rendered on CPU0 before starting the audio engine. */
#ifndef RUPTURE_INTRO_TIMELINE_H
#define RUPTURE_INTRO_TIMELINE_H
#include <stdint.h>

#define RU_INTRO_FRAME_MS 60u
#define RU_INTRO_LOGO_IN 7u
#define RU_INTRO_LOGO_HOLD 12u
#define RU_INTRO_LOGO_OUT 7u
#define RU_INTRO_TEXT_IN 8u
#define RU_INTRO_TEXT_HOLD 13u
#define RU_INTRO_WORD_FLICKER 15u
#define RU_INTRO_WORD_HOLD 16u
#define RU_INTRO_TEXT_OUT 8u
#define RU_INTRO_FRAMES (RU_INTRO_LOGO_IN+RU_INTRO_LOGO_HOLD+RU_INTRO_LOGO_OUT+RU_INTRO_TEXT_IN+RU_INTRO_TEXT_HOLD+RU_INTRO_WORD_FLICKER+RU_INTRO_WORD_HOLD+RU_INTRO_TEXT_OUT)
#define RU_INTRO_DURATION_MS (RU_INTRO_FRAMES*RU_INTRO_FRAME_MS)

#define RU_INTRO_SCENE_LOGO 0u
#define RU_INTRO_SCENE_TEXT 1u
typedef struct {
 uint8_t scene,opacity,uppercase,word_visible;
 int8_t word_dx,word_dy;
} RuIntroFrame;

static RuIntroFrame ru_intro_frame_desc(uint8_t index){
 RuIntroFrame result={RU_INTRO_SCENE_LOGO,0u,0u,1u,0,0};
 uint8_t i=index;
 if(i>=RU_INTRO_FRAMES)i=RU_INTRO_FRAMES-1u;
 if(i<RU_INTRO_LOGO_IN){
  result.opacity=(uint8_t)((((uint16_t)i+1u)*16u)/RU_INTRO_LOGO_IN);
  return result;
 }
 i=(uint8_t)(i-RU_INTRO_LOGO_IN);
 if(i<RU_INTRO_LOGO_HOLD){result.opacity=16u;return result;}
 i=(uint8_t)(i-RU_INTRO_LOGO_HOLD);
 if(i<RU_INTRO_LOGO_OUT){
  result.opacity=(uint8_t)((((uint16_t)RU_INTRO_LOGO_OUT-1u-i)*16u)/RU_INTRO_LOGO_OUT);
  return result;
 }
 i=(uint8_t)(i-RU_INTRO_LOGO_OUT);
 result.scene=RU_INTRO_SCENE_TEXT;
 if(i<RU_INTRO_TEXT_IN){
  result.opacity=(uint8_t)((((uint16_t)i+1u)*16u)/RU_INTRO_TEXT_IN);
  return result;
 }
 i=(uint8_t)(i-RU_INTRO_TEXT_IN);
 result.opacity=16u;
 if(i<RU_INTRO_TEXT_HOLD)return result;
 i=(uint8_t)(i-RU_INTRO_TEXT_HOLD);
 if(i<RU_INTRO_WORD_FLICKER){
  /* Only word pixels can change. The prefix and suffix stay anchored. */
  static const int8_t dx[15]={0,1,-1,2,-2,1,0,-1,2,-2,1,-1,0,1,0};
  static const int8_t dy[15]={0,-1,1,0,-1,2,-2,1,-1,1,0,-1,1,0,0};
  static const uint8_t caps[15]={0,1,0,1,0,1,1,0,1,0,1,1,0,1,1};
  static const uint8_t shown[15]={1,1,0,1,1,1,0,1,1,0,1,1,1,1,1};
  result.word_dx=dx[i];result.word_dy=dy[i];
  result.uppercase=caps[i];result.word_visible=shown[i];
  return result;
 }
 i=(uint8_t)(i-RU_INTRO_WORD_FLICKER);
 result.uppercase=1u;
 if(i<RU_INTRO_WORD_HOLD)return result;
 i=(uint8_t)(i-RU_INTRO_WORD_HOLD);
 result.opacity=(uint8_t)((((uint16_t)RU_INTRO_TEXT_OUT-1u-i)*16u)/RU_INTRO_TEXT_OUT);
 return result;
}

static uint8_t ru_intro_frame_at(uint32_t elapsed_ms){
 uint32_t idx=elapsed_ms/RU_INTRO_FRAME_MS;
 return (uint8_t)(idx>=RU_INTRO_FRAMES?RU_INTRO_FRAMES-1u:idx);
}
#endif
