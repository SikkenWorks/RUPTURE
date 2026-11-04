/* RUPTURE V1.66: preserve audio-first UI scheduling without slowing the art.
 * Sprite state is advanced by the 25ms service clock, NOT by LCD rendering.
 * A new animation/shake frame has priority over cosmetic knob interpolation.
 * The caller latches animation_changed until an actual paint completes.
 * Never called from the audio ISR. SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_LCD_BUDGET_H
#define RUPTURE_LCD_BUDGET_H
#include <stdint.h>
static inline uint16_t ru_lcd_animation_period(uint8_t pressure,uint8_t missed){
 /* Only smooth knob interpolation and the splash require a rate budget.
  * This value must NEVER be applied to individual sprite changes. */
 (void)pressure; /* High max DSP percent is not a DMA underrun. */
 if(missed>=3u)return 100u; /* Actual repeated lateness: protect audio. */
 if(missed)return 50u;
 return 25u; /* 40 FPS target for knob easing at healthy DMA. */
}
static inline uint16_t ru_lcd_idle_period(uint8_t pressure,uint8_t missed){
 if(missed>=3u||pressure>=20u)return 750u;
 if(missed||pressure>=9u)return 500u;
 return 350u;
}
static inline uint8_t ru_lcd_should_paint(uint32_t now,uint32_t last,
 uint8_t urgent,uint8_t animation_changed,uint8_t visual_moving,
 uint8_t pressure,uint8_t missed){
 if(urgent)return 1u; /* Input/note/page changes never delayed. */
 const uint32_t elapsed=(uint32_t)(now-last);
 /* CRITICAL: A new 1-bit panel or leak frame is an event, not an idle visual.
  * Even under reported audio pressure, show each due sprite on the next
  * service tick (25ms) without adding work to the audio callback. If actual
  * LCD transfers are too slow, the time-based state catches up independently. */
 if(animation_changed){
  /* Never confuse animation with idle redraw. Real repeated DMA misses
   * still impose the original protective cap to avoid audio dropouts. */
  /* A high DSP MAX is historical and alone must not slow graphics.
   * Throttle only when actual DMA deadline misses persist. */
  const uint16_t period=missed>=3u?100u:missed?50u:25u;
  if(elapsed>=period)return 1u;
 }
 if(visual_moving && elapsed>=ru_lcd_animation_period(pressure,missed))return 1u;
 return (uint8_t)(elapsed>=ru_lcd_idle_period(pressure,missed));
}
#endif
