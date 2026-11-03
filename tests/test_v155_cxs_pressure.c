/* RUPTURE V1.55: PRESSURE must change sustained CXS colour over its full range.
 * Exercises the exact native kernel built into FM1 firmware, not a UI mock.
 * No filesystem/device access needed. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.h"
#define N 44100
static int16_t reference[N];
static int16_t latest[N];
static uint64_t power(const int16_t *a){uint64_t p=0;for(int i=N/2;i<N;i++){int32_t n=a[i];p+=(uint64_t)(n*n);}return p;}
static uint64_t delta_power(const int16_t *a,const int16_t *b){uint64_t p=0;for(int i=N/2;i<N;i++){int32_t d=(int32_t)a[i]-(int32_t)b[i];p+=(uint64_t)(d*d);}return p;}
static void render(uint8_t pressure,uint32_t pitch_inc,int16_t *out){
 RuptureCXSQ15 s;rupture_q15_init(&s,pitch_inc);
 rupture_q15_params(&s,pressure,50,52,68);s.edge=50;
 for(int i=0;i<N;i++){
  out[i]=rupture_q15_sample(&s);
  if((i&127)==0)assert(rupture_q15_valid(&s));
 }
 assert(rupture_q15_valid(&s));
}
static void test_pitch(uint32_t pitch_inc){
 const uint8_t values[]={0,10,25,50,75,100};
 render(0,pitch_inc,reference);
 uint64_t previous=0;
 for(unsigned k=1;k<sizeof(values);k++){
  render(values[k],pitch_inc,latest);
  uint64_t val=power(latest);
  uint64_t diff=delta_power(reference,latest);
  /* Each setting has significant sustained output difference, far above
   * integer rounding/quantisation. Use broad, portable thresholds. */
  assert(diff>1000000000ULL);
  if(k>1)assert(val>previous+100000000ULL);
  previous=val;
  printf("PRESSURE=%u power=%llu diff_from_zero=%llu\n",
    (unsigned)values[k],(unsigned long long)val,(unsigned long long)diff);
 }
}
static void test_live_change(void){
 RuptureCXSQ15 s;rupture_q15_init(&s,12687301u);
 for(int i=0;i<160000;i++){
  uint8_t p=(uint8_t)((i/128)%101); /* continuous live knob sweep */
  rupture_q15_params(&s,p,50,52,68);
  int16_t sample=rupture_q15_sample(&s);
  (void)sample;
  if(!(i%1024))assert(rupture_q15_valid(&s));
 }
 for(int pressure=0;pressure<=100;pressure+=10){
  for(int density=0;density<=100;density+=25){
   rupture_q15_params(&s,(uint8_t)pressure,97,0,(uint8_t)density);
   for(int t=0;t<4096;t++)(void)rupture_q15_sample(&s);
   assert(rupture_q15_valid(&s));
  }
 }
}
int main(void){
 test_pitch(12687301u); /* ~130 Hz at 44.1 kHz */
 test_pitch(50500000u); /* ~519 Hz */
 test_live_change();
 puts("PASS: V1.55 native CXS PRESSURE across 0..100, live sweep, parameter extremes");
 return 0;
}
