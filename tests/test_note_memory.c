#include <assert.h>
#include <stdio.h>
#include "../src/rupture_note_memory.h"
int main(void){
 RuNoteMemory m;ru_note_init(&m);RuNoteDecision d;
 assert(m.count==0&&m.legato==1);
 for(int i=0;i<6;i++){d=ru_note_on(&m,RU_SOURCE_USB,0,(uint8_t)(40+i),(uint8_t)(40+i),110);assert(d.pitch==40+i);}
 assert(m.count==6);d=ru_note_on(&m,RU_SOURCE_USB,0,46,46,110);assert(m.count==6&&d.pitch==46);
 d=ru_note_off(&m,RU_SOURCE_USB,0,46);assert(d.pitch==45);
 d=ru_note_off(&m,RU_SOURCE_USB,0,45);assert(d.pitch==44);
 d=ru_note_off(&m,RU_SOURCE_USB,0,44);assert(d.pitch==43);
 d=ru_note_off(&m,RU_SOURCE_USB,0,43);assert(d.pitch==42);
 d=ru_note_off(&m,RU_SOURCE_USB,0,42);assert(d.pitch==41);
 d=ru_note_off(&m,RU_SOURCE_USB,0,40);assert(d.pitch==41);/* evicted 40 */
 d=ru_note_off(&m,RU_SOURCE_USB,0,41);assert(!d.active&&m.count==0);
 /* FIFO matching repeated pitches and two independent transports. */
 d=ru_note_on(&m,RU_SOURCE_USB,0,60,60,44);assert(d.retrigger);
 d=ru_note_on(&m,RU_SOURCE_USB,0,60,60,99);assert(d.retrigger&&d.velocity==99);
 d=ru_note_on(&m,RU_SOURCE_TRS,0,60,60,127);assert(m.count==3&&d.source==RU_SOURCE_TRS);
 d=ru_note_off(&m,RU_SOURCE_USB,0,60);assert(m.count==2&&m.notes[0].velocity==99);
 d=ru_note_off(&m,RU_SOURCE_TRS,0,60);assert(m.count==1&&d.source==RU_SOURCE_USB);
 d=ru_note_off(&m,RU_SOURCE_USB,0,60);assert(!d.active);
 /* Sustain: released notes remain audible until pedal-up. */
 ru_note_on(&m,RU_SOURCE_USB,1,52,52,90);
 ru_note_sustain(&m,RU_SOURCE_USB,1,1);
 d=ru_note_off(&m,RU_SOURCE_USB,1,52);assert(d.active&&m.count==1);
 d=ru_note_off(&m,RU_SOURCE_USB,1,52);assert(m.count==1);/* no double release */
 d=ru_note_sustain(&m,RU_SOURCE_USB,1,0);assert(!d.active&&m.count==0);
 /* High/low priority; identical pitch resolves most-recent. */
 m.priority=RU_PRIORITY_HIGH;
 ru_note_on(&m,RU_SOURCE_PANEL,0,66,66,120);
 d=ru_note_on(&m,RU_SOURCE_PANEL,0,62,62,120);assert(d.pitch==66);
 m.priority=RU_PRIORITY_LOW;assert(ru_note_current(&m).pitch==62);
 ru_note_panic(&m,RU_SOURCE_PANEL,0,1);assert(!m.count);
 /* Glide and envelope retrigger flags distinguish initial from legato switch. */
 m.priority=RU_PRIORITY_LAST;m.glide_samples=256;
 d=ru_note_on(&m,RU_SOURCE_USB,0,40,40,127);assert(d.retrigger&&!d.glide);
 d=ru_note_on(&m,RU_SOURCE_USB,0,43,43,127);assert(!d.retrigger&&d.glide);
 d=ru_note_off(&m,RU_SOURCE_USB,0,43);assert(!d.retrigger&&d.glide&&d.pitch==40);
 m.legato=0;d=ru_note_on(&m,RU_SOURCE_USB,0,48,48,127);assert(d.retrigger);
 ru_note_panic(&m,RU_SOURCE_USB,0,1);
 /* Nonexistent IDs never release another note; corrupt transport rejected. */
 ru_note_on(&m,RU_SOURCE_USB,0,52,48,100);
 d=ru_note_off(&m,RU_SOURCE_TRS,0,52);assert(d.active&&d.pitch==48);
 d=ru_note_off(&m,RU_SOURCE_USB,0,52);assert(!d.active);
 d=ru_note_on(&m,19,0,52,52,120);assert(!d.active);
 puts("PASS RUPTURE six-note memory (overflow, return, duplicates, sustain, port identity, priority, glide, panic)");
 return 0;
}
