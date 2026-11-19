#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
int main(void){
 /* Later versions append two persistent GROWL expression assignments
  * (94/95). Keep old IDs unchanged and test the expanded list. */
 assert(RU_PRESET_ASSIGN_COUNT>=96u && RU_PRESET_ASSIGN_COUNT<128u);
 assert(RU_PRESET_ASSIGN_EXPRESSION==94u);
 assert(RU_PRESET_ASSIGN_EXPRESSION_RETURN==95u);
 assert(ru_preset_targets[0].page==RU_PRESET_ASSIGN_OFF_PAGE);
 assert(ru_preset_targets[1].page==RU_PAGE_SEQ&&ru_preset_targets[1].control==1u);
 unsigned guide_root=0u;
 for(uint8_t id=0u;id<RU_PRESET_ASSIGN_COUNT;id++){
  const RuPresetAssign *t=&ru_preset_targets[id];
  assert(t->group<RU_PRESET_GROUP_COUNT);
  assert(t->page==RU_PRESET_ASSIGN_OFF_PAGE||t->page<RU_PAGE_COUNT);
  if(t->group==9u)
   assert(t->control==4u||t->control==5u);
  else assert(t->control<=RU_PRESET_ASSIGN_SUB_BLEND);
  if(id)assert(ru_preset_targets[id-1u].group<=t->group);
  if(t->page==RU_PAGE_GUIDE&&t->control==0u)guide_root=id;
 }
 assert(guide_root>1u);
 assert(ru_preset_targets[guide_root].group==4u);
 for(uint8_t group=0;group<RU_PRESET_GROUP_COUNT;group++){
  uint8_t first=ru_preset_group_first(group),last=ru_preset_group_last(group);
  assert(first<last&&last<=RU_PRESET_ASSIGN_COUNT);
  for(uint8_t t=first;t<last;t++)assert(ru_preset_targets[t].group==group);
 }
 RuState s={0};s.page=RU_PAGE_SEQ;s.seq_length=128u;
 s.seq_current_step=127u;s.seq_first_note=36u;s.seq_note_count=3u;
 s.seq_sustain=12u;s.seq_slide=1u;
 ru_render(&s);
 /* The information area must not draw anywhere in the y=151..155 gap,
  * and is always above the 4 knob rings that start near y=156. */
 for(int y=150;y<=155;y++)for(int x=6;x<234;x++)assert(!ru_get(x,y));
 s.seq_note_count=0u;s.seq_slide=0u;ru_render(&s);
 for(int y=150;y<=155;y++)for(int x=6;x<234;x++)assert(!ru_get(x,y));
 s.page=RU_PAGE_SETTINGS;s.settings_open=2u;s.preset_category=4u;ru_render(&s);
 s.settings_open=3u;s.preset_cursor=(uint8_t)guide_root;s.preset_assign=(uint8_t)guide_root;ru_render(&s);
 s.settings_open=0u;s.preset_save_stage=3u;s.preset_save_progress=100u;ru_render(&s);
 puts("PASS V2.0.5: ordered PRESETS categories, Scale Guide ROOT, sequencer note/empty layout and save overlay");
 return 0;
}
