/* V1.76: nearest-grid live capture, click aging and high-frequency MID LED. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_quant.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_live.h"
int main(void){
 uint32_t period=ru_quant_step_period(120u,2u,0u,4u);
 assert(period==5512u);
 assert(ru_quantize_rec_step(4u,16u,period-2u,0u,120u,2u,0u)==4u);
 assert(ru_quantize_rec_step(4u,16u,1000u,0u,120u,2u,0u)==5u);
 /* UI sampled after step transition, physical event was shortly BEFORE it:
  * still quantizes to current boundary instead of mistaking it for the previous. */
 assert(ru_quantize_rec_step(5u,16u,period-1u-250u,300u,120u,2u,0u)==5u);
 /* A press well inside the preceding step, processed late, is assigned back. */
 assert(ru_quantize_rec_step(5u,16u,period-1u-250u,3500u,120u,2u,0u)==4u);
 /* Loop boundary is seamless, both directions. */
 assert(ru_quantize_rec_step(15u,16u,50u,0u,120u,2u,0u)==0u);
 assert(ru_quantize_rec_step(0u,16u,period-1u-100u,3800u,120u,2u,0u)==15u);
 /* Swing uses audio's actual odd/even per-step lengths; not straight 16ths. */
 uint32_t longp=ru_quant_step_period(120u,2u,70u,2u);
 uint32_t shortp=ru_quant_step_period(120u,2u,70u,3u);
 assert(longp>period && shortp<period);
 assert(ru_quantize_rec_step(2u,16u,1000u,0u,120u,2u,70u)==3u);
 assert(ru_quantize_rec_step(3u,16u,shortp-1u,0u,120u,2u,70u)==3u);
 assert(ru_quant_age_samples(240000u)>=439u && ru_quant_age_samples(240000u)<=443u);
 assert(ru_quant_age_samples(3000000u)==0u);
 for(uint8_t bpm=30;bpm<=240;bpm=(uint8_t)(bpm==30?60:bpm==60?120:bpm==120?240:241)){
  for(uint8_t rate=0;rate<=3u;rate++){
   for(uint16_t i=0;i<16u;i++){
    uint32_t p=ru_quant_step_period(bpm,rate,25u,i);
    assert(p>0u);
    assert(ru_quantize_rec_step(i,16u,p-1u,0u,bpm,rate,25u)==i);
    assert(ru_quantize_rec_step(i,16u,0u,0u,bpm,rate,25u)==(i+1u)%16u);
   }
  }
 }
 static struct etch_native_pattern pat;
 struct RuLiveCapture rec={0};
 etch_pattern_clear(&pat);
 assert(etch_seq_put(&pat,3u,60u,100u,1u));
 ru_live_advance(&pat,&rec,2u,16u,0u);
 ru_live_note_on_quantized(&rec,4u,3u,60u,16u,2u); /* early snap */
 assert((rec.pending&(1u<<4u))!=0u);
 assert(!ru_live_advance(&pat,&rec,2u,16u,1u<<4u));
 assert(pat.steps[3].notes[0].length==1u);
 assert(!ru_live_advance(&pat,&rec,3u,16u,1u<<4u)); /* origin begins NOW */
 assert(pat.steps[3].notes[0].length==1u && !(rec.pending&(1u<<4u)));
 assert(ru_live_advance(&pat,&rec,4u,16u,1u<<4u));
 assert(pat.steps[3].notes[0].length==2u);
 assert(ru_live_advance(&pat,&rec,5u,16u,0u));
 assert(pat.steps[3].notes[0].length==3u && !rec.active);
 /* Late-published event with quantized origin in prior step begins at two. */
 etch_pattern_clear(&pat);ru_live_reset(&rec);
 assert(etch_seq_put(&pat,2u,62u,100u,1u));
 ru_live_advance(&pat,&rec,3u,16u,0u);
 ru_live_note_on_quantized(&rec,4u,2u,62u,16u,3u);
 assert(ru_live_advance(&pat,&rec,3u,16u,1u<<4u));
 assert(pat.steps[2].notes[0].length==2u);
 assert(ru_live_advance(&pat,&rec,4u,16u,0u));
 assert(pat.steps[2].notes[0].length==3u);
 puts("PASS V1.76: nearest-grid, timestamp correction, swing, loop wrap, pending/previous-step sustain");
 return 0;
}
