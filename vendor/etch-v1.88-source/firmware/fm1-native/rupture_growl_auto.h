/* RUPTURE V2.0.16: light, independent GROWL self-modulation.
 * Based on the role of free LFO + smooth chaos in performance synths,
 * not SHRUB's copyrighted code or actual attractor implementation.
 * Called from the 25ms foreground UI service; NEVER from PCM sample loop.
 * No float, malloc, tables, new voices, storage writes or timer IRQs. */
#ifndef RUPTURE_GROWL_AUTO_H
#define RUPTURE_GROWL_AUTO_H
#include <stdint.h>

#define RU_GROWL_AUTO_SLOW_MS 5400u   /* 0.185 Hz throat breathing */
#define RU_GROWL_AUTO_FAST_MS 1700u   /* 0.588 Hz delicate pulse */
#define RU_GROWL_AUTO_DRIFT_MS 8000u  /* nonrepeating slow formant drift */
#define RU_GROWL_AUTO_USER_PRIORITY_MS 450u

typedef struct {int8_t expression_offset;int8_t formant_drift;} RuGrowlAutoFrame;

/* One continuous, twice-eased triangle: no sharp corner at the extrema. */
static inline int32_t ru_growl_auto_ease(uint32_t x){
 if(x>=1024u)return 1024;
 return (int32_t)(((uint64_t)x*x*(3072u-2u*x))>>20);
}
static inline int32_t ru_growl_auto_wave(uint32_t ms,uint32_t period){
 uint32_t phase=((ms%period)+(period/4u))%period;
 uint32_t pos=(phase*2048u)/period;
 int32_t a=pos<=1024u?ru_growl_auto_ease(pos):ru_growl_auto_ease(2048u-pos);
 return pos<=1024u?(-1024+2*a):(2*a-1024);
}
/* Tiny deterministic, interpolated control-rate drift. Not pitch jitter. */
static inline int8_t ru_growl_auto_random(uint32_t step){
 if(!step)return 0;
 uint32_t v=step*747796405u+2891336453u;
 v^=v>>15;v*=277803737u;v^=v>>15;
 return (int8_t)((int32_t)(v%9u)-4);
}
static inline RuGrowlAutoFrame ru_growl_auto_frame(uint32_t now_ms,
                                                  uint32_t last_manual_ms){
 RuGrowlAutoFrame out={0,0};
 uint32_t step=now_ms/RU_GROWL_AUTO_DRIFT_MS;
 uint32_t fraction=((now_ms%RU_GROWL_AUTO_DRIFT_MS)*1024u)/RU_GROWL_AUTO_DRIFT_MS;
 int32_t a=ru_growl_auto_random(step);
 int32_t b=ru_growl_auto_random(step+1u);
 int32_t drift=a+((b-a)*ru_growl_auto_ease(fraction))/1024;
 int32_t slow=ru_growl_auto_wave(now_ms,RU_GROWL_AUTO_SLOW_MS);
 int32_t fast=ru_growl_auto_wave(now_ms,RU_GROWL_AUTO_FAST_MS);
 int32_t offset=(slow*10)/1024+(fast*2)/1024+drift;
 /* Physical knob input wins for 450ms. LATCH still holds its actual value;
  * RETURN's existing 125ms delay/glide stays unchanged. */
 if(last_manual_ms){
  uint32_t age=(uint32_t)(now_ms-last_manual_ms);
  if(age<RU_GROWL_AUTO_USER_PRIORITY_MS){
   offset=(offset*(int32_t)age)/(int32_t)RU_GROWL_AUTO_USER_PRIORITY_MS;
   drift=(drift*(int32_t)age)/(int32_t)RU_GROWL_AUTO_USER_PRIORITY_MS;
  }
 }
 out.expression_offset=(int8_t)offset;
 out.formant_drift=(int8_t)drift;
 return out;
}
#endif
