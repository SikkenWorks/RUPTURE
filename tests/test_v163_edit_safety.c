/* Host-only tests: UI alert pixels and the same undo gesture policy compiled into the FM1. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_undo_gesture.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static uint32_t hash(void){uint32_t h=2166136261u;
 for(uint32_t i=0;i<sizeof ru_bits;i++)h=(h^ru_bits[i])*16777619u;return h;}
int main(void){
 RuKnobUndo u={0};
 ru_undo_gesture_capture(&u,RU_PAGE_PLAY,0,58,1000);
 assert(u.valid && u.old_value==58);
 ru_undo_gesture_capture(&u,RU_PAGE_PLAY,0,64,1100);
 assert(u.old_value==58); /* coalesced into one gesture */
 ru_undo_gesture_capture(&u,RU_PAGE_PLAY,1,50,1150);
 assert(u.old_value==50 && u.control==1); /* different knob */
 ru_undo_gesture_capture(&u,RU_PAGE_PLAY,1,54,1900);
 assert(u.old_value==54); /* new gesture after pause */
 RuState v={0};v.page=RU_PAGE_PLAY;v.k[0]=58;v.k[1]=50;v.k[2]=52;v.k[3]=68;
 ru_render(&v);uint32_t before=hash();
 assert(!ru_get(154,35));
 v.project_dirty=1u;ru_render(&v);
 assert(hash()!=before && ru_get(154,35)); /* star within safe header gap */
 v.page=RU_PAGE_PROJECTS;v.project=1;v.active_project=1;v.project_mask=1;
 v.project_names[0][0]='M';v.project_names[0][1]='Y';v.project_names[0][2]='P';
 v.project_status=5;v.project_dirty=0;ru_render(&v);uint32_t clean=hash();
 v.project_dirty=1;ru_render(&v);assert(hash()!=clean); /* discard prompt */
 v.page=RU_PAGE_STORAGE_ACTIONS;v.manager_status=4;v.project_dirty=0;ru_render(&v);clean=hash();
 v.project_dirty=1;ru_render(&v);assert(hash()!=clean); /* alternate manager path */
 ru_storage_popup(1,3,100,1);uint32_t saved=hash();
 ru_storage_popup(1,3,100,2);assert(hash()!=saved);
 ru_storage_popup(2,4,100,1);assert(hash()!=saved);
 puts("PASS V1.63: 1-level knob undo, unsaved marker, double-confirm alerts, verified feedback render");
 return 0;
}
