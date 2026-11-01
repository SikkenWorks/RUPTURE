/* RUPTURE V1.41: regressions for EQ2 bass boost / Q / 4-band combos.
 * Host simulation, not FM1 DMA hardware validation. */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx_bypass_v181.h"
#define PI 3.14159265358979323846
static struct etch_native_fx fx;
static struct etch_fx_settings c;
static struct rupture_eq2_params p;
static uint16_t g[4];
static struct rupture_eq2_bq coeff[4];
static void setup(void){etch_fx_reset(&fx);etch_fx_defaults(&c);rupture_eq2_defaults(&p,1);fx.eq2_enabled=1;for(int i=0;i<4;i++)g[i]=16384;}
static void publish(void){rupture_eq2_compute(&p,g,coeff);rupture_eq2_publish(&fx.eq2,coeff,g);}
static double tone(int hz,int amp,int n){double sum=0;for(int i=0;i<n;i++){int16_t x=(int16_t)lrint(amp*sin(2*PI*i*hz/44100.));int16_t y=etch_fx_process_ex(&fx,x,0,0,0,&c);if(i>n/2)sum+=(double)y*y;}return sqrt(sum/(n/2));}
int main(void){
 setup();publish();double dry=tone(80,3500,44100);assert(dry>2300&&dry<2600);
 g[0]=32767;publish();double boosted=tone(80,3500,44100);
 printf("bass+9: dry %.1f boosted %.1f ratio %.2f\n",dry,boosted,boosted/dry);
 assert(boosted>dry*2.3);
 /* Earlier auto trim multiplied -9dB into the entire signal, so 4 boosts
  * could produce a mute-like -36dB even though the response indicated +9. */
 for(int i=0;i<4;i++)g[i]=32767;publish();double multi=tone(80,2200,44100);
 printf("all+9dB: dry-ref %.1f, multi %.1f\n",dry,multi);
 assert(multi>dry*.8);
 /* Simulate frequency/Q changes while all bands remain active. */
 for(int i=0;i<160;i++){
  p.freq[i%4]=(uint8_t)((i*71)%101);p.q[i%2]=(uint8_t)((i*43)%101);
  g[i%4]=(uint16_t)((i%3==0)?0:(i%3==1?16384:32767));publish();
  for(int j=0;j<512;j++){
   int16_t x=(int16_t)(((i*599u+j*127u)%14500u)-7250);
   int16_t y=etch_fx_process_ex(&fx,x,0,0,0,&c);
   (void)y;
  }
  assert(fx.eq2.current_headroom==32767);
  assert(fx.eq2.headroom_q15[fx.eq2.index]==32767);
 }
 /* All gains exactly 0dB and settled => full bypass. */
 for(int i=0;i<4;i++)g[i]=16384;publish();
 for(int i=0;i<6000;i++)(void)etch_fx_process_ex(&fx,(int16_t)(i%200-100),0,0,0,&c);
 assert(rupture_eq2_is_unity(&fx.eq2));
 for(int i=0;i<500;i++){int16_t in=(int16_t)(i*99-22000);assert(etch_fx_process_ex(&fx,in,0,0,0,&c)==in);}
 /* A previously saved boosted EQ must become EXACT dry when EQ is OFF,
  * including hot input near full-scale. No phantom wet limiter attenuation. */
 c.eq_low=32767u;c.eq_mid=0u;c.eq_high=32767u;c.eq_hpf=14000u;
 for(int i=0;i<1500;i++){
  int16_t in=(int16_t)((i*31337u)%60001u-30000),l=0,r=0;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&c,ETCH_FXB_BIT(ETCH_FXB_EQ),&l,&r);
  assert(l==in&&r==in);
 }
 puts("PASS V1.41: bass boost, four-band high boost, active Q/frequency sweeps and dry bypass");
}
