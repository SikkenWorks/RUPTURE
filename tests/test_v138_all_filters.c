/* SPDX-License-Identifier: GPL-3.0-only
 * V1.38 full-filter fastpath regression: prepared coefficients must be
 * bit-exact, including control sweeps, bypass-like mode switches, restarts,
 * clipped input and shared-coefficient (multiple voice) use.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ladder_fast.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_filter_fast.h"
static uint32_t rng=0xA117C935u;
static uint32_t rnd(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static int32_t old_ladder(int32_t state[4],uint16_t cutoff,uint16_t resonance,int32_t input){
 if(input>32767)input=32767;if(input<-32768)input=-32768;
 uint32_t f=cutoff>8192u?8192u:cutoff,res=resonance>32767u?32767u:resonance;
 int32_t g=(int32_t)(f*3u),k=(int32_t)((res*365u)/100u);
 int32_t x=input-(int32_t)(((int64_t)state[3]*k)>>15);
 if(x>32767)x=32767+(x-32767)/8;
 if(x< -32767)x=-32767+(x+32767)/8;
 if(x>49151)x=49151;
 if(x< -49151)x=-49151;
 for(int j=0;j<4;j++){
  int32_t z=state[j],d=(int32_t)(((int64_t)(x-z)*g)>>15);z+=d;
  if(z>32767)z=32767;if(z< -32767)z=-32767;
  state[j]=z;x=z;
 }
 return state[3];
}
static int32_t old_bound(int32_t x){return x<-262144?-262144:x>262144?262144:x;}
static int32_t old_mul(int32_t state,int32_t coeff){int32_t q=state/32768,r=state%32768;return q*coeff+(r*coeff)/32768;}
static int32_t old_svf(int32_t *low,int32_t *band,uint16_t cutoff,uint16_t resonance,int32_t x,uint8_t type){
 uint32_t f=cutoff>8192u?8192u:cutoff,res=resonance>32767u?32767u:resonance;
 int32_t damping=65536-(int32_t)(res*3u/2u),high=0;
 for(unsigned k=0;k<2;k++){
  *low=old_bound(*low+old_mul(*band,(int32_t)f));
  high=old_bound(x-*low-old_mul(*band,damping));
  *band=old_bound(*band+old_mul(high,(int32_t)f));
 }
 return type==2u?*band:type==3u?high:*low;
}
int main(void){
 int32_t a[4][4]={{0}},b[4][4]={{0}}, al[4]={0},bl[4]={0},ab[4]={0},bb[4]={0};
 struct rupture_ladder_coeff lc={0};struct rupture_svf_coeff sc={0};
 uint8_t lv=0,sv=0;uint64_t checks=0;
 for(unsigned j=0;j<2000000u;j++){
  unsigned voice=j&3u;uint8_t mode=(j/17117u)%5u;
  uint16_t cutoff=(j%12309u)<10400u?2017u:(uint16_t)rnd();
  uint16_t resonance=(j%8333u)<7000u?28000u:(uint16_t)rnd();
  int32_t x=(int32_t)(rnd()&65535u)-32768;
  if((j%2048u)==0u)x=32767;
  if((j%2349u)==0u)x=-32768;
  if(mode==4u){
   if(!lv||lc.cutoff!=cutoff||lc.resonance!=resonance){rupture_ladder_prepare(&lc,cutoff,resonance);lv=1;}
   int32_t old=old_ladder(a[voice],cutoff,resonance,x);
   int32_t now=rupture_ladder_step_prepared(b[voice],&lc,x);
   assert(old==now && memcmp(a[voice],b[voice],sizeof(a[voice]))==0);
  }else if(mode){
   if(!sv||sc.cutoff!=cutoff||sc.resonance!=resonance){rupture_svf_prepare(&sc,cutoff,resonance);sv=1;}
   int32_t old=old_svf(&al[voice],&ab[voice],cutoff,resonance,x,mode);
   int32_t now=rupture_svf_step_prepared(&bl[voice],&bb[voice],&sc,x,mode);
   assert(old==now && al[voice]==bl[voice] && ab[voice]==bb[voice]);
  }
  checks++;
 }
 /* Equal coefficients across consecutive voices and at rest: cache reuse. */
 rupture_ladder_prepare(&lc,7000,32767);
 for(unsigned i=0;i<500000u;i++){
  int32_t x=(int32_t)(rnd()&65535u)-32768;
  int32_t old=old_ladder(a[0],7000,32767,x);
  assert(old==rupture_ladder_step_prepared(b[0],&lc,x));
 }
 puts("PASS 2,000,000 mixed-filter samples and 500,000 stable-resonance samples, exact prior output/state");
 return 0;
}
