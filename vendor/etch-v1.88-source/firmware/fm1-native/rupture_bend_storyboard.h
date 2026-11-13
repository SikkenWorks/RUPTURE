/* RUPTURE V1.88.1: literal poster storyboard frame ordering, UI/control-thread only.
 * Four eruptive scenes have five rise frames (0..4) followed by three
 * return frames (5..7). Camera close-up pans 0..7 in both directions.
 * No dynamic drawing, no audio callback work, no OTA/storage changes.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_BEND_STORYBOARD_H
#define RUPTURE_BEND_STORYBOARD_H
#include <stdint.h>
#include "rupture_bend_curves.h"
#include "rupture_bend_sprites.h"
static inline uint8_t ru_bend_poster_frame(uint8_t curve,uint8_t shaped,uint8_t releasing){
 if(!shaped)return 0u;
 if(curve==RU_BEND_SCURVE){
  uint32_t frame=((uint32_t)shaped*(RU_BEND_FRAMES-1u)+99u)/100u;
  return (uint8_t)(frame<RU_BEND_FRAMES?frame:RU_BEND_FRAMES-1u);
 }
 if(releasing){
  if(shaped>=67u)return 5u;
  if(shaped>=34u)return 6u;
  return 7u;
 }
 return (uint8_t)(((uint32_t)shaped*4u+99u)/100u);
}
#endif
