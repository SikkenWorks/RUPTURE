/* V1.73 independent REC click control routing. Pure host test, no flash. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_metronome.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_advanced_controls_v130.h"
int main(void){
 volatile uint8_t level=65;
 int32_t alg=4;
 assert(!ru_metronome_record_algorithm(&level,&alg,0,1));
 assert(level==65&&alg==4); /* ALGORITHM normal function preserved */
 assert(!ru_metronome_record_algorithm(&level,&alg,1,0));
 assert(level==65&&alg==4); /* paused transport is not live REC */
 assert(ru_metronome_record_algorithm(&level,&alg,1,1));
 assert(level==69&&alg==0); /* live REC routes to click, consumes knob */
 alg=200;assert(ru_metronome_record_algorithm(&level,&alg,1,1));assert(level==100&&alg==0);
 alg=-500;assert(ru_metronome_record_algorithm(&level,&alg,1,1));assert(level==0&&alg==0);
 alg=0;assert(!ru_metronome_record_algorithm(&level,&alg,1,1));
 /* PRESETS BPM function unchanged and available at record tempo too. */
 assert(etch_v130_bpm(120,3)==123);
 assert(etch_v130_bpm(120,-2)==118);
 puts("PASS V1.73: PRESETS BPM behavior, REC-only ALGORITHM volume, 0/100 clamp, normal mode preserved");
 return 0;
}
