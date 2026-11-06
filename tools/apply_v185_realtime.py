#!/usr/bin/env python3
"""V1.85: bounded burst MIDI, idle MIDI scan fast path, read-only DMA profiling."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=r/'vendor/etch-v1.88-source/firmware/fm1-native'
def change(path,needle,replacement):
 p=s/path; data=p.read_text();count=data.count(needle)
 if count!=1:raise RuntimeError(f'{path} expected one match, found {count} for {needle[:75]!r}')
 p.write_text(data.replace(needle,replacement))
(s/'native_midi_rt_budget.h').write_text('''/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.85: deterministic USB-MIDI work budget per 128-frame block.
 * USB MIDI events are NEVER dropped by this budget. A remaining event stays
 * queued for the next DMA half; original overflow/panic handling remains.
 * 32 events per block covers extreme 12k-event/s bursts at 48kHz.
 */
#pragma once
#include <stdint.h>
#define RUPTURE_MIDI_EVENTS_PER_HALF 32u
static inline uint32_t ru_midi_batch_size(uint32_t pending){
 return pending<RUPTURE_MIDI_EVENTS_PER_HALF?pending:RUPTURE_MIDI_EVENTS_PER_HALF;
}
static inline uint8_t ru_midi_scope_skip(uint32_t remaining){
 return (uint8_t)(remaining>0u);
}
''')
change('native_panel_midi.h',
 'struct etch_panel_midi { uint8_t active[27], note[27]; };',
 '''struct etch_panel_midi {
 uint8_t active[27],note[27];
 /* Audio ISR idle fast path: bypass 54 unchanged-key scans per block.
  * retry_pending MUST remain set on a full USB TX ring. */
 uint32_t cached_keys;
 int8_t cached_octave;
 uint8_t cached_connected,cached_valid,retry_pending;
};''')
change('native_panel_midi.c',
 'for(uint8_t k=0;k<27;k++){m->active[k]=0;m->note[k]=0;}\n}',
 '''for(uint8_t k=0;k<27;k++){m->active[k]=0;m->note[k]=0;}
 m->cached_keys=0u;m->cached_octave=0;
 m->cached_connected=0u;m->cached_valid=0u;m->retry_pending=0u;
}''')
change('native_panel_midi.c',
 ''' if(!connected){etch_panel_midi_reset(m);return;}
 if(!emit)return;
 keys &= 0x7ffffffu;''',
 ''' /* A disconnected panel never has pending USB notes. Avoid repeating a
  * 54-entry clear at each DMA half once the disconnect is acknowledged. */
 if(!connected){
  if(!m->cached_valid || m->cached_connected){
   etch_panel_midi_reset(m);m->cached_valid=1u;
  }
  return;
 }
 if(!emit)return;
 keys &= 0x7ffffffu;
 if(m->cached_valid && m->cached_connected && !m->retry_pending &&
    m->cached_keys==keys && m->cached_octave==octave)return;
 m->cached_valid=1u;m->cached_connected=1u;
 m->cached_keys=keys;m->cached_octave=octave;m->retry_pending=0u;''')
change('native_panel_midi.c',
 'if(emit(packet,context))m->active[k]=0;',
 'if(emit(packet,context))m->active[k]=0;else m->retry_pending=1u;')
change('native_panel_midi.c',
 'if(emit(packet,context)){m->active[k]=1;m->note[k]=note;}',
 'if(emit(packet,context)){m->active[k]=1;m->note[k]=note;}else m->retry_pending=1u;')
change('audio_target.c',
 '#include "native_performance_midi.h"',
 '#include "native_performance_midi.h"\n#include "native_midi_rt_budget.h"')
change('audio_target.c',
 'static volatile uint32_t native_min_dma_slack,native_scope_deferred;',
 '''static volatile uint32_t native_min_dma_slack,native_scope_deferred;
/* V1.85: classify real DMA pressure without serial/logging/allocations
 * in the interrupt. Historical peaks reset at audio restart. */
static volatile uint32_t native_controls_wall_peak,native_last_dma_slack;
static volatile uint32_t native_midi_queue_peak,native_midi_deferred_batches;
static volatile uint32_t native_postpublish_late;
static volatile uint8_t native_midi_batch_peak,native_midi_backlog;''')
change('audio_target.c',
 ''' uint32_t end=mi_w;RING_PUBLISH();
 if(midi_in_overflow){etch_native_midi_panic(&native_audio);mi_r=end;RING_PUBLISH();midi_in_overflow=0;return;}
 for(uint32_t count=0;mi_r!=end&&count<MQ;count++){''',
 ''' uint32_t end=mi_w;RING_PUBLISH();
 if(midi_in_overflow){etch_native_midi_panic(&native_audio);mi_r=end;RING_PUBLISH();midi_in_overflow=0;native_midi_backlog=0u;return;}
 uint32_t pending=end-mi_r;
 if(pending>native_midi_queue_peak)native_midi_queue_peak=pending;
 uint32_t limit=ru_midi_batch_size(pending);
 if(limit>native_midi_batch_peak)native_midi_batch_peak=(uint8_t)limit;
 for(uint32_t count=0;count<limit;count++){''')
change('audio_target.c',
 '''   etch_native_midi(&native_audio,status,note,velocity);
  }
 }
}
#endif''',
 '''   etch_native_midi(&native_audio,status,note,velocity);
  }
 }
 /* Original ring and overflow ownership remain intact: only defer heavy
  * USB bursts, never remove events or re-order Note OFF/ON. */
 native_midi_backlog=ru_midi_scope_skip(end-mi_r);
 if(native_midi_backlog&&native_midi_deferred_batches<UINT32_MAX)
  native_midi_deferred_batches++;
}
#endif''')
change('audio_target.c',
 '''  uint32_t render_start=fm1_ticks();
  if(!native_output_enabled''',
 '''  uint32_t render_start=fm1_ticks();
  uint32_t ctrl_elapsed=(uint32_t)(render_start-start);
  if(ctrl_elapsed>native_controls_wall_peak)native_controls_wall_peak=ctrl_elapsed;
  if(!native_output_enabled''')
change('audio_target.c',
 '''  if(etch_audio_scope_may_capture((uint32_t)(fm1_ticks()-start),
        ETCH_NATIVE_FRAMES*544u,native_ui_pressure)){''',
 '''  if(!native_midi_backlog &&
     etch_audio_scope_may_capture((uint32_t)(fm1_ticks()-start),
        ETCH_NATIVE_FRAMES*544u,native_ui_pressure)){''')
change('audio_target.c',
 '''  if(slack<native_min_dma_slack)native_min_dma_slack=slack;''',
 '''  native_last_dma_slack=slack;
  if(slack<native_min_dma_slack)native_min_dma_slack=slack;''')
change('audio_target.c',
 '''  if(measure.late){
   ++native_late;''',
 '''  if(measure.late){
   ++native_postpublish_late;
   ++native_late;''')
change('audio_target.c',
 ''' native_render_wall_peak=native_publish_wall_peak=native_scope_deferred=0u;
 native_min_dma_slack=ETCH_NATIVE_FRAMES*544u;''',
 ''' native_render_wall_peak=native_publish_wall_peak=native_scope_deferred=0u;
 native_controls_wall_peak=native_midi_queue_peak=native_midi_deferred_batches=0u;
 native_postpublish_late=0u;native_midi_batch_peak=0u;native_midi_backlog=0u;
 native_min_dma_slack=native_last_dma_slack=ETCH_NATIVE_FRAMES*544u;''')
change('rupture_ui.c',
 ''' uint32_t diag_late,diag_guard,diag_scope;uint16_t ota_error;} RuState;''',
 ''' uint32_t diag_late,diag_guard,diag_scope;uint16_t ota_error;
 /* V1.85: second page of the existing read-only DMA diagnostic. */
 uint8_t diag_view;uint8_t diag_midi_batch;
 uint16_t diag_control,diag_nested,diag_live_slack;
 uint32_t diag_midi_queue,diag_midi_deferred,diag_postlate;
} RuState;''')
change('rupture_ui.c',
 ''' if(page==RU_PAGE_DIAG){
  const char *names[]={"WALL MAX","DSP MAX","RENDER","PUBLISH","MIN SLACK","LATE","GUARD","SCOPE SKIP"};''',
 ''' if(page==RU_PAGE_DIAG){
  if(v->diag_view){
   const char *names[]={"CONTROL MAX","NESTED MAX","SLACK LAST","MIDI BATCH","MIDI QUEUE","MIDI DEFER","GUARD","POST LATE"};
   uint32_t vals[]={v->diag_control,v->diag_nested,v->diag_live_slack,
    v->diag_midi_batch,v->diag_midi_queue,v->diag_midi_deferred,
    v->diag_guard,v->diag_postlate};
   ru_center("READ ONLY / MIDI + TIMING",120,53,1);
   for(int i=0;i<8;i++){
    int y=77+i*17;char value[12];ru_uint(vals[i],value);
    ru_text(names[i],10,y,1);ru_text(value,151,y,1);
    if(i<3)ru_text("%",211,y,1);
   }
   ru_footer("SEL MAIN / HOME BACK");return;
  }
  const char *names[]={"WALL MAX","DSP MAX","RENDER","PUBLISH","MIN SLACK","LATE","GUARD","SCOPE SKIP"};''')
change('rupture_ui.c',
 '''  ru_footer("HOME BACK / LATE AND GUARD 0");return;''',
 '''  ru_footer("SEL DETAILS / HOME BACK");return;''')
change('bringup.c',
 '''static uint8_t ru_project_slot=1,ru_reverb_type=1,ru_help_chapter=0,ru_help_scroll=0,ru_selected;''',
 '''static uint8_t ru_project_slot=1,ru_reverb_type=1,ru_help_chapter=0,ru_help_scroll=0,ru_selected;
static uint8_t ru_diag_view; /* read-only main/detail page, never stored */''')
change('bringup.c',
 ''' case RU_PAGE_DIAG:
  {uint32_t budget=ETCH_NATIVE_FRAMES*544u;''',
 ''' case RU_PAGE_DIAG:
  {uint32_t budget=ETCH_NATIVE_FRAMES*544u;
   v.diag_view=ru_diag_view;
   v.diag_control=(uint16_t)(etch_diag_cpu_permille(native_controls_wall_peak,budget)/10u);
   v.diag_nested=(uint16_t)(etch_diag_cpu_permille(native_max_nested_ticks,budget)/10u);
   v.diag_live_slack=(uint16_t)(etch_diag_cpu_permille(native_last_dma_slack,budget)/10u);
   v.diag_midi_batch=native_midi_batch_peak;
   v.diag_midi_queue=native_midi_queue_peak;
   v.diag_midi_deferred=native_midi_deferred_batches;
   v.diag_postlate=native_postpublish_late;''')
change('bringup.c',
 '''   ru_nav_to(ru_selected==0u?RU_PAGE_SETTINGS:ru_selected==1u?RU_PAGE_SYS_INFO:
             ru_selected==2u?RU_PAGE_HELP:ru_selected==3u?RU_PAGE_DIAG:
             ru_selected==4u?RU_PAGE_ABOUT:RU_PAGE_BUG_REPORT);
   ru_settings_open=0u;''',
 '''   ru_nav_to(ru_selected==0u?RU_PAGE_SETTINGS:ru_selected==1u?RU_PAGE_SYS_INFO:
             ru_selected==2u?RU_PAGE_HELP:ru_selected==3u?RU_PAGE_DIAG:
             ru_selected==4u?RU_PAGE_ABOUT:RU_PAGE_BUG_REPORT);
   if(ru_page==RU_PAGE_DIAG)ru_diag_view=0u;
   ru_settings_open=0u;''')
change('bringup.c',
 ''' else if(ru_page==RU_PAGE_SYS_INFO){
  /* Explicit drill-down; all figures are read-only. */
  if(pressed&(1u<<3))ru_nav_to(RU_PAGE_DIAG);
 }
 else if(ru_page==RU_PAGE_SETTINGS){''',
 ''' else if(ru_page==RU_PAGE_SYS_INFO){
  /* Explicit drill-down; all figures are read-only. */
  if(pressed&(1u<<3)){ru_diag_view=0u;ru_nav_to(RU_PAGE_DIAG);}
 }
 else if(ru_page==RU_PAGE_DIAG){
  /* SEL switches two purely diagnostic displays. No stored settings and
   * no further ISR work; all values are existing peak/snapshot counters. */
  if(pressed&(1u<<3)){ru_diag_view^=1u;first_frame=1u;}
 }
 else if(ru_page==RU_PAGE_SETTINGS){''')
change('build.py',"'version':'RUPTURE V1.84.2'","'version':'RUPTURE V1.85'")
change('rupture_ui.c', 'RUPTURE V1.84.2', 'RUPTURE V1.85') if 'RUPTURE V1.84.2' in (s/'rupture_ui.c').read_text() else None
print('Patched V1.85 realtime MIDI and diagnostics')
