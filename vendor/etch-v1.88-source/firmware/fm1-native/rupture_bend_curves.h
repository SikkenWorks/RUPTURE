/* RUPTURE V1.86.1: K3 momentary BEND response curve presets.
 * The control thread computes these bounded integer curves, not the audio ISR.
 * Keeping this independent of patch/storage/OTA avoids a project format change.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_BEND_CURVES_H
#define RUPTURE_BEND_CURVES_H
#include <stdint.h>
#define RU_BEND_CURVE_COUNT 5u
enum {RU_BEND_LINEAR=0u,RU_BEND_SOFT=1u,RU_BEND_HARD=2u,
      RU_BEND_SCURVE=3u,RU_BEND_EXP=4u};
static uint8_t ru_bend_curve_percent(uint8_t curve,uint8_t raw){
 uint32_t x=raw;
 if(x>=100u)return 100u;
 const uint32_t x2=x*x;
 uint32_t y=x;
 switch(curve){
  case RU_BEND_SOFT: y=(200u*x-x2+50u)/100u;break;        /* ease out */
  case RU_BEND_HARD: y=(x2+50u)/100u;break;              /* ease in */
  case RU_BEND_SCURVE: y=(3u*x2*100u-2u*x2*x+5000u)/10000u;break;
  case RU_BEND_EXP: y=(x2*x+5000u)/10000u;break;         /* cubic */
  default:break;                                         /* LINEAR */
 }
 return (uint8_t)(y>100u?100u:y);
}
static const char *const ru_bend_curve_names[RU_BEND_CURVE_COUNT]={
 "LINEAR","SOFT","HARD","S-CURVE","EXP"};
#endif
