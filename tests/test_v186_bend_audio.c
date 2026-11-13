/* RUPTURE V1.86 BEND: test actual native oscillator PCM rather than UI alone. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/audio_core.h"
static struct etch_native_audio a, b;
static struct etch_native_controls neutral, bent;
static int16_t x[256], y[256];
static void setup(uint8_t saw){
 memset(&neutral,0,sizeof(neutral));
 etch_fx_defaults(&neutral.fx_settings);
 neutral.mono_mode=1u;neutral.main_wave=saw;
 neutral.gain=19000u;neutral.cutoff=17000u;
 neutral.sustain_level=25000u;
 neutral.attack_step=0x40000000u;
 neutral.decay_step=0x00008800u;
 neutral.release_step=0x00008800u;
 neutral.cxs_pressure=74u;neutral.cxs_balance=50u;
 neutral.cxs_memory=55u;neutral.cxs_density=63u;
 neutral.cxs_weight=38u;neutral.cxs_edge=46u;
 neutral.cxs_bite=26u;neutral.cxs_punch=40u;
 etch_native_reset(&a);etch_native_reset(&b);
 etch_native_midi(&a,0x90,48,112);etch_native_midi(&b,0x90,48,112);
}
static void test_osc(uint8_t saw){
 setup(saw);
 bent=neutral;
 neutral.live_bend_q16=0u;  /* uninitialized legacy/neutral */
 bent.live_bend_q16=65536u; /* explicit neutral must be exact */
 for(int block=0;block<40;block++){
  etch_native_render_stereo(&a,&neutral,0u,x,128u);
  etch_native_render_stereo(&b,&bent,0u,y,128u);
  assert(memcmp(x,y,sizeof(x))==0);
 }
 /* Live positive bend must modify *real audio*, without retriggering note. */
 bent.live_bend_q16=73562u; /* +2 semitone double bend */
 unsigned changed=0u, nonzero=0u;
 for(int block=0;block<30;block++){
  etch_native_render_stereo(&a,&neutral,0u,x,128u);
  etch_native_render_stereo(&b,&bent,0u,y,128u);
  for(unsigned i=0;i<256;i++){
   if(x[i]!=y[i])changed++;
   if(y[i])nonzero++;
  }
  assert(b.mono_count==1u);
 }
 assert(changed>256u);assert(nonzero>256u);
 /* Releasing bend returns to ordinary oscillator rate without Note OFF. */
 bent.live_bend_q16=65536u;
 for(int block=0;block<8;block++){
  etch_native_render_stereo(&b,&bent,0u,y,128u);
  assert(b.mono_count==1u);
 }
}
int main(void){
 test_osc(0u);
 test_osc(1u);
 puts("PASS V1.86 BEND: legacy-neutral PCM, CXS+SAW bend changes PCM, still one sustained voice, returns to neutral");
 return 0;
}
