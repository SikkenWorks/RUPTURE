/* RUPTURE V2.0.12: high-contrast, voice-like bandlimited GROWL tone.
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
 p.throat_a=96+((int32_t)bright*(int32_t)bright)/3;
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

/* GROWL's one-dimensional PERFORMANCE gesture deliberately moves several
 * independent physical-sounding axes along different curves: vocal tract
 * position (two filters), breath/brightness, pressure/drive and folds.
 * 50 is exactly neutral, preserving existing patch audio bit for bit.
 * This is evaluated ONCE per DMA block, never inside the PCM sample loop. */
static inline void ru_growl_apply_expression(RuGrowlParams *p,uint8_t expression){
 int32_t d=(int32_t)ru_growl_u100(expression)-50;
 if(!d)return;
 /* EXPRESSION=0 must stay audible. Keep the change continuous around 50,
  * retain the original sound at 50 and the original snarl above it. */
 const int32_t dark=d<0;
 const int32_t depth=dark?-d:0;
 const int32_t min_low=dark?1200:240;
 const int32_t min_throat=dark?96:36;
 const int32_t min_body=dark?180:100;
 const int32_t min_band=dark?100:40;
 const int32_t min_presence=depth*2;
 const int32_t min_drive=dark?160+depth*2:128;
 int32_t a=(int32_t)p->low_a+d*32;
 p->low_a=(uint16_t)(a<min_low?min_low:a>28000?28000:a);
 a=(int32_t)p->throat_a+d*7;
 p->throat_a=(uint16_t)(a<min_throat?min_throat:a>17000?17000:a);
 a=(int32_t)p->body_q10-d*2;
 p->body_q10=(uint16_t)(a<min_body?min_body:a>1150?1150:a);
 a=(int32_t)p->band_q10+d*7;
 p->band_q10=(uint16_t)(a<min_band?min_band:a>1450?1450:a);
 a=(int32_t)p->presence_q10+d*3;
 p->presence_q10=(uint16_t)(a<min_presence?min_presence:a>500?500:a);
 a=(int32_t)p->drive_q8+d*11;
 p->drive_q8=(uint16_t)(a<min_drive?min_drive:a>2100?2100:a);
 a=(int32_t)p->pos_knee-d*80;
 p->pos_knee=(uint16_t)(a<7000?7000:a>30000?30000:a);
 a=(int32_t)p->neg_knee-d*45;
 p->neg_knee=(uint16_t)(a<7000?7000:a>30000?30000:a);
 a=(int32_t)p->fold_q8+d*2;
 p->fold_q8=(uint16_t)(a<0?0:a>250?250:a);
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
