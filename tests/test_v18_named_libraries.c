#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/wave_store.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_white_steps.h"
static struct etch_wave_library source,decoded;
static struct etch_native_pattern notes,decoded_notes;
static struct etch_project_settings state,decoded_state;
static uint8_t raw[ETCH_PROJECT_MAX_BYTES];
int main(void){
 assert(ru_white_step_key(0)==0u && ru_white_step_key(15)==26u);
 assert(ru_white_step_key(16)==0u && ru_white_step_key(31)==26u);
 assert(__builtin_popcount(ru_white_step_mask(4,0))==4);
 assert(__builtin_popcount(ru_white_step_mask(8,0))==8);
 assert(__builtin_popcount(ru_white_step_mask(16,0))==16);
 assert(__builtin_popcount(ru_white_step_mask(32,23))==16);
 assert(__builtin_popcount(ru_white_step_mask(32,31))==16);
 memset(&source,0,sizeof(source));memset(&state,0,sizeof(state));
 memset(&notes,0,sizeof(notes));
 state.seq_length=16;state.seq_gate=75;state.tempo=120;
 etch_fx_defaults(&state.global_fx);
 source.valid[0]=1u;source.selection[0]=0;source.selection[1]=0;source.selection[2]=0;
 for(int i=0;i<8;i++){
  int id=24+i;
  source.patches[id].valid=1;
  memcpy(source.patches[id].name,"BASS LINE   ",12);
  source.patches[id].name[10]=(char)('0'+(i+1)/10);
  source.patches[id].name[11]=(char)('0'+(i+1)%10);
 }
 uint32_t size=0;
 assert(etch_project_encode(&source,&notes,&state,raw,sizeof(raw),&size)==0);
 assert(etch_project_decode(raw,size,&decoded,&decoded_notes,&decoded_state)==0);
 for(int i=0;i<8;i++){
  assert(decoded.patches[24+i].valid==1);
  assert(!memcmp(decoded.patches[24+i].name,source.patches[24+i].name,12));
 }
 puts("RUPTURE 8 persisted sequence names and white step LED mapping PASS");
 return 0;
}
