#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_policy.h"
int main(void){
 struct etch_native_pattern p={0};
 assert(!rupture_step_velocity(&p,0,80));
 p.steps[0].count=2; p.steps[0].notes[0].velocity=100; p.steps[0].notes[1].velocity=90;
 assert(rupture_step_velocity(&p,0,48)==2);assert(p.steps[0].notes[0].velocity==48);assert(p.steps[0].notes[1].velocity==48);
 assert(!rupture_step_velocity(&p,0,0));assert(!rupture_step_velocity(&p,ETCH_SEQ_STEPS,60));
 assert(!rupture_autosave_due(10000,1,0,1,1,0,0,0));
 assert(rupture_autosave_due(30000,1,0,1,1,0,0,0));
 assert(!rupture_autosave_due(30000,1,0,1,0,0,0,0));
 assert(!rupture_autosave_due(30000,1,0,1,1,1,0,0));
 assert(!rupture_autosave_due(30000,1,0,1,1,0,1,0));
 assert(!rupture_autosave_due(30000,1,0,1,1,0,0,1));
 assert(!rupture_autosave_due(30000,1,10000,1,1,0,0,0));
 assert(rupture_autosave_due(90000,60000,20000,1,1,0,0,0));
 puts("RUPTURE V1.7 sequence velocity / guarded autosave PASS");return 0;
}
