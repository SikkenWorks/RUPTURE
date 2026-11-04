/* Pixel-exact RUPTURE V1.68 sequencer LCD regression, 240x240 framebuffer. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static uint32_t digest(void){uint32_t h=2166136261u;for(unsigned i=0;i<sizeof ru_bits;i++){h^=ru_bits[i];h*=16777619u;}return h;}
int main(void){
 RuState v={0};v.page=RU_PAGE_SEQ;v.seq_length=16u;v.seq_bpm=120u;
 v.seq_current_step=0u;v.seq_playing=1u;v.seq_playhead=0u;v.seq_grid_mask=1u;
 ru_render(&v);uint32_t step0=digest();
 v.seq_playhead=1u;ru_render(&v);uint32_t step1=digest();assert(step0!=step1);
 v.seq_playhead=15u;ru_render(&v);uint32_t step15=digest();assert(step15!=step1 && step15!=step0);
 v.seq_playhead=0u;ru_render(&v);assert(digest()==step0);
 v.seq_recording=1u;ru_render(&v);uint32_t rec0=digest();assert(rec0!=step0);
 v.seq_playhead=1u;ru_render(&v);uint32_t rec1=digest();assert(rec1!=rec0);
 v.seq_playing=0u;v.seq_recording=0u;v.seq_current_step=0u;ru_render(&v);uint32_t edit0=digest();
 v.seq_grid_mask=0u;ru_render(&v);assert(digest()!=edit0); /* step-1 note visibly removed */
 puts("PASS V1.68: step 1/2/16 live playhead, wraparound, REC label, step 1 erase in packed 240x240 framebuffer");
 return 0;
}
