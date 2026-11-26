/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE 2.0.58: independently persisted CXS and GROWL expression profiles.
 * Tiny scalar representation; no changes to the existing PJT35 patch ABI.
 */
#ifndef RUPTURE_TOUCH_PROFILE_H
#define RUPTURE_TOUCH_PROFILE_H
#include <stdint.h>
enum { RU_TOUCH_MODE_PRESS, RU_TOUCH_MODE_DRIFT, RU_TOUCH_MODE_RANDOM,
       RU_TOUCH_MODE_VELOCITY };
enum { RU_TOUCH_PROFILE_CXS, RU_TOUCH_PROFILE_GROWL, RU_TOUCH_PROFILE_COUNT };
struct ru_touch_profile {
 uint8_t target,mode,sensitivity,min_pct,max_pct,rate;
};
#define RU_TOUCH_PROFILE_DEFAULT {0u,RU_TOUCH_MODE_PRESS,1u,0u,100u,1u}
static inline uint8_t ru_touch_profile_valid(const struct ru_touch_profile *p){
 return p&&p->target<=7u&&p->mode<=RU_TOUCH_MODE_VELOCITY&&
  p->sensitivity<4u&&p->min_pct<=p->max_pct&&p->max_pct<=100u&&p->rate<4u;
}
static inline uint8_t ru_touch_profile_equal(const struct ru_touch_profile *a,
 const struct ru_touch_profile *b){
 return a->target==b->target&&a->mode==b->mode&&
  a->sensitivity==b->sensitivity&&a->min_pct==b->min_pct&&
  a->max_pct==b->max_pct&&a->rate==b->rate;
}
#endif
