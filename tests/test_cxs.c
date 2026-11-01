#include "../src/cxs.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
int main(void){
 CXSEngine e;cxs_init(&e);assert(cxs_verify(&e));
 for(int p=0;p<=100;p+=10){CXSParams k=e.knob;k.pressure=p;k.memory=100-p;k.balance=p;k.density=p;cxs_set(&e,&k);cxs_note_on(&e,36,110,1);
 for(int i=0;i<44100;i++){float s=cxs_tick(&e);assert(isfinite(s));assert(fabsf(s)<=.951f);assert(cxs_verify(&e));}
 cxs_note_off(&e,36);for(int i=0;i<10000;i++)(void)cxs_tick(&e);cxs_panic(&e);assert(e.env==0.f);}
 cxs_note_on(&e,36,100,1);cxs_note_on(&e,40,100,0);cxs_note_on(&e,43,100,0);
 assert(e.held_count==3&&e.note==43);cxs_note_off(&e,43);assert(e.note==40);cxs_note_off(&e,40);assert(e.note==36);
 puts("PASS: 11 parameter sweeps, cycle conservation, bounded PCM, mono note stack, panic");return 0;
}
