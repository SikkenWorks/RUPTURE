/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V2.0.58 optional FM1 key pressure: the digital key matrix remains
 * the sole source of note on/off, and the ADC only drives expression.
 * No dynamic allocation, floats, sample-loop work or changes to CXS DSP.
 */
#ifndef RUPTURE_KEY_TOUCH_H
#define RUPTURE_KEY_TOUCH_H
#include <stdint.h>
#include "audio_core.h"
#include "rupture_touch_profile.h"

enum {RU_TOUCH_OFF, RU_TOUCH_FILTER, RU_TOUCH_GROWL, RU_TOUCH_MOSFET,
      RU_TOUCH_DISTORTION, RU_TOUCH_REVERB, RU_TOUCH_SUB1, RU_TOUCH_WAH};
#define RU_TOUCH_MAX_TARGET RU_TOUCH_WAH
#define RU_TOUCH_CONFIG_DEFAULT 8u /* OFF, MEDIUM sensitivity */

/* Existing 8-byte global preference record: low three bits = target, high
 * two bits = sensitivity. Never copy into an actual project or preset. */
static inline uint8_t ru_touch_target(uint8_t c){return c&7u;}
static inline uint8_t ru_touch_sensitivity(uint8_t c){return (c>>3)&3u;}
static inline uint8_t ru_touch_config(uint8_t target,uint8_t sensitivity){
 return (uint8_t)((target&7u)|((sensitivity&3u)<<3));
}
static inline uint8_t ru_touch_clamp(uint32_t x){return (uint8_t)(x>127u?127u:x);}

struct ru_touch_state {
 uint32_t prior_keys,last_ms;
 uint16_t landing,seen_generation,raw;
 uint8_t active_key,valid,pressure;
};

/* Called by the FOREGROUND at most once per 20 ms. The 20 available keys
 * are F3..C5 (bits 0..19). The upper seven have no ADC and stay expression-
 * neutral. The newest held key wins, as in a monophonic bass instrument.
 * A new note discards old pressure and never inherits a previous key's force.
 * No analog sample can synthesize a note event or extend a note release. */
static inline uint8_t ru_touch_update(struct ru_touch_state *s,uint32_t keys,
 const uint16_t samples[20],const uint16_t generations[20],uint32_t now,
 uint8_t config){
 if(!s)return 0u;
 if(!ru_touch_target(config)){
  s->prior_keys=keys;s->active_key=255u;s->valid=0u;s->pressure=0u;s->raw=0u;return 0u;
 }
 keys&=0x07ffffffu;
 uint32_t new_keys=keys&~s->prior_keys;
 s->prior_keys=keys;
 uint8_t chosen=s->active_key;
 if(new_keys){
  for(uint8_t k=0;k<27u;k++)if(new_keys&(1u<<k))chosen=k;
 }else if(chosen>=27u||!(keys&(1u<<chosen))){
  chosen=255u;
  for(uint8_t k=0;k<27u;k++)if(keys&(1u<<k))chosen=k;
 }
 if(chosen!=s->active_key){
  s->active_key=chosen;s->valid=0u;s->pressure=0u;s->landing=0u;s->raw=0u;
  s->seen_generation=chosen<20u?generations[chosen]:0u;
  s->last_ms=now;
 }
 if(chosen>=20u){s->pressure=0u;s->raw=0u;return 0u;}
 const uint16_t gen=generations[chosen],sample=samples[chosen];
 if(gen!=s->seen_generation){
  s->seen_generation=gen;s->last_ms=now;
  if(sample<=3u||sample>=1020u){ /* ADC rails, not valid pressure */
   s->valid=0u;s->raw=0u;s->pressure=0u;
  }else if(!s->valid){s->landing=sample;s->raw=sample;s->valid=1u;
  }else{
   /* The first landed reading is the light-contact zero. A relaxed finger
    * raises the ADC value: let the zero rise immediately, so releasing
    * force brings reverb all the way back to its configured MIN. Otherwise
    * a later firmer press lowers the resistance, hence lowers the ADC.
    * This is held pressure, not a measured MIDI note-on velocity. */
   s->raw=sample;
   if(sample>s->landing)s->landing=sample;
   uint32_t delta=s->landing>sample?(uint32_t)(s->landing-sample):0u;
   if(delta>3u)delta-=3u;else delta=0u;
   /* The old 40..180 ADC-count calibration saturated at a light
    * touch on a number of panels. Use a wider physical range and also
    * bound the CHANGE per 20ms foreground observation. Neither the
    * pressure ADC nor this smoother owns note-on/note-off events.
    * LOW is least sensitive, ULTRA most sensitive. */
   static const uint16_t ranges[4]={480u,350u,260u,185u};
   uint32_t raw=delta*127u/ranges[ru_touch_sensitivity(config)];
   uint8_t goal=ru_touch_clamp(raw);
   uint8_t current=s->pressure;
   /* Never jump to 100% on one noisy/strong ADC reading. Release is
    * allowed to return slightly faster so MIN comes back naturally. */
   uint8_t up_limit=(uint8_t)(7u+ru_touch_sensitivity(config));
   uint8_t down_limit=15u;
   if(goal>current && goal-current>up_limit)goal=(uint8_t)(current+up_limit);
   if(goal<current && current-goal>down_limit)goal=(uint8_t)(current-down_limit);
   s->pressure=goal;
  }
 }
 if((uint32_t)(now-s->last_ms)>70u){
  /* Missing ADC updates fail silent for expression, not for notes. */
  s->valid=0u;s->raw=0u;s->pressure=(uint8_t)(s->pressure*3u/4u);
 }
 return s->pressure;
}

static inline uint16_t ru_touch_add_q15(uint16_t base,uint8_t p,uint16_t range){
 uint32_t n=(uint32_t)base+((uint32_t)p*range+63u)/127u;
 return (uint16_t)(n>32767u?32767u:n);
}

/* A single COPY of the regular controls is modified on the ALNK block
 * boundary. Do not mutate the saved preset, the chain, or the CXS engine.
 * When the feature is OFF, return before changing even a single field. */
static inline void ru_touch_apply(struct etch_native_controls *c,
 uint8_t config,uint8_t pressure){
 if(!c||!pressure)return;
 uint8_t target=ru_touch_target(config);
 if(!target)return;
 switch(target){
  case RU_TOUCH_FILTER:
   c->cutoff=ru_touch_add_q15(c->cutoff,pressure,15000u);return;
  case RU_TOUCH_GROWL:
   if(c->main_wave){uint32_t x=c->cxs_pressure+(uint32_t)pressure*45u/127u;
    c->cxs_pressure=(uint8_t)(x>100u?100u:x);}
   return;
  case RU_TOUCH_SUB1:
   if(c->sub_blend[0]){uint32_t x=c->sub_blend[0]+(uint32_t)pressure*50u/127u;
    c->sub_blend[0]=(uint8_t)(x>100u?100u:x);}
   return;
  default:break;
 }
 if(!c->fx_chain_active||!c->fx_chain.independent)return;
 for(uint8_t i=0u;i<RU_FX_SLOTS;i++){
  if(!(c->fx_chain.enabled&(1u<<i)))continue;
  uint8_t type=c->fx_chain.type[i];
  uint8_t match=(uint8_t)((target==RU_TOUCH_MOSFET&&type==RU_FX_MOSFET)||
   (target==RU_TOUCH_DISTORTION&&type==RU_FX_DISTORTION)||
   (target==RU_TOUCH_REVERB&&type==RU_FX_REVERB)||
   (target==RU_TOUCH_WAH&&type==RU_FX_WAH));
  if(!match)continue;
  uint8_t knob=(uint8_t)(target==RU_TOUCH_REVERB?3u:0u);
  c->fx_chain.parameter[i][knob]=ru_touch_add_q15(
    c->fx_chain.parameter[i][knob],pressure,13000u);
 }
}

/* A drift is time-driven, never triggered by ADC noise. Both shapes are
 * deterministic and smoothed in the 20ms foreground; no timers, randomness
 * or division is added to the per-sample or core2 FX hot paths. */
struct ru_touch_drift_state {
 uint32_t start_ms,rng;
 uint8_t initialized,from,to,mode,rate;
};
static inline uint8_t ru_touch_motion(struct ru_touch_drift_state *s,
 const struct ru_touch_profile *p,uint32_t now,uint32_t keys,uint8_t pressure){
 if(!s||!p||!p->target||!(keys&0x07ffffffu)){
  if(s)s->initialized=0u;
  return 0u;
 }
 if(p->mode==RU_TOUCH_MODE_PRESS){s->initialized=0u;return pressure;}
 if(p->mode==RU_TOUCH_MODE_VELOCITY){s->initialized=0u;return 0u;}
 static const uint16_t periods[4]={8000u,5000u,2500u,1250u};
 const uint32_t period=periods[p->rate&3u];
 if(!s->initialized||s->mode!=p->mode||s->rate!=p->rate){
  s->initialized=1u;s->mode=p->mode;s->rate=p->rate;s->start_ms=now;
  s->from=0u;s->to=0u;
  if(!s->rng)s->rng=0x53696b6bu;
  if(p->mode==RU_TOUCH_MODE_RANDOM){
   s->rng=s->rng*1664525u+1013904223u;
   s->to=(uint8_t)((s->rng>>16)&127u);
  }
 }
 uint32_t elapsed=now-s->start_ms;
 if(p->mode==RU_TOUCH_MODE_DRIFT){
  elapsed%=period;
  uint32_t x=elapsed*254u/period;
  return (uint8_t)(x<=127u?x:254u-x);
 }
 if(elapsed>=period){
  /* Reset to the last destination, then interpolate to the next random
   * point. One PRNG update per interval at most, never one per sample. */
  s->start_ms=now;
  s->from=s->to;
  s->rng=s->rng*1664525u+1013904223u;
  s->to=(uint8_t)((s->rng>>16)&127u);
  elapsed=0u;
 }
 uint32_t t=elapsed*256u/period;
 if(t>256u)t=256u;
 const uint32_t eased=(t*t*(768u-2u*t)+32768u)>>16;
 return (uint8_t)(((uint32_t)s->from*(256u-eased)+(uint32_t)s->to*eased+128u)>>8);
}

static inline uint16_t ru_touch_percent_q15(uint8_t percent){
 return (uint16_t)((uint32_t)percent*32767u/100u);
}
/* Shared by the realtime snapshot and the readonly LCD monitor. The stored
 * FX slot remains at MIN and is not rewritten on every pressure update. */
static inline uint8_t ru_touch_level_percent(const struct ru_touch_profile *p,uint8_t level){
 if(!ru_touch_profile_valid(p))return 0u;
 uint32_t pct=(uint32_t)p->min_pct+
  ((uint32_t)(p->max_pct-p->min_pct)*level+63u)/127u;
 return (uint8_t)(pct>100u?100u:pct);
}
static inline uint8_t ru_touch_reverb_slot(const struct rupture_fx_chain *chain){
 if(!chain)return 255u;
 for(uint8_t i=0u;i<RU_FX_SLOTS;i++)
  if(chain->type[i]==RU_FX_REVERB)return i;
 return 255u;
}
/* Caller holds the normal editor-side FX publication contract. Never
 * overwrite an occupied slot when all eight are in use. Return 0 on FULL. */
static inline uint8_t ru_touch_ensure_reverb(struct rupture_fx_chain *chain,
 uint8_t min_pct){
 if(!chain||min_pct>100u||!chain->independent)return 0u;
 uint8_t slot=ru_touch_reverb_slot(chain);
 if(slot==255u){
  for(uint8_t i=0u;i<RU_FX_SLOTS;i++)if(chain->type[i]==RU_FX_EMPTY){slot=i;break;}
  if(slot==255u)return 0u;
  ru_chain_insert(chain,slot,RU_FX_REVERB);
 }
 chain->enabled|=(uint8_t)(1u<<slot);
 chain->parameter[slot][3]=ru_touch_percent_q15(min_pct);
 return 1u;
}

/* New absolute, user-defined 0..100% range. CXS oscillator/FX formulas
 * are unchanged; only this ALNK block's local controls snapshot moves.
 * At level zero the reverb slot retains its persisted MIN value. */
static inline void ru_touch_apply_profile(struct etch_native_controls *c,
 const struct ru_touch_profile *p,uint8_t level){
 if(!c||!ru_touch_profile_valid(p)||!p->target)return;
 uint8_t pct=ru_touch_level_percent(p,level);
 const uint16_t q=ru_touch_percent_q15(pct);
 switch(p->target){
  case RU_TOUCH_FILTER:c->cutoff=q;return;
  case RU_TOUCH_GROWL:
   if(c->main_wave)c->cxs_pressure=(uint8_t)pct;
   return;
  case RU_TOUCH_SUB1:
   if(c->sub_blend[0])c->sub_blend[0]=(uint8_t)pct;
   return;
  default:break;
 }
 if(!c->fx_chain_active||!c->fx_chain.independent)return;
 for(uint8_t i=0u;i<RU_FX_SLOTS;i++){
  if(!(c->fx_chain.enabled&(1u<<i)))continue;
  uint8_t type=c->fx_chain.type[i];
  if((p->target==RU_TOUCH_REVERB&&type==RU_FX_REVERB)||
     (p->target==RU_TOUCH_MOSFET&&type==RU_FX_MOSFET)||
     (p->target==RU_TOUCH_DISTORTION&&type==RU_FX_DISTORTION)||
     (p->target==RU_TOUCH_WAH&&type==RU_FX_WAH)){
   /* All EFFECT destinations modulate WET/DRY MIX (knob 4).
    * MOSFET historically had no MIX knob and its stored p[3] == 0;
    * 0 must continue to mean legacy 100% wet, preserving all previous
    * projects. A live modulation snapshots q+1, with 1 == dry and
    * 32767 == full MOSFET, without writing any saved project field. */
   c->fx_chain.parameter[i][3]=p->target==RU_TOUCH_MOSFET?
      (uint16_t)(q==32767u?32767u:q+1u):q;
   return; /* Never alter a second instance unintentionally. */
  }
 }
}
#endif
