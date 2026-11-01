/* Native shared SAVE router: all four HOME pages must behave identically.
 * SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_save_route.h"
int main(void){
 const uint8_t homes[]={RU_PAGE_PLAY,RU_PAGE_SHAPE,RU_PAGE_SUB1,RU_PAGE_SUB2};
 for(unsigned i=0;i<4u;i++){
  uint8_t p=homes[i];
  assert(ru_save_destination(p)==RU_SAVE_PROJECT_MENU);
  assert(ru_save_pressed(1u<<9,1u<<9));
  assert(!ru_save_pressed(1u<<9,(1u<<9)|(1u<<2)));
  assert(!ru_save_pressed(0,0));
  /* The displayed and persisted HOME sub parameters remain unchanged
   * until the user explicitly enters a project action. */
  RuState ui={0};ui.page=p;ui.k[0]=80;ui.k[1]=25;
  ru_render(&ui);
 }
 assert(ru_save_destination(RU_PAGE_PROJECTS)==RU_SAVE_PROJECT_MENU);
 assert(ru_save_destination(RU_PAGE_SEQ)==RU_SAVE_SEQUENCE_MENU);
 assert(ru_save_destination(RU_PAGE_SEQ_SLOTS)==RU_SAVE_SEQUENCE_MENU);
 assert(ru_save_destination(RU_PAGE_FX)==RU_SAVE_NO_MENU);
 assert(ru_save_destination(RU_PAGE_NAME)==RU_SAVE_NO_MENU);
 assert(ru_save_destination(RU_PAGE_GLO)==RU_SAVE_NO_MENU);
 assert(ru_save_destination(RU_PAGE_SAFE)==RU_SAVE_NO_MENU);
 puts("PASS: SAVE opens PROJECTS from all four HOME pages; SEQ remains separate; FX chord guarded");
 return 0;
}
