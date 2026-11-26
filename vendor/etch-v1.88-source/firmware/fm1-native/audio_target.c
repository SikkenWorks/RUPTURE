/* SPDX-License-Identifier: GPL-3.0-only
 * Included in bringup.c after the HAL. Main-loop controls are published in
 * a short IRQ-off critical section. Audio state belongs to ALNK0 only.
 */
#include "audio_core.h"
#include "native_panel_midi.h"
#include "native_audio_timing.h"
#include "native_output_startup.h"
#include "native_performance_midi.h"
#include "native_midi_rt_budget.h"
#include "native_preset_ramp.h"
#include "native_storage_audio.h"
#ifdef RUPTURE_BUILD
#include "rupture_note_warmup_gate.h"
#include "rupture_note_release_guard.h"
#include "rupture_note_dma_meter.h"
#include "rupture_note_scope_guard.h"
#include "rupture_core2_pressure_guard.h"
#include "rupture_midi_expression.h"
#endif
static struct etch_native_audio native_audio __attribute__((section(".pool")));
#ifdef RUPTURE_BUILD
/* Eight poly-aftertouch tags and 16 channel-pressure values; bounded state
 * only at MIDI event/block boundaries, never inside CXS or CORE2 DSP. */
static struct ru_midi_expression ru_midi_expr;
#endif
/* V1.81: published control snapshot resides in zeroed shared DSP pool.
 * This reclaims main SRAM without changing the ISR ABI or flash layout. */
static volatile struct etch_native_controls native_controls __attribute__((section(".pool")));
static int32_t native_dma[ETCH_NATIVE_FRAMES*4] __attribute__((aligned(4)));
static int16_t native_mono[ETCH_NATIVE_FRAMES] __attribute__((section(".noinit")));
static int16_t native_stereo[ETCH_NATIVE_FRAMES*2] __attribute__((section(".noinit")));
#ifndef ETCH_NATIVE_PANEL_MIDI
#define ETCH_NATIVE_PANEL_MIDI 0
#endif
static struct etch_panel_midi native_panel_out;
static struct etch_perf_midi native_perf_out;
#if ETCH_NATIVE_PANEL_MIDI
static int native_panel_usb_emit(uint32_t packet,void *unused){(void)unused;return midi_out_event(packet);}
#endif
#define ETCH_SCOPE_CAPTURE 1024u
/* The 1024-frame double capture is explicitly cleared by native_start()
 * and published only on completed blocks. On UAC builds it shares the
 * dedicated SRAM window with the USB ring, without touching audio DMA,
 * stack, storage, protected .noinit, or the legacy UI canvas. */
#if ETCH_NATIVE_UAC
static int16_t native_scope[2][ETCH_SCOPE_CAPTURE] __attribute__((section(".uac_ram"),aligned(4)));
#else
static int16_t native_scope[2][ETCH_SCOPE_CAPTURE];
#endif
static volatile uint8_t native_scope_ready;
static uint16_t native_scope_cursor;
static volatile uint8_t native_scope_has_signal;
/* Published from ALNK0 at complete capture boundaries; includes MIDI and
 * sequencer voices, not merely front-panel keys. FX tails do not count. */
static volatile uint8_t native_scope_held_note;
static uint32_t native_scope_window_peak;
static uint8_t native_scope_silent_windows;
static volatile uint32_t native_halves,native_late,native_max_ticks;
static volatile uint8_t native_ui_pressure; /* ISR-owned hysteretic LCD budget hint */
static volatile uint32_t native_max_wall_ticks,native_max_nested_ticks;
/* V1.19: four read-only on-device DMA diagnostics: synth/FX render wall
 * time, PCM+USB publish wall time, minimum slack and deferred scope
 * captures. Only ALNK0 writes them. No tracing, allocation or serial
 * printing in the realtime interrupt. */
static volatile uint32_t native_render_wall_peak,native_publish_wall_peak;
static volatile uint32_t native_min_dma_slack,native_scope_deferred;
/* V1.85: classify real DMA pressure without serial/logging/allocations
 * in the interrupt. Historical peaks reset at audio restart. */
static volatile uint32_t native_controls_wall_peak,native_last_dma_slack;
static volatile uint32_t native_midi_queue_peak,native_midi_deferred_batches;
/* Diagnose intermittent MIDI release faults without printf/heap in ALNK0.
 * Counters remain cumulative across an audio-engine restart. */
#ifdef RUPTURE_BUILD
static volatile uint32_t native_midi_on_seen,native_midi_off_seen;
static volatile uint32_t native_midi_panic_seen;
#endif
static volatile uint32_t native_postpublish_late;
static volatile uint8_t native_midi_batch_peak,native_midi_backlog;

/* DMA pre-publish guard hits: block writes into a buffer that may be active. */
static volatile uint32_t native_dma_blocked;
#ifdef RUPTURE_BUILD
/* Only the ALNK0 interrupt writes these counters; foreground reads 32-bit
 * snapshots. They are intentionally cumulative across audio restarts. */
static volatile ru_note_dma_meter_t native_note_dma;
/* Owned by ALNK0: protects ONLY oscilloscope samples, never PCM or MIDI. */
static ru_note_scope_guard_t native_note_scope_guard;
/* Preset-local peaks permit apples-to-apples DMA diagnosis without touching
 * the oscillator or effect PCM path. The foreground resets these counters
 * inside the same short IRQ-off handoff as the newly loaded parameters. */
static volatile uint32_t native_preset_wall_peak,native_preset_render_peak;
static volatile uint32_t native_preset_min_slack;
static volatile uint32_t native_preset_late_base,native_preset_guard_base;
static void native_preset_profile_reset(void){
 native_preset_wall_peak=0u;native_preset_render_peak=0u;
 native_preset_min_slack=ETCH_NATIVE_FRAMES*544u;
 native_preset_late_base=native_late;
 native_preset_guard_base=native_dma_blocked;
}
#endif
static volatile uint8_t native_missed_streak;
static volatile uint16_t native_healthy_blocks;
static volatile uint32_t native_voice_shed_count;
static volatile uint32_t native_audio_peak,native_audio_clip_frames;
static volatile uint8_t native_tripped,native_panic,native_suspended,native_edit_keys_blocked;
/* Capture one external MIDI note into the Project GROUPS editor. The main
 * thread validates/stores the range. Never write flash or project bytes in ISR. */
static volatile uint8_t native_group_capture_active,native_group_midi_pending,native_group_midi_note;
/* The PC6 analog enable is a binary control and may inherently make a
 * hardware bias pop. V1.03 isolates the first large LCD frame from audible
 * PCM: the analog section stabilizes first, the PLAY UI renders in digital
 * silence, then the ISR executes the sample-by-sample fade. This prevents
 * first-frame DMA contention from occurring with nonzero synth audio.
 * It does not promise an analog pop-free physical power enable. */
static volatile uint8_t native_output_enabled;
/* Foreground owns requests; ALNK owns fade progress. Successful storage
 * pauses retain clock, codec bias and the already-qualified output gate. */
static volatile uint8_t native_audio_clock_live,native_storage_quiet_request;
static volatile struct etch_storage_audio native_storage_audio;
static uint8_t native_storage_read_resume,native_storage_audio_failed;
static void native_start(void);
static void native_pause(void);
static void native_storage_read_poll(uint8_t allowed){
 if(!native_storage_read_resume || !allowed || native_tripped)return;
 native_storage_read_resume=0u;
 fm1_irq_off();native_start();fm1_irq_enable_all();
}
/* USB attach/suspend can change analog ground/power bias. No claim of
 * eliminating the hardware edge; defer it until bus state is stable. */
static struct etch_output_usb_gate native_usb_gate;
static volatile uint8_t native_output_analog_ready;
static volatile uint8_t native_boot_audio_released;
static volatile uint16_t native_output_fade;
/* The main loop publishes a one-byte request alongside patch control changes.
 * Only ALNK0 touches ramp state. Every preset jump crossfades its outgoing
 * PCM smoothly over 256 samples, including sudden FX/filter changes. */
static volatile uint8_t native_preset_ramp_requested;
static struct etch_preset_ramp native_preset_ramp;
static volatile uint16_t native_prepared_blocks;
static struct etch_output_gate native_output_gate;
/* Main-loop-owned: never synchronized through the realtime callback. */
static struct etch_output_screen_gate native_screen_gate;
static void native_output_poll(void){
 /* GLO-held-at-boot diagnostic runs the SAME normal FM1 audio init with
  * PC6 permanently low. The zeroed DMA remains silent, unlike REC SAFE
  * which skips audio_init entirely. No output enable occurs in this mode. */
 if(native_analog_gate_test)return;
 if(native_suspended||native_tripped||native_output_enabled)return;
 uint32_t now=fm1_ticks();
 uint8_t usb_state=0u;
#if ETCH_NATIVE_USB
 usb_state=(uint8_t)((usb.config?1u:0u)|(usb.suspended?2u:0u));
#endif
 /* PC USB enumeration can repeatedly reconfigure/suspend the link.
  * V1.26 had a 250 ms delay with a 3 s fall-through: PC6 could be
  * enabled immediately after a USB change. Hold DIGITAL ZERO and wait
  * for an uninterrupted stable window instead. This gate is NOT a VBUS
  * voltage or an analogue offset detector. */
 uint8_t usb_stable=etch_output_usb_ready(&native_usb_gate,now,usb_state);
 if(native_output_gate.phase==ETCH_OUTPUT_CLOCK_SILENCE &&
    (!usb_stable || !etch_output_led_quiet_ready(native_led_seen,now,
        native_led_last_change_ticks)))return;
 uint8_t action=etch_output_gate_advance(&native_output_gate,fm1_ticks(),native_halves);
 if(action==ETCH_OUTPUT_ENABLE_ANALOG){
  /* The DMA ISR still outputs zeros; no audio reaches the DAC yet. */
  fm1_audio_output_enable(1u);
 }else if(action==ETCH_OUTPUT_ANALOG_READY){
  /* LCD is still showing the splash. Keep the DMA sample stream zero
   * until the first PLAY frame has been completely transferred. */
  native_output_analog_ready=1u;
 }
}

/* Three bank buffers are owned by audio_core; referenced by the UI compiler. */
static struct etch_wave_editor native_editor __attribute__((section(".pool")));
static uint8_t native_draw;
/* UI-only V1.85 brush selection; never read in the audio callback. */
static uint8_t native_draw_brush;
/* SPECTRAL rebuilds are not realtime. Keep the edited points pending until
 * no voices remain; a bank swap transiently doubles oscillator work. */
static uint8_t native_spectral_bank_pending;
/* A RUPTURE preset changes CXS/GROWL DSP parameters, never the played
 * ETCH carrier table. Keep its optional editor/table rebuild pending until
 * the synth is silent. Otherwise the three-bank compiler can compete with
 * a heavy GROWL + dual SUB + DISTORTION note for the DMA deadline. */
#ifdef RUPTURE_BUILD
static uint8_t native_patch_wave_pending;
#endif
static uint8_t native_blend_audio_busy(void);
/* Three already allocated mip banks are reused when GROUPS is enabled:
 * bank 0 = currently edited main sound; bank 1/2 = group preset waves.
 * No new 18-KiB tables and no compilation in the audio callback.
 * The output is muted while changing the bank contents, preventing half-
 * compiled tables from being audible (including any incoming MIDI notes). */
static uint8_t native_groups_compiled;
/* V1.85: cache the source state for the published (non-GROUPS) wavetable.
 * Compare exact 128-point waves and FX before recomputing. ~816 bytes
 * in the existing DSP pool, versus wasting 2/3 of the compilation work
 * every time the user edits one wave. No fourth runtime wavetable. */
static int16_t native_last_compiled_points[3][128] __attribute__((section(".pool")));
static struct etch_wave_fx_params native_last_compiled_fx[3] __attribute__((section(".pool")));
static uint8_t native_last_compiled_valid __attribute__((section(".pool")));
static uint8_t native_wave_matches_published(uint8_t w){
 if(!native_last_compiled_valid)return 0u;
 for(uint32_t n=0u;n<128u;n++)
  if(native_editor.points[w][n]!=native_last_compiled_points[w][n])return 0u;
 const uint16_t *a=(const uint16_t *)&native_editor.fx[w];
 const uint16_t *b=(const uint16_t *)&native_last_compiled_fx[w];
 for(uint32_t n=0u;n<sizeof(native_editor.fx[w])/2u;n++)if(a[n]!=b[n])return 0u;
 return 1u;
}
static void native_wave_publish_cache(void){
 for(uint32_t w=0u;w<3u;w++){
  for(uint32_t n=0u;n<128u;n++)native_last_compiled_points[w][n]=native_editor.points[w][n];
  native_last_compiled_fx[w]=native_editor.fx[w];
 }
 native_last_compiled_valid=1u;
}
static uint8_t native_groups_enabled_now(void);
static void native_compile_group_bank(uint8_t index);
static void native_compile_edit(void){
 uint8_t grouped=native_groups_enabled_now();
 if(!native_editor.dirty && grouped==native_groups_compiled){
  native_spectral_bank_pending=0u;return;
 }
 /* All three banks are needed while grouped. Wait for complete note tails.
  * When leaving group mode, also wait for old Group-owned voices. */
 if(grouped || native_groups_compiled){
  native_last_compiled_valid=0u;
  if(native_blend_audio_busy())return;
  fm1_irq_off();
  native_controls.bank=0;native_audio.bank=0;native_audio.old_bank=0;native_audio.bank_fade=0;
  fm1_irq_enable_all();
  if(grouped){
   etch_editor_compile(&native_editor,&native_banks[0]);fm1_wdt_feed();
   native_compile_group_bank(0);fm1_wdt_feed();
   native_compile_group_bank(1);fm1_wdt_feed();
   fm1_irq_off();native_controls.bank=&native_banks[0];
   native_audio.bank=&native_banks[0];native_audio.old_bank=0;native_audio.bank_fade=0;fm1_irq_enable_all();
   native_groups_compiled=1u;native_editor.dirty=0u;native_spectral_bank_pending=0u;
   return;
  }
  /* Back to the original three-bank crossfade allocator. */
  native_groups_compiled=0u;native_editor.dirty=1u;
 }
 if(!native_editor.dirty){native_spectral_bank_pending=0u;return;}
 if(native_spectral_bank_pending && (!native_mono_mode || native_blend_audio_busy()))return;
 const struct etch_native_bank *current,*old,*published;
 fm1_irq_off();current=native_audio.bank;old=native_audio.old_bank;published=native_controls.bank;fm1_irq_enable_all();
 for(uint32_t i=0;i<3;i++)if(&native_banks[i]!=current&&&native_banks[i]!=old&&&native_banks[i]!=published){
  uint8_t changed[3];uint8_t total=0u;
  for(uint8_t w=0u;w<3u;w++){changed[w]=(uint8_t)!native_wave_matches_published(w);total+=changed[w];}
  /* The published bank is immutable during the compilation and PCM ISR.
   * Copy unchanged 6x512-point tables, compile only changed waves, then
   * atomically publish the completed idle bank after all writes finish. */
  if(total<3u && published && native_last_compiled_valid){
   for(uint8_t w=0u;w<3u;w++){
    if(changed[w])etch_editor_compile_wave_source(native_editor.points,native_editor.fx,&native_banks[i],w);
    else for(uint32_t l=0u;l<6u;l++)for(uint32_t n=0u;n<512u;n++)
     native_banks[i].table[w][l][n]=published->table[w][l][n];
   }
  }else etch_editor_compile(&native_editor,&native_banks[i]);
  native_wave_publish_cache();
  fm1_irq_off();native_controls.bank=&native_banks[i];fm1_irq_enable_all();
  native_editor.dirty=0;native_spectral_bank_pending=0u;break;
 }
}
#if ETCH_NATIVE_USB
static void native_midi_drain(uint8_t play){
 uint32_t end=mi_w;RING_PUBLISH();
 if(midi_in_overflow){
#ifdef RUPTURE_BUILD
  if(native_midi_panic_seen<UINT32_MAX)++native_midi_panic_seen;
  ru_midi_expr_reset(&ru_midi_expr);
#endif
  etch_native_midi_panic(&native_audio);mi_r=end;RING_PUBLISH();midi_in_overflow=0;native_midi_backlog=0u;return;
 }
 uint32_t pending=end-mi_r;
 if(pending>native_midi_queue_peak)native_midi_queue_peak=pending;
 uint32_t limit=ru_midi_batch_size(pending);
 if(limit>native_midi_batch_peak)native_midi_batch_peak=(uint8_t)limit;
 for(uint32_t count=0;count<limit;count++){
  uint32_t pkt=midi_in_q[mi_r%MQ];RING_PUBLISH();++mi_r;
  if(play){
   uint8_t status=(uint8_t)(pkt>>8),note=(uint8_t)(pkt>>16),velocity=(uint8_t)(pkt>>24);
#ifdef RUPTURE_BUILD
   if((status&0xf0u)==0x80u || ((status&0xf0u)==0x90u&&!velocity)){
    if(native_midi_off_seen<UINT32_MAX)++native_midi_off_seen;
   }else if((status&0xf0u)==0x90u&&velocity){
    if(native_midi_on_seen<UINT32_MAX)++native_midi_on_seen;
   }
#endif
   if(ru_midi_capture_attack(native_group_capture_active,status,velocity)&&note<128u){
    if(!native_group_midi_pending){native_group_midi_note=note;native_group_midi_pending=1u;}
    continue;
   }
   /* V2.0.43: do NOT swallow MIDI NOTE OFF during group-key capture.
    * Previously this silently discarded the release of a note that was
    * already sounding when capture began, leaving a latched mono voice.
    * A capture-only key attack was never voiced, so its later OFF is safe. */
   etch_native_midi(&native_audio,status,note,velocity);
#ifdef RUPTURE_BUILD
   ru_midi_expr_event(&ru_midi_expr,status,note,velocity);
#endif
  }
 }
 /* Original ring and overflow ownership remain intact: only defer heavy
  * USB bursts, never remove events or re-order Note OFF/ON. */
 native_midi_backlog=ru_midi_scope_skip(end-mi_r);
 if(native_midi_backlog&&native_midi_deferred_batches<UINT32_MAX)
  native_midi_deferred_batches++;
}
#endif
extern void isr_alnk0(void);
static void native_stop(void){native_audio_clock_live=0u;native_storage_read_resume=0u;native_storage_quiet_request=0u;native_suspended=1;native_output_enabled=0u;fm1_audio_output_enable(0u);fm1_audio_stop();native_tripped=1;}
static void native_budget_late(uint32_t core2_jobs_before){
 native_healthy_blocks=0;
#ifdef RUPTURE_BUILD
 /* RUPTURE is hard mono (render_internal limit==1), so reducing the nominal
  * 8-voice budget cannot save a SINGLE DSP cycle. Immediately suspend
  * optional LCD/scope work instead. NEVER weaken the true DMA ownership
  * check, skip an audio block, or mask a persistent underrun. */
 native_ui_pressure=32u;
#if ETCH_CPU1_FX_LIVE_LAB
 /* An actual CORE2 FX lane in this missed DMA block is evidence that the
  * experimental intercore route did not meet the realtime deadline.
  * All FX lanes have completed or retired before reaching this callback.
  * Switch to the bit-exact CPU0 path for subsequent blocks instead of
  * repeatedly risking an audio-engine stop on this same CORE2 workload. */
 if(ru_core2_pressure_should_retire(ru_c1_online,
      ru_fx_probe_finished(ru_c1_good,ru_c1_fir_good),
      core2_jobs_before,ru_c1_fx_commits)){
  ru_cpu1_retire(RU_C1_PRESSURE);
 }
#else
 (void)core2_jobs_before;
#endif
 if(native_missed_streak<255u)++native_missed_streak;
 if(native_missed_streak>=8u){
  etch_boot_audio_fault(&etch_boot);native_stop();
 }
#else
 if(native_audio.voice_limit>2u){
  native_audio.voice_limit=etch_voice_budget_reduce(native_audio.voice_limit);
  ++native_voice_shed_count;
  native_missed_streak=0;
 }else if(++native_missed_streak>=8u){
  etch_boot_audio_fault(&etch_boot);native_stop();
 }
#endif
}
static void native_budget_ok(void){
 native_missed_streak=0;
 if(++native_healthy_blocks>=768u){
  native_healthy_blocks=0;
  if(native_audio.voice_limit<8u)
   native_audio.voice_limit=etch_voice_budget_restore(native_audio.voice_limit);
 }
}
/* Main-only: leave the last faded half before borrowing SFC/XIP.
 * ISR returns only fully owned zero buffers. With both halves zero, DMA
 * may loop safely for arbitrarily long IRQ-masked NOR erase/programs. */
static void native_pause(void){
 if(native_suspended)return; /* nested commit/load/rollback must be inert */
 if(native_audio_clock_live && !native_tripped){
  fm1_irq_off();
  etch_storage_audio_begin(&native_storage_audio);
  native_storage_quiet_request=1u;
  fm1_irq_enable_all();
  const uint32_t began=fm1_ticks();
  while(!etch_storage_audio_ready(&native_storage_audio)){
   fm1_wdt_feed();
   if(native_tripped ||
      (uint32_t)(fm1_ticks()-began)>=ETCH_STORAGE_QUIET_TIMEOUT_TICKS){
    /* A broken DMA interrupt must never hang SAVE or permit unsafe XIP.
     * Emergency analogue disable is reserved for this actual fault. */
    fm1_irq_off();native_stop();fm1_irq_enable_all();
    native_storage_audio_failed=1u;return;
   }
  }
 }
 fm1_irq_off();
 native_suspended=1u;native_output_enabled=0u;
 native_storage_quiet_request=0u;fm1_audio_ack_half();
 fm1_irq_enable_all();
#if ETCH_NATIVE_PANEL_MIDI
 etch_perf_midi_flush(&native_perf_out,native_panel_usb_emit,0);
#endif
}
/* Covers catalog/pref reads whose caller did not explicitly pause. Resume
 * once in the normal main loop, never between tiny journal reads. Boot,
 * MIDI-only, explicit mute, faults and OTA cannot be auto-unmuted here. */
static int native_storage_audio_prepare(void){
 if(native_storage_audio_failed)return -1;
 if(native_audio_clock_live && !native_suspended && !native_tripped){
  native_pause();
  if(native_storage_audio_failed)return -1;
  native_storage_read_resume=1u;
 }
 return 0;
}
void fm1_alnk0_irq(void){
 /* Mark the entire ALNK0 ownership window, not just etch_native_render.
  * TIMER5 nests: only EP4 UAC streaming may be serviced in that window. */
 /* Start a fresh TIMER5 accounting window before allowing nested IRQs.
  * Matches Felucca: wall time controls DMA deadline, nested time is
  * deducted only from the DSP load meter. */
 native_nested_ticks=0u;
 native_in_audio=1u;
 uint8_t p=fm1_audio_pending();fm1_audio_ack_aux(p);
 if(p&FM1_AUDIO_HALF){
  if(native_suspended){fm1_audio_ack_half();native_in_audio=0u;return;}
  uint32_t half=fm1_audio_free_half(),start=fm1_ticks();
  /* Compare completed jobs only within THIS block, including MIDI, USB,
   * render and publish. Earlier FX activity cannot trip CORE2 on an
   * unrelated LCD/USB-only deadline miss. */
#if ETCH_CPU1_FX_LIVE_LAB
  const uint32_t core2_jobs_before=ru_c1_fx_commits;
#else
  const uint32_t core2_jobs_before=0u;
#endif
  struct etch_native_controls c;
  c=native_controls;
#ifdef RUPTURE_BUILD
  /* Per-voice expression changes ONLY the immutable ALNK block snapshot.
   * Neither CXS oscillator math nor the two CPU cores' FX code changes.
   * OFF returns before touching controls; the realtime path owns no RNG. */
  const struct ru_touch_profile touch_profile=ru_touch_profiles[c.main_wave?1u:0u];
#endif
  uint32_t keys=native_edit_keys_blocked?0:fm1_in.notes;
  if(native_panic){native_panic=0;
#ifdef RUPTURE_BUILD
   ru_midi_expr_reset(&ru_midi_expr);
#endif
#if ETCH_NATIVE_USB
mi_r=mi_w;RING_PUBLISH();
#endif
etch_native_reset(&native_audio);native_audio.keys=keys&0x7ffffffu;}
#ifdef RUPTURE_BUILD
  const uint32_t nd_old_keys=native_audio.keys;
  const uint32_t nd_old_serial=native_audio.alloc_serial;
  const uint8_t nd_old_stack=native_audio.mono_count;
  const uint8_t nd_old_held=native_audio.voice[0].held;
#endif
#if ETCH_NATIVE_USB
  native_midi_drain(native_output_enabled?1u:0u);
#endif
#ifdef RUPTURE_BUILD
  /* MIDI Note On has now reached the ORIGINAL bounded mono stack. Read only
   * the actual winner, so stacked/released pitches, scale locks and sustain
   * keep their original ownership. Do not invent or delay a Note OFF. */
  uint8_t expr_level=ru_touch_live_motion;
  uint8_t expr_source=RU_MIDI_EXPR_NONE,expr_raw=0u;
  ru_touch_midi_held=ru_midi_expr_held(&native_audio);
  if(ru_touch_midi_held &&
     (touch_profile.mode==RU_TOUCH_MODE_PRESS||touch_profile.mode==RU_TOUCH_MODE_VELOCITY)){
   expr_level=ru_midi_expr_level(&ru_midi_expr,&native_audio,&touch_profile,
      &expr_source,&expr_raw);
  }else if(touch_profile.mode==RU_TOUCH_MODE_VELOCITY){
   /* Internal panel/sequence voices already have fixed per-note velocity.
    * The FM1 panel's physical force sensor remains PRESS, not MIDI velocity. */
   const struct etch_native_voice *v=&native_audio.voice[0];
   expr_level=(v->active&&v->held)?
      ru_midi_velocity_level(v->velocity,touch_profile.sensitivity):0u;
  }
  ru_touch_midi_source=expr_source;ru_touch_midi_raw=expr_raw;
  ru_touch_audio_level=expr_level;
  if(touch_profile.target)ru_touch_apply_profile(&c,&touch_profile,expr_level);
#endif
#if ETCH_NATIVE_PANEL_MIDI
  /* STRICT must not transmit disabled keys to an attached USB MIDI synth.
   * Keep the panel's note-off handler active for previously held notes. */
  uint32_t panel_keys=keys;
  if(c.harmony.keylock==3u)
   panel_keys&=etch_harmony_live_key_mask(c.octave,&c.harmony);
  etch_panel_midi_update(&native_panel_out,panel_keys,c.octave,(uint8_t)(usb.config && !native_suspended),native_panel_usb_emit,0);
#endif
#if ETCH_NATIVE_UAC
  uac_render_start();
#endif
  /* Boot gate is silent in DIGITAL AND analog phases. Do not expend full
   * voice+FX DSP cycles before the DAC link is stable. A missed DMA deadline
   * while PC6 is switching can force a restart and a second hard pop. */
  native_audio.mono_mode=(uint8_t)(c.mono_mode!=0 && !etch_native_groups_playing());
  uint32_t render_start=fm1_ticks();
  uint32_t ctrl_elapsed=(uint32_t)(render_start-start);
  if(ctrl_elapsed>native_controls_wall_peak)native_controls_wall_peak=ctrl_elapsed;
  if((native_storage_quiet_request&&!native_storage_audio.remaining) ||
     (!native_output_enabled && !native_output_analog_ready)){
   for(uint32_t i=0;i<ETCH_NATIVE_FRAMES*2;i++)native_stereo[i]=0;
  }else{
   /* Prime DSP while output is muted. Do not silently consume the first
    * held panel notes while the codec/LCD settle gate is still closed.
    * Once audio is audible, unchanged keys enter the original renderer. */
#ifdef RUPTURE_BUILD
   uint32_t render_keys=ru_note_keys_while_output_gated(keys,native_output_enabled);
#else
   uint32_t render_keys=keys;
#endif
   etch_native_render_stereo(&native_audio,&c,render_keys,native_stereo,ETCH_NATIVE_FRAMES);
   if(!native_output_enabled && native_prepared_blocks<65535u)native_prepared_blocks++;
  }
  uint32_t render_elapsed=(uint32_t)(fm1_ticks()-render_start);
#ifdef RUPTURE_BUILD
  const uint8_t nd_edges=native_output_enabled?
    ru_note_dma_edge(nd_old_keys,native_audio.keys,nd_old_serial,
      native_audio.alloc_serial,nd_old_stack,native_audio.mono_count,
      nd_old_held,native_audio.voice[0].held):0u;
#endif
  if(render_elapsed>native_render_wall_peak)native_render_wall_peak=render_elapsed;
#ifdef RUPTURE_BUILD
  if(render_elapsed>native_preset_render_peak)native_preset_render_peak=render_elapsed;
#endif
#if ETCH_NATIVE_PANEL_MIDI
  etch_perf_midi_update(&native_perf_out,&native_audio,&c,(uint8_t)(usb.config && !native_suspended),ETCH_NATIVE_FRAMES,native_panel_usb_emit,0);
#endif
  /* Reject an expired DMA ownership window BEFORE touching the target DMA
   * buffer. MIDI/render/FX can take long enough for the active half to move.
   * If that happens we stop playback rather than corrupting live samples. */
  if(!etch_audio_dma_preflight(half,fm1_audio_free_half(),start,
      fm1_ticks(),ETCH_NATIVE_FRAMES*544u)){
#ifdef RUPTURE_BUILD
   ru_note_dma_record(&native_note_dma,nd_edges,render_elapsed,
                      (uint32_t)(fm1_ticks()-start),1u);
#endif
   fm1_audio_ack_half();
   ++native_dma_blocked;++native_late;
   /* Drop extra voice load (8 > 6 > 4 > 3 > 2) before the watchdog
    * considers stopping DMA; never write to an expired half. */
   native_budget_late(core2_jobs_before);
   native_in_audio=0u;return;
  }
  uint32_t publish_start=fm1_ticks();
  int32_t *dst=&native_dma[half*ETCH_NATIVE_FRAMES*2];
  if(native_preset_ramp_requested){
   native_preset_ramp_requested=0u;
   etch_preset_ramp_begin(&native_preset_ramp);
  }
  uint32_t block_peak=0,block_clips=0;
  /* The mono scope is optional, and a loaded RUPTURE deliberately skips
   * the scope at ui_pressure >= 16 or with MIDI backlog. Do not spend
   * division/write cycles preparing 128 unused scope samples then. The
   * gate is read once; both flags are audio-ISR-owned and remain stable
   * until the next block. PCM DMA, USB and FX samples are untouched. */
  uint8_t prepare_scope=(uint8_t)(!native_midi_backlog&&native_ui_pressure<16u);
#ifdef RUPTURE_BUILD
  /* Skip mono scope averaging/capture around note on/off with 3+ FX, or
   * at <=20% slack for ANY chain, including MOSFET+REVERB. Never run an FX count in the
   * common no-pressure/no-edge case. All 128 PCM frames still publish. */
  const uint32_t scope_budget=ETCH_NATIVE_FRAMES*544u;
  const uint32_t scope_slack=native_last_dma_slack;
  const uint8_t inspect_fx=(uint8_t)(nd_edges||
      native_note_scope_guard.quiet_halves);
  const uint8_t scope_fx=inspect_fx?
      ru_note_fx_active_count(&c.fx_chain,c.fx_chain_active):0u;
  const uint8_t normal_scope=prepare_scope;
  prepare_scope=ru_note_scope_capture_allow(&native_note_scope_guard,
      prepare_scope,nd_edges,scope_fx,scope_slack,scope_budget);
  if(normal_scope&&!prepare_scope&&native_scope_deferred<UINT32_MAX)
   ++native_scope_deferred;
#endif
  const uint8_t storage_block=(uint8_t)(native_storage_quiet_request!=0u);
  const uint8_t storage_zero=(uint8_t)(storage_block&&!native_storage_audio.remaining);
  for(uint32_t i=0;i<ETCH_NATIVE_FRAMES;i++){
   int32_t left=native_stereo[2*i],right=native_stereo[2*i+1];
   /* Hold digital silence during codec warmup, then smoothly fade in.
    * 32767*128 cannot overflow signed 32-bit. No division/heap in IRQ. */
   if(!native_output_enabled){left=0;right=0;}
   else if(native_output_fade<ETCH_OUTPUT_RAMP_SAMPLES){
    /* One-sample resolution fade, unlike the former 32-sample staircase. */
    left=(left*(int32_t)native_output_fade)/(int32_t)ETCH_OUTPUT_RAMP_SAMPLES;
    right=(right*(int32_t)native_output_fade)/(int32_t)ETCH_OUTPUT_RAMP_SAMPLES;
    ++native_output_fade;
   }
   /* Avoid hard PCM jumps when scrolling radically different patches.
    * Never touch the analog enable, FX delay buffers or DMA routing. */
   if(native_output_enabled)etch_preset_ramp_frame(&native_preset_ramp,&left,&right);
   if(storage_block)etch_storage_audio_frame(&native_storage_audio,&left,&right);
   uint32_t pl=(uint32_t)(left<0?-left:left),pr=(uint32_t)(right<0?-right:right);
   if(pl>block_peak)block_peak=pl;if(pr>block_peak)block_peak=pr;
   if(pl>=32767u||pr>=32767u)block_clips++;
   if(prepare_scope)native_mono[i]=(int16_t)((left+right)/2);
   dst[2*i]=left*128;dst[2*i+1]=right*128;
  }
#if ETCH_NATIVE_UAC
  /* All output processing is complete in dst. Its Q15 << 7 samples are
   * decoded by the USB tap without a duplicate stereo PCM allocation. */
  if(uac.feed)uac_tap(dst,ETCH_NATIVE_FRAMES);
#endif
  /* Publish a completed 1024-sample contiguous audio window every 8 DMA
   halves. The renderer sees a stable waveform instead of arbitrary 128-sample
   fragments; the audio ISR does no expensive trigger search or UI drawing. */
  /* On near-deadline blocks audio and UAC beat the oscilloscope: skip
   * optional visualization capture entirely. This NEVER omits a PCM DMA
   * sample, a MIDI event or an FX sample, and does not touch storage.
   * The scope freezes briefly under load rather than stealing deadline.
   * Hysteresis uses the already-owned ISR pressure state. */
  if(prepare_scope &&
     etch_audio_scope_may_capture((uint32_t)(fm1_ticks()-start),
        ETCH_NATIVE_FRAMES*544u,native_ui_pressure)){
   uint8_t scope_to=native_scope_ready^1u;
  for(uint32_t i=0;i<ETCH_NATIVE_FRAMES;i++)
   native_scope[scope_to][native_scope_cursor+i]=native_mono[i];
  native_scope_cursor=(uint16_t)(native_scope_cursor+ETCH_NATIVE_FRAMES);
  if(block_peak>native_scope_window_peak)native_scope_window_peak=block_peak;
  if(native_scope_cursor==ETCH_SCOPE_CAPTURE){
   native_scope_cursor=0;native_scope_ready=scope_to;
   uint8_t any_held=0u;
   for(uint8_t vi=0;vi<ETCH_NATIVE_VOICES;vi++){
    const struct etch_native_voice *voice=&native_audio.voice[vi];
    if(voice->active && voice->held){any_held=1u;break;}
   }
   native_scope_held_note=any_held;
   /* A whole 1024-sample window is authoritative. Decay for two
    * complete windows, not every 128-sample ARP gate/rest. */
   if(native_scope_window_peak>256u){native_scope_has_signal=1u;native_scope_silent_windows=0u;}
   else if(native_scope_silent_windows<3u){
    ++native_scope_silent_windows;
    if(native_scope_silent_windows>=3u)native_scope_has_signal=0u;
   }
   native_scope_window_peak=0u;
  }
  }else if(native_scope_deferred<UINT32_MAX){
   ++native_scope_deferred;
  }
  uint32_t publish_elapsed=(uint32_t)(fm1_ticks()-publish_start);
  if(publish_elapsed>native_publish_wall_peak)native_publish_wall_peak=publish_elapsed;
  if(block_peak>native_audio_peak)native_audio_peak=block_peak;
  if(block_clips>UINT32_MAX-native_audio_clip_frames)native_audio_clip_frames=UINT32_MAX;
  else native_audio_clip_frames+=block_clips;
  fm1_audio_ack_half();++native_halves;
  /* Deadline uses total wall time (USB/input interrupts still consume it).
   * CPU meter excludes TIMER5 time and must not misreport USB work as DSP.
   * A moved DMA half is always a fault, regardless of measured duration. */
  struct etch_audio_timing measure=etch_audio_timing_end(start,fm1_ticks(),
      native_nested_ticks,ETCH_NATIVE_FRAMES*544u,
      (uint8_t)(fm1_audio_free_half()!=half));
#ifdef RUPTURE_BUILD
  ru_note_dma_record(&native_note_dma,nd_edges,render_elapsed,
                     measure.wall_ticks,measure.late);
#endif
  if(measure.render_ticks>native_max_ticks)native_max_ticks=measure.render_ticks;
  if(measure.wall_ticks>native_max_wall_ticks)native_max_wall_ticks=measure.wall_ticks;
#ifdef RUPTURE_BUILD
  if(measure.wall_ticks>native_preset_wall_peak)native_preset_wall_peak=measure.wall_ticks;
#endif
  uint32_t slack=measure.wall_ticks>=ETCH_NATIVE_FRAMES*544u?0u:
     ETCH_NATIVE_FRAMES*544u-measure.wall_ticks;
  native_last_dma_slack=slack;
  if(slack<native_min_dma_slack)native_min_dma_slack=slack;
#ifdef RUPTURE_BUILD
  if(slack<native_preset_min_slack)native_preset_min_slack=slack;
#endif
  if(measure.nested_ticks>native_max_nested_ticks)native_max_nested_ticks=measure.nested_ticks;
  /* LCD frames are expensive SPI DMA transfers. Track recent wall-time
   * pressure so the UI backs off before repeated audio underruns. This
   * counter never changes oscillator, MIDI or sequencer timing. */
  if(measure.late || measure.wall_ticks>ETCH_NATIVE_FRAMES*544u*65u/100u){
   if(native_ui_pressure<29u)native_ui_pressure+=3u;
   else native_ui_pressure=32u;
  }else if((native_halves&15u)==0u && native_ui_pressure)native_ui_pressure--;
  if(measure.late){
   ++native_postpublish_late;
   ++native_late;
   /* A rare late DMA boundary must not disable the instrument forever.
    * The pre-write ownership guard above already rejects expired halves.
    * Escalate only repeated deadlines, and expose LATE/GUARD counters. */
   native_budget_late(core2_jobs_before);
  }else{
   native_budget_ok();
   if(storage_block)etch_storage_audio_published(&native_storage_audio,storage_zero,half);
  }
 }
 native_in_audio=0u;
}
static void native_start(void){
 if(native_storage_audio_failed)return; /* explicit HOME+PLAY rearm only */
 const uint8_t warm=native_audio_clock_live;
 if(warm&&!native_suspended)return; /* caller already resumed nested SAVE */
 const uint8_t play=(uint8_t)(warm&&native_boot_audio_released);
 const uint8_t analog=(uint8_t)(warm&&native_output_analog_ready);
 native_storage_read_resume=0u;native_storage_quiet_request=0u;
 native_storage_audio_failed=0u;
 native_suspended=0;native_missed_streak=0;native_healthy_blocks=0;native_tripped=0;native_ui_pressure=0;
 native_render_wall_peak=native_publish_wall_peak=native_scope_deferred=0u;
 native_controls_wall_peak=native_midi_queue_peak=native_midi_deferred_batches=0u;
 /* LATE and GUARD remain cumulative across audio restarts; POST LATE must
  * remain cumulative too, so LATE == GUARD + POST LATE stays meaningful. */
 native_midi_batch_peak=0u;native_midi_backlog=0u;
 native_min_dma_slack=native_last_dma_slack=ETCH_NATIVE_FRAMES*544u;
#ifdef RUPTURE_BUILD
 native_preset_profile_reset();
 native_note_scope_guard.quiet_halves=0u;
#endif
 native_output_enabled=play;native_output_analog_ready=analog;
 native_boot_audio_released=play;native_output_fade=0;native_prepared_blocks=0;
 if(!warm)native_halves=0;
 native_preset_ramp_requested=0u;etch_preset_ramp_reset(&native_preset_ramp);
#if ETCH_NATIVE_USB
 mi_r=mi_w;RING_PUBLISH();midi_in_overflow=0;
#endif
#ifdef RUPTURE_BUILD
 ru_midi_expr_reset(&ru_midi_expr);
 ru_touch_midi_held=0u;ru_touch_midi_source=0u;
 ru_touch_midi_raw=0u;ru_touch_audio_level=0u;
#endif
 etch_native_reset(&native_audio);
 native_scope_ready=0;native_scope_cursor=0;native_scope_has_signal=0;native_scope_held_note=0;native_scope_window_peak=0;native_scope_silent_windows=0;etch_output_usb_reset(&native_usb_gate);
 for(uint32_t i=0;i<ETCH_SCOPE_CAPTURE;i++){native_scope[0][i]=0;native_scope[1][i]=0;}
 etch_panel_midi_reset(&native_panel_out);
 /* A warm pause has already handed off two zero halves. Never rewrite an
  * active DMA buffer, restart I2S or toggle PC6 just to access storage. */
 if(warm)return;
 for(uint32_t i=0;i<ETCH_NATIVE_FRAMES*4;i++)native_dma[i]=0;
 fm1_audio_output_enable(0u);
 fm1_audio_init(native_dma,ETCH_NATIVE_FRAMES*2,isr_alnk0,3);
 native_audio_clock_live=1u;
 etch_output_gate_reset(&native_output_gate,fm1_ticks());
 /* Every DMA restart must re-arm the silent-first-frame gate. The current
  * UI stays visible on warm restart, but cannot publish audio until after
  * the analog stage has settled and the PLAY renderer has completed. */
 etch_output_screen_reset(&native_screen_gate,fm1_ticks());
}
