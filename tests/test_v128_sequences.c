#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.h"
static struct etch_pattern_bank b;
static struct etch_native_pattern p,q;
int main(void){
 etch_pattern_bank_clear(&b);etch_pattern_clear(&p);
 p.steps[0].count=1;p.steps[0].notes[0].note=36;
 p.steps[0].notes[0].velocity=100;p.steps[0].notes[0].length=1;
 assert(etch_pattern_bank_capture(&b,24,&p)==0);
 assert(etch_pattern_bank_copy_slot(&b,24,25)==0);
 assert(etch_pattern_bank_fetch(&b,25,&q)==1);
 assert(q.steps[0].count==1&&q.steps[0].notes[0].note==36);
 assert(etch_pattern_bank_delete_slot(&b,24)==0);
 assert(!b.lengths[24] && b.lengths[25]);
 assert(etch_pattern_bank_fetch(&b,25,&q)==1);
 assert(q.steps[0].notes[0].velocity==100);
 assert(etch_pattern_bank_copy_slot(&b,25,20)==0); /* copy BEFORE source; memmove overlap */
 assert(etch_pattern_bank_fetch(&b,20,&q)==1&&q.steps[0].notes[0].note==36);
 assert(etch_pattern_bank_validate(&b)==0);
 puts("PASS V1.28 packed sequence duplicate both directions and delete");return 0;
}
