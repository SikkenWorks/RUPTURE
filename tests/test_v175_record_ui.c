#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
int main(void){
 RuState v={0};v.page=RU_PAGE_SEQ;v.seq_length=16;v.seq_bpm=120;v.seq_recording=1;
 ru_render(&v);static uint8_t normal[7200];memcpy(normal,ru_bits,sizeof normal);
 v.seq_countin=1;v.seq_countin_beat=1;ru_render(&v);static uint8_t waiting[7200];memcpy(waiting,ru_bits,sizeof waiting);
 assert(memcmp(normal,waiting,sizeof normal)!=0);
 v.seq_countin_pulse=1;ru_render(&v);
 assert(memcmp(waiting,ru_bits,sizeof waiting)!=0);
 v.seq_countin_beat=5;v.seq_countin_pulse=0;ru_render(&v);
 assert(memcmp(waiting,ru_bits,sizeof waiting)!=0); /* different bar */
 v.seq_countin=0;v.seq_playing=1;v.seq_recording=1;ru_render(&v);
 assert(memcmp(waiting,ru_bits,sizeof waiting)!=0);
 printf("PASS V1.75: ARM, bar1/bar2, grid pulse ON/OFF, normal REC distinguishable on native 240x240 panel\n");
}
