/* RUPTURE V1.80 native reverb network QA. No hardware timing is asserted. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static uint64_t render(uint8_t typ,uint16_t size,uint16_t decay,int16_t *peak,int64_t *late){
 etch_fx_reset(&fx);
 struct etch_fx_settings s;etch_fx_defaults(&s);
 etch_fx_set_reverb_type(&s,typ);
 s.reverb_size=size;s.reverb_decay=decay;
 uint64_t h=1469598103934665603ULL;
 int32_t pk=0;
 int64_t tail_energy=0;
 for(uint32_t i=0;i<88200u;i++){
  int16_t x=i<11025u?(int16_t)(9600.0*sin(6.283185307179586*92.5*(double)i/44100.0)):0;
  int16_t l=0,r=0;
  etch_fx_process_stereo_masked(&fx,x,0,0,32767u,&s,0,&l,&r);
  int m=l<0?-(int32_t)l:l;
  if((int)r<0&&-(int32_t)r>m)m=-(int32_t)r;
  if((int)r>=0&&r>m)m=r;
  if(m>pk)pk=m;
  assert(m<=28000);
  if(i>15000u&&i<40000u)tail_energy+=(int64_t)l*l+(int64_t)r*r;
  h=(h^(uint16_t)l)*1099511628211ULL;
  h=(h^(uint16_t)r)*1099511628211ULL;
 }
 assert(pk>500);
 *peak=(int16_t)pk;*late=tail_energy;
 return h;
}
int main(void){
 for(uint8_t t=0;t<3;t++){
  int16_t p0,p1,p2,p3,p4;int64_t e0,e1,e2,e3,e4;
  uint64_t a=render(t,0,25000,&p0,&e0);
  uint64_t b=render(t,32767,25000,&p1,&e1);
  uint64_t c=render(t,25000,0,&p2,&e2);
  uint64_t d=render(t,25000,32767,&p3,&e3);
  uint64_t e=render(t,25000,25000,&p4,&e4);
  assert(a!=b&&c!=d&&e0>0&&e1>0&&e4>0);
  printf("type %u size hashes %016llx %016llx; decay tail %lld -> %lld; peak %d\n",
   t,(unsigned long long)a,(unsigned long long)b,(long long)e2,(long long)e3,p4);
 }
 /* Stress rapid topology + size changes, including Hall->Spring after >4096
  * frames, which must never use an out-of-range legacy ring write index. */
 etch_fx_reset(&fx); struct etch_fx_settings cfg;etch_fx_defaults(&cfg);
 for(uint32_t i=0;i<2000000u;i++){
  if((i%12001u)==0){
   etch_fx_set_reverb_type(&cfg,(uint8_t)((i/12001u)%3u));
   cfg.reverb_size=(uint16_t)((i*53u)%32768u);
   cfg.reverb_decay=(uint16_t)((i*101u)%32768u);
  }
  int16_t x=(int16_t)(((i*173u)%32000u)-16000u),l=0,r=0;
  etch_fx_process_stereo_masked(&fx,x,0,0,32767,&cfg,0,&l,&r);
  assert(l<=28000&&l>=-28000&&r<=28000&&r>=-28000);
  assert(fx.reverb_pos<4096u);
  for(uint32_t j=0;j<4u;j++)assert(fx.reverb_line_cursor[j]<2048u);
 }
 puts("PASS V1.80 three reverb networks, real SIZE/DECAY controls, zero overflow, 2m hot-switch samples");
 return 0;
}
