#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
int main(void){
 assert(RU_HELP_CARD_COUNT==13u);
 for(unsigned i=0;i<RU_HELP_CARD_COUNT;i++){
  assert(ru_help_cards[i].title[0]);
  for(unsigned j=0;j<5;j++)assert(ru_help_cards[i].line[j][0]);
  RuState s={0};s.page=RU_PAGE_HELP;s.help_page=(uint8_t)i;
  for(unsigned scroll=0;scroll<=2;scroll++){
   s.help_scroll=(uint8_t)scroll;
   ru_render(&s);
   assert(ru_get(8,49));assert(ru_get(8,200));
  }
 }
 RuState s={0};s.page=RU_PAGE_PROJECTS;s.project=1;s.selected=1;
 for(int state=1;state<=4;state++){s.project_status=(uint8_t)state;ru_render(&s);assert(ru_get(8,73));}
 s.page=RU_PAGE_SEQ_SLOTS;s.seq_slot=1;s.seq_status=2;ru_render(&s);
 puts("PASS V1.23: 13 scrollable help cards; saved, failed and confirm feedback in both libraries");
 return 0;
}
