/* RUPTURE V2.0.16: reference-tuned and lightly self-modulated GROWL tone.
 * GROWL is ONLY the MAIN oscillator mode; CXS and both subs stay intact.
 * Control-to-coefficient conversion runs ONCE per render/DMA block.
 * PCM: signed 32-bit multiply/shift/branch only; no float, divide or malloc.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_GROWL_TONE_H
#define RUPTURE_GROWL_TONE_H
#include <stdint.h>

typedef struct {
 int32_t low;
 int32_t throat;
} RuGrowlState;

typedef struct {
 uint16_t low_a,throat_a;
 uint16_t body_q10,band_q10,presence_q10;
 uint16_t drive_q8,pos_knee,neg_knee;
 uint16_t fold_q8,level_q8;
} RuGrowlParams;

static inline uint8_t ru_growl_u100(uint8_t value){return value>100u?100u:value;}

/* Four PLAY controls and four SHAPE controls are intentionally very audible:
 * BRIGHT = dark throat -> brilliant, open upper-mid range.
 * EDGE = round -> distorted, asymmetrically folded and aggressively raspy.
 * BODY = thin -> powerful lower-harmonic body.
 * LEVEL = 0 silence; 68 historical unity; 100 a boost.
 * SHAPE EDGE = moving formant emphasis; WEIGHT = low tone weight;
 * BITE = additional folds/drive; PUNCH = raw high-mid presence.
 * The filter kernels remain bounded and require just two 32-bit state words. */
static inline RuGrowlParams ru_growl_make_params(
 uint8_t bright,uint8_t edge,uint8_t body,uint8_t level,
 uint8_t shape_edge,uint8_t shape_weight,uint8_t shape_bite,uint8_t shape_punch){
 bright=ru_growl_u100(bright);edge=ru_growl_u100(edge);
 body=ru_growl_u100(body);level=ru_growl_u100(level);
 shape_edge=ru_growl_u100(shape_edge);shape_weight=ru_growl_u100(shape_weight);
 shape_bite=ru_growl_u100(shape_bite);shape_punch=ru_growl_u100(shape_punch);
 RuGrowlParams p;
 /* Approximately 260 Hz -> 7 kHz. Even BRIGHT=0 must retain an
  * audible bass core, unlike the overattenuated first candidate. */
 p.low_a=1200+(int32_t)bright*(int32_t)bright*2;
 /* Center vowel is lower than the earlier almost-highpass throat: the
  * reference carries significant 2nd/3rd partial alongside partial four. */
 p.throat_a=96+((int32_t)bright*(int32_t)bright)/5;
 p.body_q10=180+(int32_t)body*4+(int32_t)shape_weight*3;
 p.band_q10=100+(int32_t)shape_edge*6+(int32_t)bright*2;
 p.presence_q10=((int32_t)bright*(32+(int32_t)shape_punch*3))>>7;
 /* Keep EDGE=0 clean enough for the full knob range to remain dynamic. */
 p.drive_q8=160+(int32_t)edge*11+(int32_t)shape_bite*3;
 p.pos_knee=26000-(int32_t)edge*110-(int32_t)shape_edge*30;
 p.neg_knee=26000-(int32_t)edge*70-(int32_t)body*30;
 p.fold_q8=((int32_t)edge*80+(int32_t)shape_bite*170)/100;
 if(p.fold_q8>250)p.fold_q8=250;
 p.level_q8=((int32_t)level*256+34)/68;
 return p;
}

/* Seaboard-style expression: low (closed/uh), center (throat/ah), high
 * (bright/strained/eh) are genuinely different vowel/pressure colours.
 * Re-analysis of the supplied D1 reference found strong evolving partials
 * three and four; its changes are not adequately modeled by a linear
 * brightness gain. Nonlinear sweep of BOTH formant poles, pressure and
 * asymmetric fold gives the entire 0..100 range stronger articulation.
 * Coefficients are updated ONCE per audio block; the PCM path is unchanged.
 * 50 is exactly neutral, and 0 never gates the actual oscillator.
 * This is evaluated ONCE per DMA block, never inside the PCM sample loop. */
static inline void ru_growl_apply_expression(RuGrowlParams *p,uint8_t expression){
 int32_t d=(int32_t)ru_growl_u100(expression)-50;
 if(!d)return;
 const int32_t signed_curve=d<0?-(d*d):d*d;
 const int32_t depth=d<0?-d:0;
 /* Closed end keeps a real bass carrier and spectral body, not silence. */
 int32_t a=(int32_t)p->low_a+d*35+signed_curve;
 p->low_a=(uint16_t)(a<1200?1200:a>28000?28000:a);
 /* Upper tract opens more rapidly above neutral to resemble a singing
  * vowel shift rather than just a brighter fixed oscillator. */
 a=(int32_t)p->throat_a+d*13+signed_curve/2;
 p->throat_a=(uint16_t)(a<96?96:a>17000?17000:a);
 a=(int32_t)p->body_q10-d*3-signed_curve/35;
 p->body_q10=(uint16_t)(a<180?180:a>1200?1200:a);
 a=(int32_t)p->band_q10+d*8+signed_curve/16;
 p->band_q10=(uint16_t)(a<100?100:a>1600?1600:a);
 a=(int32_t)p->presence_q10+d*4+signed_curve/18;
 p->presence_q10=(uint16_t)(a<depth*2?depth*2:a>650?650:a);
 a=(int32_t)p->drive_q8+d*11+signed_curve/9;
 p->drive_q8=(uint16_t)(a<(160+depth*2)?160+depth*2:a>2100?2100:a);
 a=(int32_t)p->pos_knee-d*80-signed_curve/9;
 p->pos_knee=(uint16_t)(a<7000?7000:a>30000?30000:a);
 a=(int32_t)p->neg_knee-d*45-signed_curve/12;
 p->neg_knee=(uint16_t)(a<7000?7000:a>30000?30000:a);
 a=(int32_t)p->fold_q8+d*2+signed_curve/28;
 p->fold_q8=(uint16_t)(a<0?0:a>250?250:a);
 /* A closed vowel should be dark, NOT sound like the volume is disappearing.
  * Compensate the energy naturally lost at the bottom of the formant sweep.
  * LEVEL=0 still yields 0, and neutral/upward expression keep their gain. */
 if(depth>30)p->level_q8=(uint16_t)(((int32_t)p->level_q8*
                                  (256+(depth-30)*12))>>8);
}

/* V2.0.18: the physical PRESETS encoder moves in integer detents, but the
 * audible formants must move continuously between them. Interpolate the
 * exact original integer-param snapshots using 8-bit fractional expression.
 * This runs ONCE per DMA block, not once per PCM sample. Integer positions
 * (including neutral 50) remain bit-exact with V2.0.17. */
static inline void ru_growl_apply_expression_q8(RuGrowlParams *p,uint16_t expression_q8){
 uint8_t low=(uint8_t)(expression_q8>>8u);
 uint32_t fraction=(uint32_t)(expression_q8&255u);
 if(low>=100u){ru_growl_apply_expression(p,100u);return;}
 if(!fraction){ru_growl_apply_expression(p,low);return;}
 RuGrowlParams high=*p;
 ru_growl_apply_expression(p,low);
 ru_growl_apply_expression(&high,(uint8_t)(low+1u));
#define RU_GROWL_LERP_FIELD(field) do{ \
 int32_t diff=(int32_t)high.field-(int32_t)p->field; \
 p->field=(uint16_t)((int32_t)p->field+((diff*(int32_t)fraction+128)>>8)); \
}while(0)
 RU_GROWL_LERP_FIELD(low_a);
 RU_GROWL_LERP_FIELD(throat_a);
 RU_GROWL_LERP_FIELD(body_q10);
 RU_GROWL_LERP_FIELD(band_q10);
 RU_GROWL_LERP_FIELD(presence_q10);
 RU_GROWL_LERP_FIELD(drive_q8);
 RU_GROWL_LERP_FIELD(pos_knee);
 RU_GROWL_LERP_FIELD(neg_knee);
 RU_GROWL_LERP_FIELD(fold_q8);
 RU_GROWL_LERP_FIELD(level_q8);
#undef RU_GROWL_LERP_FIELD
}

/* Faster encoder movement briefly introduces extra pressure/rasp; holding
 * still always settles into the selected vowel. No additional samples,
 * oscillators or processing are introduced in the PCM sample loop. */
static inline void ru_growl_apply_motion(RuGrowlParams *p,uint8_t energy){
 int32_t n=energy>36u?36u:energy;
 int32_t a=(int32_t)p->band_q10+n*5;
 p->band_q10=(uint16_t)(a>1600?1600:a);
 a=(int32_t)p->drive_q8+n*9;
 p->drive_q8=(uint16_t)(a>2100?2100:a);
 a=(int32_t)p->fold_q8+n*2;
 p->fold_q8=(uint16_t)(a>250?250:a);
}

/* A second independent drift lane gently shifts the voice tract. Pure
 * block-rate coefficient math; never changes pitch or amplitude envelopes. */
static inline void ru_growl_apply_auto_drift(RuGrowlParams *p,int8_t drift){
 int32_t a=(int32_t)p->throat_a+(int32_t)drift*32;
 p->throat_a=(uint16_t)(a<96?96:a>17000?17000:a);
 a=(int32_t)p->band_q10+(int32_t)drift*5;
 p->band_q10=(uint16_t)(a<100?100:a>1600?1600:a);
}

static inline int32_t ru_growl_bound(int32_t x,int32_t limit){
 if(x>limit)return limit;
 if(x< -limit)return -limit;
 return x;
}

/* Unlike V2.0.11, GROWL now uses BOTH a broad swept throat band and a
 * controllable wavefold before the output ceiling. A single mirrored fold
 * creates new rasp at bass pitch without another audio oscillator/FFT/tank. */
static inline int32_t ru_growl_process(RuGrowlState *s,int32_t raw,
                                      const RuGrowlParams *p){
 int32_t low=s->low+(((raw-s->low)*p->low_a)>>15);
 s->low=low;
 int32_t throat=s->throat+(((low-s->throat)*p->throat_a)>>15);
 s->throat=throat;
 int32_t band=low-throat;
 int32_t wet=((throat*p->body_q10)+(band*p->band_q10)+
              (raw*p->presence_q10))>>10;
 wet=(wet*p->drive_q8)>>8;
 /* Bound the waveshaper input; all subsequent paths fit signed 32-bit. */
 wet=ru_growl_bound(wet,72000);
 int32_t smooth=wet;
 if(smooth>p->pos_knee)smooth=p->pos_knee+((smooth-p->pos_knee)>>4);
 else if(smooth< -p->neg_knee)smooth=-p->neg_knee-(((-smooth)-p->neg_knee)>>4);
 if(p->fold_q8){
  int32_t magnitude=wet<0?-wet:wet;
  int32_t folded=magnitude>20000?40000-magnitude:magnitude;
  if(folded< -24000)folded=-24000;
  if(wet<0)folded=-folded;
  smooth+=((folded-smooth)*p->fold_q8)>>8;
 }
 wet=(smooth*p->level_q8)>>8;
 /* Output ceiling is intentionally SOFT, even with all eight knobs at max. */
 if(wet>27500)wet=27500+((wet-27500)>>3);
 else if(wet< -27500)wet=-27500-(((-wet)-27500)>>3);
 return ru_growl_bound(wet,31000);
}

/* Compatibility for standalone tools. The FM1 uses process() with
 * make_params() cached once per DMA block, never this per-sample wrapper. */
static inline int32_t rupture_growl_tone(RuGrowlState *s,int32_t raw,
 uint8_t bright,uint8_t edge,uint8_t body,uint8_t level,
 uint8_t shape_edge,uint8_t shape_weight,uint8_t shape_bite,uint8_t shape_punch){
 RuGrowlParams params=ru_growl_make_params(bright,edge,body,level,
                               shape_edge,shape_weight,shape_bite,shape_punch);
 return ru_growl_process(s,raw,&params);
}
#endif
