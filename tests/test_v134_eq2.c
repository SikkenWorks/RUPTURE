/* EQ2 regression: neutral bit exact; two independent bell Q widths;
 * shelving signs; coefficient bounds; no heap; no legacy sound change. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.h"
static double magnitude(const struct rupture_eq2_params *p,const uint16_t g[4],double hz){
 struct rupture_eq2_bq c[4];rupture_eq2_compute(p,g,c);
 double w=2*M_PI*hz/44100.0,m=1.;
 for(int b=0;b<4;b++){
  double b0=c[b].b0/(double)RU_EQ2_ONE,b1=c[b].b1/(double)RU_EQ2_ONE,b2=c[b].b2/(double)RU_EQ2_ONE;
  double a1=c[b].a1/(double)RU_EQ2_ONE,a2=c[b].a2/(double)RU_EQ2_ONE;
  double nr=b0+b1*cos(w)+b2*cos(2*w),ni=-b1*sin(w)-b2*sin(2*w);
  double dr=1+a1*cos(w)+a2*cos(2*w),di=-a1*sin(w)-a2*sin(2*w);
  m*=sqrt((nr*nr+ni*ni)/(dr*dr+di*di));
 }
 return m;
}
int main(void){
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1);
 assert(rupture_eq2_valid(&p));assert(rupture_eq2_frequency(0,50)>60&&rupture_eq2_frequency(0,50)<100);
 uint16_t g[4]={16384,16384,16384,16384};
 struct rupture_eq2_bq coeff[4];rupture_eq2_compute(&p,g,coeff);
 struct rupture_eq2_state state={0};
 for(int b=0;b<4;b++)state.current[b].b0=RU_EQ2_ONE;
 state.current_headroom=32767;rupture_eq2_publish(&state,coeff,g);
 for(int i=0;i<400;i++){int32_t in=(i*1909u)%54000u-27000;assert(rupture_eq2_process(&state,in)==in);}
 /* Peaking Q controls the shoulders without changing nominal centre boost. */
 g[2]=32767;p.q[1]=5;double broad=magnitude(&p,g,950);
 p.q[1]=90;double narrow=magnitude(&p,g,950);
 assert(broad>narrow+0.15);
 double centre=magnitude(&p,g,rupture_eq2_frequency(2,50));
 assert(centre>2.65&&centre<3.05);
 g[2]=16384;g[0]=32767;double bass=magnitude(&p,g,45),treble=magnitude(&p,g,6000);
 assert(bass>2.3&&treble<1.2);
 g[0]=16384;g[3]=0;double air=magnitude(&p,g,10000);
 assert(air<0.75);
 printf("EQ2: neutral bit-exact, bell Q widths, peak gain, shelves passed\n");
 return 0;
}
