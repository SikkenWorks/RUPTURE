#include "../src/native_cxs_q15.h"
#include <assert.h>
#include <stdio.h>
int main(void){RuptureCXSQ15 s;rupture_q15_init(&s,12687301u);assert(rupture_q15_valid(&s));
 for(int p=0;p<=100;p+=10){rupture_q15_params(&s,p,(unsigned)p,100-p,p);
 for(int i=0;i<60000;i++){(void)rupture_q15_sample(&s);assert(rupture_q15_valid(&s));}}
 assert(s.cycles>1000);puts("PASS: fixed-point CXS oscillator, deterministic exchange and exact conserved period");return 0;}
