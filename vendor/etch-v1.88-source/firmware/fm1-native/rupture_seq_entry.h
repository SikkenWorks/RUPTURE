/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE UI note editing, including step zero and dependent HOLD cleanup.
 * Not called from an audio interrupt; does not modify the native pattern codec.
 */
#ifndef RUPTURE_SEQ_ENTRY_H
#define RUPTURE_SEQ_ENTRY_H
#include <stdint.h>
#include "native_performance.h"

/* Delete one trigger and precisely its directed HOLD dependents.  Unbound
 * legacy HOLD events (hold_ago=0) and other same-pitch triggers survive.
 * This is intentionally different from the historical low-level
 * etch_seq_note_remove(), which refuses deletion if any HOLD references it.
 * The RUPTURE step editor offers an explicit delete intent, so leaving a
 * permanent unremovable note in step 1 is worse than clearing its dependents.
 */
static inline uint8_t ru_seq_note_delete(struct etch_native_pattern *p,
                                        uint16_t step,uint8_t pitch){
 if(!p||step>=ETCH_SEQ_STEPS||pitch>127)return 0u;
 struct etch_seq_step *src=&p->steps[step];
 if(src->count>ETCH_SEQ_NOTES)return 0u;
 uint8_t index=0u;
 while(index<src->count && src->notes[index].note!=pitch)index++;
 if(index==src->count)return 0u;
 /* Validate all HOLD counters before any mutation, ensuring a corrupted
  * project cannot cause a partial unlink/delete transaction. */
 for(uint16_t st=0u;st<ETCH_SEQ_STEPS;st++)
  if(p->steps[st].hold_count>ETCH_SEQ_NOTES)return 0u;
 for(uint16_t st=0u;st<ETCH_SEQ_STEPS;st++){
  struct etch_seq_step *s=&p->steps[st];
  uint8_t keep=0u;
  for(uint8_t j=0u;j<s->hold_count;j++){
   const uint8_t ago=s->hold_ago[j];
   if(s->holds[j]==pitch && ago &&
       (uint8_t)(st-ago)==(uint8_t)step)continue;
   if(keep!=j){s->holds[keep]=s->holds[j];s->hold_ago[keep]=s->hold_ago[j];}
   keep++;
  }
  const uint8_t survivors=keep;
  while(keep<s->hold_count){s->holds[keep]=0u;s->hold_ago[keep]=0u;keep++;}
  s->hold_count=survivors;
 }
 /* Removing the trigger cannot fail now that all directed dependents were
  * removed.  Keep the remaining notes and all parameter locks. */
 for(uint8_t j=index;j+1u<src->count;j++)src->notes[j]=src->notes[j+1u];
 --src->count;src->notes[src->count]=(struct etch_seq_note){0,0,0};
 return 1u;
}
/* Native toggle semantics: 1 added, 2 removed, 0 rejected. */
static inline uint8_t ru_seq_key_toggle(struct etch_native_pattern *p,
                                       uint16_t step,uint8_t pitch,uint8_t velocity){
 if(!p||step>=ETCH_SEQ_STEPS||pitch>127||!velocity)return 0u;
 const struct etch_seq_step *s=&p->steps[step];
 if(s->count>ETCH_SEQ_NOTES)return 0u;
 for(uint8_t j=0u;j<s->count;j++)if(s->notes[j].note==pitch)
  return ru_seq_note_delete(p,step,pitch)?2u:0u;
 return etch_seq_put(p,step,pitch,velocity,1u);
}
/* SEL in the stopped sequencer removes the displayed first note on the
 * selected step, without deleting unrelated notes or automation locks. */
static inline uint8_t ru_seq_delete_selected(struct etch_native_pattern *p,
                                             uint16_t step){
 if(!p||step>=ETCH_SEQ_STEPS||p->steps[step].count==0u||
    p->steps[step].count>ETCH_SEQ_NOTES)return 0u;
 return ru_seq_note_delete(p,step,p->steps[step].notes[0].note);
}
/* Clear the ENTIRE selected step, including note triggers, on-step HOLDs
 * and parameter locks. Remove only subsequent directed HOLD references to
 * triggers that originated on this step; other steps/legacy HOLDs survive.
 * Validate first, so malformed patterns never leave a partially cleared step.
 * Main-thread UI only, with the existing short IRQ-off ownership guard. */
static inline uint8_t ru_seq_clear_step(struct etch_native_pattern *p,uint16_t step){
 if(!p||step>=ETCH_SEQ_STEPS)return 0u;
 struct etch_seq_step *src=&p->steps[step];
 if(src->count>ETCH_SEQ_NOTES||src->hold_count>ETCH_SEQ_NOTES)return 0u;
 for(uint16_t at=0u;at<ETCH_SEQ_STEPS;at++){
  const struct etch_seq_step *s=&p->steps[at];
  if(s->hold_count>ETCH_SEQ_NOTES||s->count>ETCH_SEQ_NOTES)return 0u;
 }
 uint8_t present=(uint8_t)(src->count||src->hold_count||src->lock_mask);
 if(!present)for(uint8_t l=0u;l<ETCH_SEQ_LOCKS;l++)if(src->locks[l]){present=1u;break;}
 for(uint16_t at=0u;at<ETCH_SEQ_STEPS;at++){
  struct etch_seq_step *s=&p->steps[at];
  uint8_t keep=0u,old=s->hold_count;
  for(uint8_t j=0u;j<old;j++){
   uint8_t remove=0u,ago=s->hold_ago[j],pitch=s->holds[j];
   if(ago && (uint8_t)(at-ago)==(uint8_t)step){
    for(uint8_t k=0u;k<src->count;k++)
     if(src->notes[k].note==pitch){remove=1u;break;}
   }
   if(!remove){if(keep!=j){s->holds[keep]=pitch;s->hold_ago[keep]=ago;}++keep;}
   else present=1u;
  }
  for(uint8_t j=keep;j<old;j++){s->holds[j]=0u;s->hold_ago[j]=0u;}
  s->hold_count=keep;
 }
 *src=(struct etch_seq_step){0};
 return present;
}
#endif
