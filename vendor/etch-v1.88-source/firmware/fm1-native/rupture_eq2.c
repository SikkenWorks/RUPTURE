/* SPDX-License-Identifier: GPL-3.0-only */
#include "rupture_eq2.h"
#include "rupture_eq2_tables.h"
#include <stdint.h>
#define ONE ((int64_t)RU_EQ2_ONE)
static int64_t mul(int64_t a,int64_t b){return (a*b)>>RU_EQ2_SHIFT;}
static int32_t clamp(int64_t x){return x>INT32_MAX?INT32_MAX:x<INT32_MIN?INT32_MIN:(int32_t)x;}
/* Native pi32v2 links without compiler-rt __divdi3. This bounded software
 * divider runs ONLY on explicit parameter edits, never in DMA/audio IRQ. */
static uint64_t ru_udiv64(uint64_t n,uint64_t d){
 if(d==0u)return 0u;
 uint64_t rem=0u,quot=0u;
 for(int i=63;i>=0;i--){
  rem=(rem<<1u)|((n>>(uint32_t)i)&1u);
  if(rem>=d){rem-=d;quot|=(uint64_t)1u<<(uint32_t)i;}
 }
 return quot;
}
static int64_t ru_div64(int64_t n,int64_t d){
 if(!d)return 0;
 uint8_t neg=(n<0)^(d<0);
 uint64_t un=(uint64_t)(n<0?-n:n),ud=(uint64_t)(d<0?-d:d);
 uint64_t q=ru_udiv64(un,ud);
 return neg?-(int64_t)q:(int64_t)q;
}
static int32_t divq(int64_t n,int64_t d){if(d<=0)return 0;return clamp(ru_div64(n*ONE,d));}
uint16_t rupture_eq2_frequency(uint8_t band,uint8_t index){return ru_eq2_freq[band%4][index>100?100:index];}
uint16_t rupture_eq2_gain_index(uint16_t raw){
 /* 0..32767 maps exactly to -9..+9dB in 0.5dB increments. */
 if(raw==16384u)return 18;
 uint32_t n=((uint32_t)raw*36u+16383u)/32767u;
 return n>36u?36u:(uint16_t)n;
}
void rupture_eq2_compute(const struct rupture_eq2_params *p,const uint16_t gains[4],struct rupture_eq2_bq out[4]){
 for(int band=0;band<4;band++){
  int f=p->freq[band]>100?100:p->freq[band];
  int gi=rupture_eq2_gain_index(gains[band]);
  int64_t a=ru_eq2_a[gi],cosw=ru_eq2_cos[band][f],sinw=ru_eq2_sin[band][f];
  int64_t b0,b1,b2,a0,a1,a2;
  if(gi==18){out[band]=(struct rupture_eq2_bq){(int32_t)ONE,0,0,0,0};continue;}
  if(band==1||band==2){
   int64_t q=ru_eq2_q[p->q[band-1]>100?100:p->q[band-1]];
   int64_t alpha=ru_div64(sinw*ONE,q*2);
   int64_t aa=mul(alpha,a),ia=ru_div64(alpha*ONE,a);
   b0=ONE+aa;b1=-2*cosw;b2=ONE-aa;
   a0=ONE+ia;a1=-2*cosw;a2=ONE-ia;
  }else{
   /* RBJ low/high shelves, slope S=1, alpha=sin(w)/sqrt(2).
    * t = 2*sqrt(A)*alpha = sqrt(2)*sqrt(A)*sin(w) */
   int64_t t=mul(mul(ru_eq2_sqrta[gi],sinw),94906266LL); /* sqrt(2) Q26 */
   int64_t ap=ONE+a,am=a-ONE;
   int64_t sign=band==0?-1:1;
   int64_t z=ap+sign*mul(am,cosw);
   b0=mul(a,z+t);b2=mul(a,z-t);
   b1=band==0?2*mul(a,am-mul(ap,cosw)):-2*mul(a,am+mul(ap,cosw));
   a0=ap-sign*mul(am,cosw)+t;
   a1=band==0?-2*(am+mul(ap,cosw)):2*(am-mul(ap,cosw));
   a2=ap-sign*mul(am,cosw)-t;
  }
  out[band].b0=divq(b0,a0);out[band].b1=divq(b1,a0);out[band].b2=divq(b2,a0);
  out[band].a1=divq(a1,a0);out[band].a2=divq(a2,a0);
 }
}
void rupture_eq2_publish(struct rupture_eq2_state *s,const struct rupture_eq2_bq coeff[4],const uint16_t gains[4]){
 uint8_t next=(uint8_t)(s->index^1u);
 for(int i=0;i<4;i++)s->target[next][i]=coeff[i];
 uint32_t halfdb=0;
 for(int b=0;b<4;b++){
  int g=(int)rupture_eq2_gain_index(gains[b])-18;
  if(g>0)halfdb+=(uint32_t)g;
 }
 if(halfdb>72)halfdb=72;
 s->headroom_q15[next]=ru_eq2_auto_trim[halfdb];
 __asm__ volatile("" ::: "memory");s->index=next;
}
uint8_t rupture_eq2_is_unity(const struct rupture_eq2_state *s){
 if(s->current_headroom!=32767u || s->headroom_q15[s->index]!=32767u)return 0u;
 for(uint8_t b=0u;b<4u;b++){
  const struct rupture_eq2_bq *v=&s->current[b],*t=&s->target[s->index][b];
  if(v->b0!=RU_EQ2_ONE||v->b1||v->b2||v->a1||v->a2||
     t->b0!=RU_EQ2_ONE||t->b1||t->b2||t->a1||t->a2)return 0u;
 }
 return 1u;
}
int32_t rupture_eq2_process(struct rupture_eq2_state *s,int32_t x){
 if(rupture_eq2_is_unity(s))return x;
 /* No division, floating point or allocation in real-time callback.
  * Integer coefficients are slewed per-sample; DF1 delay states are clamped. */
 uint8_t idx=s->index;
 int32_t trim=s->current_headroom;
 int32_t diff=(int32_t)s->headroom_q15[idx]-trim;
 if(diff){int32_t step=diff/64;if(!step)step=diff>0?1:-1;trim+=step;s->current_headroom=(uint16_t)trim;}
 x=(int32_t)(((int64_t)x*trim)/32767);
 for(int i=0;i<4;i++){
  struct rupture_eq2_bq *c=&s->current[i];
  const struct rupture_eq2_bq *t=&s->target[idx][i];
  int32_t *v=&c->b0;const int32_t *u=&t->b0;
  for(int j=0;j<5;j++){
   int64_t delta=(int64_t)u[j]-v[j];
   if(delta){int32_t step=(int32_t)(delta/64);if(!step)step=delta>0?1:-1;v[j]+=step;}
  }
  /* A neutral band within an active EQ needs only to preserve its input
   * history for later automation. The original multiply-accumulate is
   * exactly x with (ONE,0,0,0,0), so skip five 64-bit products. */
  if(c->b0==RU_EQ2_ONE && !c->b1 && !c->b2 && !c->a1 && !c->a2){
   s->x2[i]=s->x1[i];s->x1[i]=x;
   s->y2[i]=s->y1[i];s->y1[i]=x;
   continue;
  }
  int64_t y64=(int64_t)c->b0*x+(int64_t)c->b1*s->x1[i]+(int64_t)c->b2*s->x2[i]
             -(int64_t)c->a1*s->y1[i]-(int64_t)c->a2*s->y2[i];
  int32_t y=clamp(y64>>RU_EQ2_SHIFT);
  if(y>131071)y=131071;else if(y<-131071)y=-131071;
  s->x2[i]=s->x1[i];s->x1[i]=x;s->y2[i]=s->y1[i];s->y1[i]=y;x=y;
 }
 return x;
}

/* The 54 display points are derived from the SAME actual biquad coefficients
 * used by the audio engine. Called in UI/control context only. */
static int32_t ru_eq_log2_256(uint64_t n){
 if(!n)return -16384;
 uint32_t b=0u;uint64_t p=n;
 while(p>>1u){p>>=1u;b++;}
 uint64_t mant=b>=8u?(n>>(b-8u)):(n<<(8u-b));
 uint32_t idx=(uint32_t)(mant-256u);
 if(idx>255u)idx=255u;
 return (int32_t)(b*256u+ru_eq2_log2_frac[idx]);
}
void rupture_eq2_response_curve(const struct rupture_eq2_bq c[4],uint8_t out[54]){
 for(int f=0;f<54;f++){
  int64_t co=ru_eq2_axis_cos[f],si=ru_eq2_axis_sin[f];
  int64_t c2=2*mul(co,co)-ONE,s2=2*mul(si,co);
  int32_t total=0;
  for(int b=0;b<4;b++){
   int64_t nr=c[b].b0+mul(c[b].b1,co)+mul(c[b].b2,c2);
   int64_t ni=mul(c[b].b1,si)+mul(c[b].b2,s2);
   int64_t dr=ONE+mul(c[b].a1,co)+mul(c[b].a2,c2);
   int64_t di=mul(c[b].a1,si)+mul(c[b].a2,s2);
   uint64_t pn=(uint64_t)(nr*nr+ni*ni),pd=(uint64_t)(dr*dr+di*di);
   int32_t delta=ru_eq_log2_256(pn)-ru_eq_log2_256(pd);
   total+=(delta*301)/2560; /* dB * 10; ratio is of squared magnitudes */
  }
  int32_t y=119-total*35/180;
  if(y<80)y=80;
  if(y>157)y=157;
  out[f]=(uint8_t)y;
 }
}
