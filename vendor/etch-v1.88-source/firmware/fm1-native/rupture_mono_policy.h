/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE deterministic mono priority and bounded sample-by-sample glide.
 * Shared by native CXS audio and host regression tests. */
#ifndef RUPTURE_MONO_POLICY_H
#define RUPTURE_MONO_POLICY_H
#include <stdint.h>
static inline uint8_t rupture_choose_pitch(const uint8_t *pitches,uint8_t count,uint8_t priority){
 uint8_t best=0u;
 for(uint8_t i=1;i<count;i++)
  if(priority==0u || (priority==1u && pitches[i]>=pitches[best]) ||
    (priority==2u && pitches[i]<=pitches[best]))best=i;
 return best;
}
static inline uint32_t rupture_glide_next(uint32_t current,uint32_t target,uint32_t remaining){
 if(!remaining)return target;
 if(current<target){uint32_t step=(target-current)/remaining;return current+(step?step:1u);}
 if(current>target){uint32_t step=(current-target)/remaining;return current-(step?step:1u);}
 return current;
}
#endif
