/* SPDX-License-Identifier: GPL-3.0-only */
#include "rupture_note_memory.h"
static uint8_t valid(uint8_t s,uint8_t c){return s<RU_SOURCE_COUNT&&c<16u;}
static uint16_t ident(uint8_t ch,uint8_t note){return (uint16_t)((uint16_t)ch*128u+note);}
static uint8_t same(const RuNote *n,uint8_t s,uint8_t c,uint8_t pitch){
 return n->source==s&&n->channel==c&&n->identity==ident(c,pitch);
}
static uint8_t winner(const RuNoteMemory *m){
 uint8_t found=0;
 for(uint8_t i=1;i<m->count;i++){
  if((m->priority==RU_PRIORITY_HIGH&&m->notes[i].pitch>=m->notes[found].pitch)||
     (m->priority==RU_PRIORITY_LOW&&m->notes[i].pitch<=m->notes[found].pitch)||
     m->priority==RU_PRIORITY_LAST)found=i;
 }
 return found;
}
RuNoteDecision ru_note_current(const RuNoteMemory *m){
 RuNoteDecision d={0};if(!m||!m->count)return d;
 const RuNote *n=&m->notes[winner(m)];
 d.active=1;d.pitch=n->pitch;d.velocity=n->velocity;d.source=n->source;d.channel=n->channel;
 return d;
}
void ru_note_init(RuNoteMemory *m){
 if(!m)return;
 uint8_t *p=(uint8_t *)m;for(unsigned i=0;i<sizeof(*m);i++)p[i]=0;
 m->legato=1u;
}
static void erase(RuNoteMemory *m,uint8_t i){
 for(uint8_t j=(uint8_t)(i+1);j<m->count;j++)m->notes[j-1]=m->notes[j];
 m->count--;
}
static RuNoteDecision transition(RuNoteMemory *m,RuNoteDecision prev,uint8_t press){
 RuNoteDecision next=ru_note_current(m);
 if(!next.active)return next;
 if(!prev.active){next.retrigger=1u;return next;}
 if(!m->legato && (press||next.pitch!=prev.pitch||next.source!=prev.source||next.channel!=prev.channel))next.retrigger=1u;
 if(m->legato&&next.pitch!=prev.pitch&&m->glide_samples)next.glide=1u;
 return next;
}
RuNoteDecision ru_note_on(RuNoteMemory *m,uint8_t src,uint8_t channel,uint8_t note,uint8_t tuned,uint8_t velocity){
 RuNoteDecision none={0};if(!m||!valid(src,channel)||note>127u||tuned>127u||!velocity)return none;
 RuNoteDecision before=ru_note_current(m);
 if(m->count==RUPTURE_NOTE_LIMIT)erase(m,0); /* oldest, never a random active voice */
 RuNote *n=&m->notes[m->count++];
 n->identity=ident(channel,note);n->pitch=tuned;n->velocity=velocity;
 n->source=src;n->channel=channel;n->down=1u;
 RuNoteDecision result=transition(m,before,1u);
 /* Replaying the same pitch is a genuine fresh attack, not a skipped legato
  * transition, but only if the newly pressed note actually wins priority. */
 if(m->legato&&before.active&&result.active&&result.pitch==before.pitch&&
    winner(m)==m->count-1u)result.retrigger=1u;
 return result;
}
RuNoteDecision ru_note_off(RuNoteMemory *m,uint8_t src,uint8_t channel,uint8_t note){
 RuNoteDecision none={0};if(!m||!valid(src,channel)||note>127u)return none;
 RuNoteDecision before=ru_note_current(m);
 for(uint8_t i=0;i<m->count;i++){
  RuNote *n=&m->notes[i];
  if(n->down&&same(n,src,channel,note)){
   if(m->sustain[src]&(1u<<channel))n->down=0u;
   else erase(m,i);
   break;
  }
 }
 return transition(m,before,0u);
}
RuNoteDecision ru_note_sustain(RuNoteMemory *m,uint8_t src,uint8_t channel,uint8_t enabled){
 RuNoteDecision none={0};if(!m||!valid(src,channel))return none;
 RuNoteDecision before=ru_note_current(m);
 if(enabled)m->sustain[src]|=(uint16_t)(1u<<channel);
 else{
  m->sustain[src]&=(uint16_t)~(1u<<channel);
  for(uint8_t i=0;i<m->count;)if(m->notes[i].source==src&&m->notes[i].channel==channel&&!m->notes[i].down)erase(m,i);else i++;
 }
 return transition(m,before,0u);
}
RuNoteDecision ru_note_panic(RuNoteMemory *m,uint8_t src,uint8_t channel,uint8_t all_channels){
 RuNoteDecision none={0};if(!m||src>=RU_SOURCE_COUNT||(!all_channels&&channel>=16u))return none;
 RuNoteDecision before=ru_note_current(m);
 for(uint8_t i=0;i<m->count;)if(m->notes[i].source==src&&(all_channels||m->notes[i].channel==channel))erase(m,i);else i++;
 if(all_channels)m->sustain[src]=0u;
 else m->sustain[src]&=(uint16_t)~(1u<<channel);
 return transition(m,before,0u);
}
