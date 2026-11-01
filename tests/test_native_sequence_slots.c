#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.h"

int main(void){
 static struct etch_pattern_bank archive;
 static struct etch_native_pattern source, loaded;
 etch_pattern_bank_clear(&archive);
 for(unsigned slot=0;slot<8;slot++){
  etch_pattern_clear(&source);
  assert(etch_seq_put(&source,slot,(uint8_t)(36+slot),110,1));
  assert(etch_pattern_bank_capture(&archive,(uint8_t)(24+slot),&source)==0);
 }
 assert(etch_pattern_bank_validate(&archive)==0);
 /* 4, 8, 16, 32, 64, 128-step operations rely on the same native 256-step pattern. */
 {
  static const uint16_t lens[]={4,8,16,32,64,128};
  etch_pattern_clear(&source);
  for(unsigned z=0;z<6;z++){
   unsigned st=lens[z]-1u;
   assert(etch_seq_toggle(&source,(uint16_t)st,(uint8_t)(36+z),110,1)==1);
   assert(source.steps[st].count==1);
   assert(etch_seq_toggle(&source,(uint16_t)st,(uint8_t)(36+z),110,1)==2);
   assert(source.steps[st].count==0);
  }
  /* Two different pitches coexist; step selection never moves the stored notes. */
  assert(etch_seq_toggle(&source,3,36,90,1)==1);
  assert(etch_seq_toggle(&source,3,40,110,1)==1);
  assert(source.steps[3].count==2);
  assert(etch_seq_toggle(&source,3,36,90,1)==2);
  assert(source.steps[3].count==1 && source.steps[3].notes[0].note==40);
 }
 for(unsigned slot=0;slot<8;slot++){
  etch_pattern_clear(&loaded);
  assert(etch_pattern_bank_fetch(&archive,(uint8_t)(24+slot),&loaded)==1);
  assert(loaded.steps[slot].count==1);
  assert(loaded.steps[slot].notes[0].note==36+slot);
 }
 /* Invalid/corrupt archived records must not overwrite live pattern. */
 etch_pattern_clear(&loaded);
 loaded.steps[0].notes[0].note=75;
 unsigned len=archive.lengths[24];
 assert(len>3);
 archive.data[6]^=0xff;
 assert(etch_pattern_bank_fetch(&archive,24,&loaded)<0);
 assert(loaded.steps[0].notes[0].note==75);
 archive.data[6]^=0xff;
 assert(etch_pattern_bank_validate(&archive)==0);
 puts("PASS: 8 reserved native SEQ slots, distinct notes, corruption isolation");
 return 0;
}
