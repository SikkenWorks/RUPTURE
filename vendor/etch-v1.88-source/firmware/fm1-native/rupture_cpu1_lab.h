/* SPDX-License-Identifier: GPL-3.0-only
 * Experimental AC791N CPU1 bring-up, for hardware qualification only.
 * Register sequence and fault handling adapted from the GPL-3 JIANT FM1
 * multicore work (Juan Ignacio Antonucci), in turn based on Melodee/X0X.
 * In integrity/FIR-probe mode, CPU1 may not write audio engine, DMA, display,
 * flash or CXS state; the FIR probe only reads immutable coefficients.
 * In V2.0.58's explicit FX LIVE LAB build, CPU1 instead owns the complete
 * shared FX state for one 128-frame streaming job with two 64-frame
 * producer watermarks. That experimental
 * route is NOT hardware-qualified for production use.
 */
#pragma once
#include <stdint.h>
#include "rupture_cpu1_protocol.h"
#include "rupture_cpu1_fir_probe.h"
#include "rupture_cpu1_offload_guard.h"
#include "rupture_core2_fx_guard.h"
#include "rupture_core2_fx_stream.h"
#include "rupture_core2_fx_wait_ema.h"
#if ETCH_CPU1_UAC_LIVE_LAB
#include "rupture_cpu1_uac_block.h"
#endif

#if ETCH_CPU1_LAB
#define RU_C1_CON (*(volatile uint32_t *)0x1EEE004u)
#define RU_C1_START (*(volatile uint32_t *)0x10008u)
#define RU_C1_SYNC() __asm__ volatile("csync" ::: "memory")
#define RU_C1_DBG_C0 ((1u<<7)|(1u<<9)|(1u<<12)|(1u<<13)|(7u<<19))
#define RU_C1_DBG_C1 ((1u<<6)|(1u<<8)|(1u<<10)|(1u<<11)|(7u<<16))
#define RU_C1_BOOT_MAGIC 0x31555052u
enum {RU_C1_OK=0u,RU_C1_BAD_INPUT=1u,RU_C1_TIMEOUT=2u,
      RU_C1_FAULT=3u,RU_C1_BAD_OUTPUT=4u,RU_C1_NO_START=5u,
      RU_C1_PRIOR_RESET=6u,RU_C1_PRESSURE=7u};

extern void ru_cpu1_entry(void);
extern uint8_t _ru_c1_stack_lo[],_ru_c1_stack_hi[];
static ru_cpu1_mailbox_t ru_c1_box __attribute__((section(".c1_ram"),aligned(64)));
#if ETCH_CPU1_UAC_LIVE_LAB
static ru_cpu1_uac_box_t ru_c1_audio_box __attribute__((section(".c1_ram"),aligned(64)));
static uint32_t ru_c1_audio_seq,ru_c1_audio_at,ru_c1_audio_verified;
static uint32_t ru_c1_audio_commits,ru_c1_audio_seen,ru_c1_audio_fallbacks,ru_c1_audio_spot_fail;
static uint32_t ru_c1_audio_pressure_trips,ru_c1_audio_last_pressure;
static uint32_t ru_c1_audio_max_wait_ticks,ru_c1_audio_ring_aborts;
static ru_cpu1_pressure_guard_t ru_c1_pressure;
static uint8_t ru_c1_audio_pending;
#endif
#if ETCH_CPU1_FX_LIVE_LAB
/* CPU1 owns every FX state mutation while a sequence is outstanding.
 * The actual PCM/settings lane lives in shared .pool, NOT this mailbox. */
typedef struct {
 volatile uint32_t seq,done,begin,end,progress,bad,ready;
} ru_cpu1_fx_box_t;
static ru_cpu1_fx_box_t ru_c1_fx_box __attribute__((section(".c1_ram"),aligned(64)));
static volatile uint32_t ru_c1_fx_commits,ru_c1_fx_fallbacks,ru_c1_fx_timeout;
static volatile uint32_t ru_c1_fx_max_wait_ticks,ru_c1_fx_avg_wait_ticks;
static uint32_t ru_c1_fx_at,ru_c1_fx_seq;
extern uint32_t ru_fx_cpu1_run(uint32_t first,uint32_t end,volatile uint32_t *progress);
#define RU_C1_FX_TIMEOUT_TICKS 26000u /* 1.08 ms after final 128-frame publish */
#define RU_C1_FX_POLL_NOP_COUNT 32u /* CORE0-local; no CPU1/shared SRAM access */
#endif
static struct {uint32_t magic,started,barred;} ru_c1_boot __attribute__((section(".noinit")));
static volatile uint32_t ru_c1_retired,ru_c1_good,ru_c1_fir_good,ru_c1_faults,ru_c1_late,ru_c1_bad;
static uint32_t ru_c1_wait_seq,ru_c1_wait_at,ru_c1_last_submit;
static uint8_t ru_c1_online,ru_c1_wait_kind;

/* No other core owns this register sequence. Bus faults from a hold can be
 * delayed; mask bus-invalid traps, settle, clear, then restore them. */
static inline __attribute__((always_inline)) void ru_cpu1_halt(void)
{
 uint32_t traps;
 if(!(FM1_DBG_WR_EN&1u))FM1_DBG_WR_EN=0xe7u;
 traps=FM1_DBG_EN;
 FM1_DBG_EN &= ~((0x3fu<<16)|(3u<<4));
 RU_C1_CON|=2u;RU_C1_CON&=~8u;
 RU_C1_SYNC();
 uint32_t t=fm1_ticks();
 while((uint32_t)(fm1_ticks()-t)<2400u)RU_C1_SYNC(); /* 100 us */
 FM1_DBG_MSG_CLR=RU_C1_DBG_C1;
 FM1_DBG_EN=traps;
 RU_C1_SYNC();
 if(FM1_DBG_WR_EN&1u)FM1_DBG_WR_EN=0xe7u;
}

static inline void ru_cpu1_retire(uint32_t reason)
{
 ru_cpu1_halt();
 ru_c1_online=0u;
 ru_c1_retired=reason;
 ru_c1_boot.barred=1u;ru_c1_boot.started=0u;
 ru_c1_wait_seq=0u;
}

/* The hardware ROM enters here via ru_cpu1_entry (crt0), with core1's
 * separate stack and interrupt bank. The idle loop is RAM-resident and
 * intentionally paced: affected FM1 chips misread busy-loop mailboxes. */
void __attribute__((section(".ram_text"),noinline,used,noreturn)) ru_cpu1_worker(void)
{
 uint32_t lo=(uint32_t)(uintptr_t)_ru_c1_stack_lo+256u;
 uint32_t hi=(uint32_t)(uintptr_t)_ru_c1_stack_hi-1u;
 *(volatile uint32_t *)0x1EEF2D8u=hi;
 *(volatile uint32_t *)0x1EEF2DCu=lo;
 *(volatile uint32_t *)0x1EEF2E0u=hi;
 *(volatile uint32_t *)0x1EEF2E4u=lo;
 *(volatile uint32_t *)0x1EEF2D0u=
  (*(volatile uint32_t *)0x1EEF2D0u&~((1u<<2)|(0x1fu<<16)))|(1u<<3);
 uint32_t done=ru_c1_box.done;
#if ETCH_CPU1_FX_LIVE_LAB
 uint32_t fx_done=ru_c1_fx_box.done;
#endif
#if ETCH_CPU1_UAC_LIVE_LAB
 uint32_t audio_done=ru_c1_audio_box.done;
#endif
 ru_c1_box.alive=RU_C1_MAGIC;
 RU_C1_SYNC();
 for(;;){
#if ETCH_CPU1_FX_LIVE_LAB
  uint32_t fx_seq=ru_c1_fx_box.seq;
  RU_C1_SYNC();
  if(fx_seq!=fx_done){
   uint32_t begin=ru_c1_fx_box.begin,end=ru_c1_fx_box.end;
   /* RAMTEXT -> XIP is >23-bit direct-branch range. Indirect far call. */
   uint32_t (*volatile fx_run)(uint32_t,uint32_t,volatile uint32_t *)=
     ru_fx_cpu1_run;
   uint32_t progress=ru_c1_fx_box.progress;
   RU_C1_SYNC();
   if(fx_seq!=fx_done+1u||
      !ru_c1_fx_stream_start(begin,end,ru_c1_fx_box.ready,progress)){
    ru_c1_fx_box.bad=RU_C1_BAD_INPUT;
    RU_C1_SYNC();for(;;)__asm__ volatile("nop");
   }
   while(progress<end){
    RU_C1_SYNC();
    uint32_t ready=ru_c1_fx_box.ready;
    if(!ru_c1_fx_stream_watermark(progress,ready)){
     ru_c1_fx_box.bad=RU_C1_BAD_INPUT;
     RU_C1_SYNC();for(;;)__asm__ volatile("nop");
    }
    if(ready==progress){
     /* Keep CPU1 off the shared SRAM bus between watermark polls.
      * CPU0 must be able to finish synth/USB work uninterrupted. */
     for(uint8_t n=0u;n<16u;n++)__asm__ volatile("nop");
     continue;
    }
    if(fx_run(progress,ready,&ru_c1_fx_box.progress)!=ready){
     ru_c1_fx_box.bad=RU_C1_BAD_INPUT;
     RU_C1_SYNC();for(;;)__asm__ volatile("nop");
    }
    progress=ready;
   }
   fx_done=fx_seq;
   RU_C1_SYNC();ru_c1_fx_box.done=fx_seq;RU_C1_SYNC();
   continue;
  }
#endif
#if ETCH_CPU1_UAC_LIVE_LAB
  uint32_t audio_seq=ru_c1_audio_box.seq;
  RU_C1_SYNC();
  if(audio_seq!=audio_done){
   if(audio_seq!=audio_done+1u||!ru_cpu1_uac_process(&ru_c1_audio_box,audio_seq)){
    ru_c1_audio_box.bad=RU_C1_BAD_INPUT;
    RU_C1_SYNC();
    for(;;)__asm__ volatile("nop");
   }
   audio_done=audio_seq;
   RU_C1_SYNC();
   ru_c1_audio_box.done=audio_seq;
   RU_C1_SYNC();
   continue;
  }
#endif
  uint32_t seq=ru_c1_box.seq;
  RU_C1_SYNC();
  if(seq==done){
   for(uint32_t n=0u;n<32u;n++)__asm__ volatile("nop");
   continue;
  }
  if(seq!=done+1u || !(ru_c1_box.kind==RU_C1_JOB_INTEGRITY?
     ru_cpu1_process(&ru_c1_box,seq):
     ru_cpu1_fir_process(&ru_c1_box,seq))){
   ru_c1_box.bad=RU_C1_BAD_INPUT;
   RU_C1_SYNC();
   for(;;)__asm__ volatile("nop");
  }
  done=seq;
  RU_C1_SYNC();
  ru_c1_box.done=seq;
  RU_C1_SYNC();
 }
}

static inline int ru_cpu1_boot_start(void)
{
 if(ru_c1_boot.magic!=RU_C1_BOOT_MAGIC){
  ru_c1_boot.magic=RU_C1_BOOT_MAGIC;ru_c1_boot.started=0u;ru_c1_boot.barred=1u;
 }
 if(fm1_boot.p3_rst&1u){ru_c1_boot.started=0u;ru_c1_boot.barred=0u;}
 else if(ru_c1_boot.started){ru_c1_boot.barred=1u;}
 if(ru_c1_boot.barred){ru_c1_retired=RU_C1_PRIOR_RESET;return 0;}
 ru_cpu1_halt();
 for(uint32_t i=0u;i<sizeof(ru_c1_box)/4u;i++)
  ((volatile uint32_t *)(void *)&ru_c1_box)[i]=0u;
#if ETCH_CPU1_UAC_LIVE_LAB
 for(uint32_t i=0u;i<sizeof(ru_c1_audio_box)/4u;i++)
  ((volatile uint32_t *)(void *)&ru_c1_audio_box)[i]=0u;
#endif
#if ETCH_CPU1_FX_LIVE_LAB
 for(uint32_t i=0u;i<sizeof(ru_c1_fx_box)/4u;i++)
  ((volatile uint32_t *)(void *)&ru_c1_fx_box)[i]=0u;
#endif
 /* CPU1 uses its own interrupt controller bank; leave every IRQ off. */
 for(uint32_t i=0u;i<32u;i++)*(volatile uint32_t *)(0x1EEF300u+4u*i)=0u;
 if(!(FM1_DBG_WR_EN&1u))FM1_DBG_WR_EN=0xe7u;
 FM1_PC_LIMIT0_L=FM1_PC_LIMIT1_L=0u;
 FM1_PC_LIMIT0_H=FM1_PC_LIMIT1_H=0xffffffffu;
 if(FM1_DBG_WR_EN&1u)FM1_DBG_WR_EN=0xe7u;
 *(volatile uint32_t *)0x01C7FFF8u=(uint32_t)(uintptr_t)&ru_cpu1_entry;
 uint32_t old=RU_C1_START;
 RU_C1_START=old|8u;
 RU_C1_SYNC();
 RU_C1_CON|=8u;
 RU_C1_CON&=~2u;
 uint32_t t=fm1_ticks();
 while(ru_c1_box.alive!=RU_C1_MAGIC && (uint32_t)(fm1_ticks()-t)<480000u)
  RU_C1_SYNC();
 RU_C1_START=old;
 fm1_guard_enable(FM1_GUARD_PC);
 if(ru_c1_box.alive!=RU_C1_MAGIC){
  ru_cpu1_retire(RU_C1_NO_START);
  return 0;
 }
 ru_c1_online=1u;
 ru_c1_boot.started=1u; /* if a watchdog reset occurs, next boot stays on CPU0 */
 return 1;
}

static inline void ru_cpu1_diag_service(uint32_t now_ms,uint8_t quiet,uint8_t recording)
{
 if(!ru_c1_online)return;
 if(ru_c1_faults){ru_cpu1_retire(RU_C1_FAULT);return;}
 if(ru_c1_wait_seq){
  RU_C1_SYNC();
  if(ru_c1_box.bad||ru_c1_box.kind!=ru_c1_wait_kind){
   ru_c1_bad++;ru_cpu1_retire(RU_C1_BAD_INPUT);return;
  }
  if(ru_c1_box.done==ru_c1_wait_seq){
   int valid=ru_c1_wait_kind==RU_C1_JOB_FIR?
    ru_cpu1_fir_verify(&ru_c1_box,ru_c1_wait_seq):
    ru_cpu1_verify(&ru_c1_box,ru_c1_wait_seq);
   if(!valid){
    ru_c1_bad++;ru_cpu1_retire(RU_C1_BAD_OUTPUT);return;
   }
   if(ru_c1_wait_kind==RU_C1_JOB_FIR)ru_c1_fir_good++;
   ru_c1_good++;ru_c1_wait_seq=0u;
  }else if((uint32_t)(now_ms-ru_c1_wait_at)>4u){
   ru_c1_late++;ru_cpu1_retire(RU_C1_TIMEOUT);return;
  }
  return;
 }
#if ETCH_CPU1_FX_LIVE_LAB
 /* NEVER issue foreground integrity/FIR jobs on the active realtime FX
  * core. Those 20ms probes could queue behind FX, exceed their 4ms deadline
  * and retire CPU1 during playback. Qualification was already completed. */
 if(ru_fx_probe_finished(ru_c1_good,ru_c1_fir_good))return;
#endif
 if((uint32_t)(now_ms-ru_c1_last_submit)<20u)return;
#if ETCH_CPU1_UAC_LIVE_LAB
 if(ru_c1_audio_pending||recording)return;
#else
 (void)recording;
#endif
 uint32_t seq=ru_c1_box.done+1u;
 uint32_t seed=ru_cpu1_mix(now_ms^seq^0x5170bba2u);
 uint32_t kind=(quiet&&ru_c1_good>0u&&(ru_c1_good&63u)==0u)?
    RU_C1_JOB_FIR:RU_C1_JOB_INTEGRITY;
 ru_c1_box.seed=seed;
 for(uint32_t i=0u;i<RU_C1_WORDS;i++)
  ru_c1_box.input[i]=ru_cpu1_expected_input(seed,i);
 ru_c1_box.kind=kind;
 ru_c1_box.check=kind==RU_C1_JOB_FIR?
  ru_cpu1_fir_check(seq,seed):ru_cpu1_descriptor_check(seq,seed);
 ru_c1_box.result=0u;
 RU_C1_SYNC();
 ru_c1_box.seq=seq;
 RU_C1_SYNC();
 ru_c1_wait_kind=(uint8_t)kind;
 ru_c1_wait_seq=seq;ru_c1_wait_at=now_ms;ru_c1_last_submit=now_ms;
}

#if ETCH_CPU1_FX_LIVE_LAB
/* Linked from audio_core_core2.c. Called in ALNK0, not foreground. After
 * qualification, only this streamed 128-frame FX job owns CPU1. */
int ru_cpu1_fx_available(void){
 return ru_c1_online&&!ru_c1_wait_seq&&
        ru_c1_good>=128u&&ru_c1_fir_good>=1u&&
        ru_c1_fx_box.seq==ru_c1_fx_box.done;
}
void ru_cpu1_fx_send(uint32_t begin,uint32_t end){
 if(!ru_c1_online||begin!=0u||end!=RU_C1_FX_STREAM_FRAMES)return;
 uint32_t seq=ru_c1_fx_box.done+1u;
 ru_c1_fx_box.begin=begin;ru_c1_fx_box.end=end;
 ru_c1_fx_box.progress=begin;ru_c1_fx_box.bad=0u;
 ru_c1_fx_box.ready=RU_C1_FX_STREAM_CHUNK;
 RU_C1_SYNC();ru_c1_fx_at=fm1_ticks();ru_c1_fx_seq=seq;
 /* Publish sequence LAST. The first 64 dry samples are now immutable. */
 ru_c1_fx_box.seq=seq;RU_C1_SYNC();
}
void ru_cpu1_fx_publish(uint32_t ready){
 if(!ru_c1_online||ru_c1_fx_box.seq!=ru_c1_fx_seq||
    !ru_c1_fx_stream_publish(ru_c1_fx_box.ready,ready))return;
 /* PCM samples and the matching 64-frame settings window are complete.
  * Never let CPU1 inspect a partially-written dry sample. */
 RU_C1_SYNC();ru_c1_fx_box.ready=ready;RU_C1_SYNC();
 if(ready==RU_C1_FX_STREAM_FRAMES)ru_c1_fx_at=fm1_ticks();
}
uint32_t ru_cpu1_fx_collect(void){
 uint32_t seq=ru_c1_fx_seq,end=ru_c1_fx_box.end;
 if(!ru_c1_online){
  ru_c1_fx_fallbacks++;
  return ru_fx_safe_progress(ru_c1_fx_box.begin,end,ru_c1_fx_box.progress);
 }
 uint8_t corrupt=0u;
 while((uint32_t)(fm1_ticks()-ru_c1_fx_at)<RU_C1_FX_TIMEOUT_TICKS){
  RU_C1_SYNC();
  uint32_t done=ru_c1_fx_box.done;
  if(done==seq){
   /* Read progress ONLY at completion, not inside the tight shared-SRAM
    * wait loop: avoid extra bus traffic that itself could cause DMA LATES. */
   uint8_t reply=ru_fx_lane_reply(seq,done,ru_c1_fx_box.bad,
                       ru_c1_fx_box.begin,end,ru_c1_fx_box.progress);
   if(reply==RU_FX_REPLY_COMPLETE){
    uint32_t waited=(uint32_t)(fm1_ticks()-ru_c1_fx_at);
    if(waited>ru_c1_fx_max_wait_ticks)ru_c1_fx_max_wait_ticks=waited;
    /* Diagnostic-only EWMA; never update the average on a failed FX job. */
    ru_c1_fx_avg_wait_ticks=ru_fx_wait_ema_next(ru_c1_fx_avg_wait_ticks,
                                                waited,ru_c1_fx_commits!=0u);
    ru_c1_fx_commits++;
    return end;
   }
   corrupt=1u;break;
  }
  if(done!=seq-1u||ru_c1_fx_box.bad){corrupt=1u;break;}
  /* CPU0 otherwise executes csync plus shared-SRAM mailbox reads as fast
   * as the bus allows. Thirty-two register-only NOPs between inconclusive
   * polls reduce speculative shared-bus traffic while CPU1 works. A known
   * failure is still retired IMMEDIATELY after its mailbox read; the same
   * wall-clock timeout, fault detection and exact PCM fallback remain. */
  for(uint8_t n=0u;n<RU_C1_FX_POLL_NOP_COUNT;n++)__asm__ volatile("nop");
 }
 /* A stalled CPU1 must be HALTED before CPU0 writes s->fx again. Resume
  * from the last completed frame. No outstanding intercore writes remain. */
 if(!corrupt)ru_c1_fx_timeout++;
 ru_cpu1_retire(corrupt?RU_C1_BAD_OUTPUT:RU_C1_TIMEOUT);
 RU_C1_SYNC();
 ru_c1_fx_fallbacks++;
 return ru_fx_safe_progress(ru_c1_fx_box.begin,end,ru_c1_fx_box.progress);
}
#endif

#if ETCH_CPU1_UAC_LIVE_LAB
/* DANGER: real USB FIR hand-off. Only included in a separately opted-in
 * engineering image. Core1 NEVER writes the actual UA ring or FIR state.
 * The first 64 blocks use the original CPU0 FIR and verify *all* samples.
 * Later blocks use CPU1 output with 8 independent CPU0 sample checks, an
 * input/phase/output envelope and periodic full calibration on reset.
 * This catches many bad reads but CANNOT mathematically rule out a single
 * corrupted, unsampled output. Not for production hardware yet. */
#define RU_C1_UAC_PROBATION 64u

static void ru_cpu1_uac_live_reset(void)
{
 ru_c1_audio_verified=0u;
 ru_c1_audio_seen=0u;
 ru_c1_pressure.armed=0u;
}

/* Run from CPU0 foreground before draining the next USB block. These are
 * observation counters, not shared writable DSP or interrupt structures.
 * Retire permanently on a *new* fault while live 48k capture is active. */
static inline void ru_cpu1_uac_pressure_watch(uint8_t active,
 uint32_t dma_lates,uint32_t dma_guard,uint32_t usb_underruns,
 uint32_t stage_drops,uint8_t audio_stopped)
{
 uint32_t flags=ru_cpu1_pressure_update(&ru_c1_pressure,
  (uint8_t)(active&&ru_c1_online),dma_lates,dma_guard,
  usb_underruns,stage_drops,audio_stopped);
 if(flags){
  ru_c1_audio_last_pressure=flags;
  ru_c1_audio_pressure_trips++;
  ru_cpu1_retire(RU_C1_PRESSURE);
 }
}

static int ru_cpu1_uac_live_ready(void)
{
#if ETCH_CPU1_FX_LIVE_LAB
 /* Reserve CPU1 for FX and keep 48k FIR on CPU0. Do not race audio IRQ. */
 return 0;
#else
 return ru_c1_online && !ru_c1_audio_pending && !ru_c1_wait_seq &&
        ru_c1_good>=128u && ru_c1_fir_good>=1u;
#endif
}

static int ru_cpu1_uac_live_submit(const uint32_t *packed,uint32_t n)
{
 if(!ru_cpu1_uac_live_ready()||n!=RU_C1_UAC_IN||
    ru_c1_audio_box.done!=ru_c1_audio_box.seq||
    UA_N-(ua_w-ua_r)<n+n/8u+2u)return 0;
 uint32_t seq=ru_c1_audio_box.done+1u;
 ru_c1_audio_box.seed=ru_cpu1_mix(seq^fm1_ticks()^0x49a581f3u);
 ru_c1_audio_box.phase=rs.ph;
 for(uint32_t i=0u;i<RU_C1_UAC_HIST;i++){
  uint32_t idx=(rs.w-RU_C1_UAC_HIST+i)&(RS_H-1u);
  ru_c1_audio_box.history[i]=(uint32_t)(uint16_t)rs.l[idx]|
    (uint32_t)(uint16_t)rs.r[idx]<<16u;
 }
 for(uint32_t i=0u;i<RU_C1_UAC_IN;i++)ru_c1_audio_box.input[i]=packed[i];
 /* The request sequence is the doorbell and MUST be written LAST. */
 ru_c1_audio_box.check=ru_cpu1_uac_input_hash(&ru_c1_audio_box,seq);
 ru_c1_audio_box.bad=0u;
 ru_c1_audio_box.count=0u;
 RU_C1_SYNC();
 ru_c1_audio_at=fm1_ticks();
 ru_c1_audio_seq=seq;
 ru_c1_audio_pending=1u;
 RU_C1_SYNC();
 ru_c1_audio_box.seq=seq;
 RU_C1_SYNC();
 return 1;
}

static int ru_cpu1_uac_live_poll(void)
{
 if(!ru_c1_audio_pending)return 0;
 uint32_t seq=ru_c1_audio_seq;
 /* Block is owned by ru_uac_block until the response is committed or the
  * CPU0 reference handles it. Nobody dequeues another block during wait. */
 if(!ru_c1_online){
  ru_c1_audio_pending=0u;ru_c1_audio_fallbacks++;
  if(usb.up&&usb.config&&uac.alt&&uac.r48&&uac.flowing&&uac.go)
   uac_tap48(ru_uac_block,RU_C1_UAC_IN);
  return 0;
 }
 if(!usb.up||!usb.config||!uac.alt||!uac.r48||!uac.flowing||!uac.go){
  ru_cpu1_retire(RU_C1_FAULT);ru_c1_audio_pending=0u;
  ru_c1_audio_fallbacks++;return 0;
 }
 RU_C1_SYNC();
 if(ru_c1_audio_box.bad){
  ru_cpu1_retire(RU_C1_BAD_INPUT);ru_c1_audio_pending=0u;
  ru_c1_audio_fallbacks++;uac_tap48(ru_uac_block,RU_C1_UAC_IN);return 0;
 }
 uint32_t dt=(uint32_t)(fm1_ticks()-ru_c1_audio_at);
 uint32_t decision=ru_cpu1_offload_decide(
  (uint8_t)(ru_c1_audio_box.done==seq),dt,ua_w-ua_r);
 if(decision==RU_C1_WAIT)return 1;
 if(decision==RU_C1_ABORT_LATE||decision==RU_C1_ABORT_BUFFER){
  if(decision==RU_C1_ABORT_BUFFER){
   ru_c1_audio_ring_aborts++;ru_cpu1_retire(RU_C1_PRESSURE);
  }else ru_cpu1_retire(RU_C1_TIMEOUT);
  ru_c1_audio_pending=0u;
  ru_c1_audio_fallbacks++;uac_tap48(ru_uac_block,RU_C1_UAC_IN);return 0;
 }
 if(dt>ru_c1_audio_max_wait_ticks)ru_c1_audio_max_wait_ticks=dt;
 uint32_t hist[RU_C1_UAC_HIST];
 for(uint32_t i=0u;i<RU_C1_UAC_HIST;i++){
  uint32_t idx=(rs.w-RU_C1_UAC_HIST+i)&(RS_H-1u);
  hist[i]=(uint32_t)(uint16_t)rs.l[idx]|
          (uint32_t)(uint16_t)rs.r[idx]<<16u;
 }
 if(!ru_cpu1_uac_verify_envelope(&ru_c1_audio_box,seq,
       ru_uac_block,hist,rs.ph)){
  ru_cpu1_retire(RU_C1_BAD_OUTPUT);ru_c1_audio_pending=0u;
  ru_c1_audio_fallbacks++;uac_tap48(ru_uac_block,RU_C1_UAC_IN);return 0;
 }
 if(ru_c1_audio_verified<RU_C1_UAC_PROBATION||
    ((ru_c1_audio_seen&255u)==255u)){
  /* Full PCM parity while qualifying; the CPU0 reference is committed,
   * even when CPU1 returned bad samples. This stage saves no CPU cycles. */
  uint32_t w=ua_w;
  uac_tap48(ru_uac_block,RU_C1_UAC_IN);
  int okay=ua_w-w==ru_c1_audio_box.count&&rs.ph==ru_c1_audio_box.next_phase;
  if(okay)for(uint32_t i=0u;i<ru_c1_audio_box.count;i++)
   if(ua_ring[(w+i)&(UA_N-1u)]!=ru_c1_audio_box.output[i]){okay=0;break;}
  if(okay){
   if(ru_c1_audio_verified<RU_C1_UAC_PROBATION)
    ru_c1_audio_verified++;
   else ru_c1_audio_seen++;
  }
  else{ru_c1_audio_spot_fail++;ru_cpu1_retire(RU_C1_BAD_OUTPUT);}
  ru_c1_audio_pending=0u;
  return 0;
 }
 /* Live offload path: 8 deterministic rotating exact checks plus complete
  * copied-input/phase/checksum validation. An isolated unsampled error can
  * escape; never claim this alone is sufficient for production safety. */
 for(uint32_t j=0u;j<8u;j++){
  uint32_t target=(ru_cpu1_mix(ru_c1_audio_box.seed+j*0x9e3779b9u)%
                   ru_c1_audio_box.count);
  if(ru_cpu1_uac_expected_one(&ru_c1_audio_box,target)!=ru_c1_audio_box.output[target]){
   ru_c1_audio_spot_fail++;ru_cpu1_retire(RU_C1_BAD_OUTPUT);
   ru_c1_audio_pending=0u;ru_c1_audio_fallbacks++;
   uac_tap48(ru_uac_block,RU_C1_UAC_IN);return 0;
  }
 }
 if(UA_N-(ua_w-ua_r)<ru_c1_audio_box.count){
  ru_c1_audio_pending=0u;uac.overruns++;
  return 0;
 }
 uint32_t w=ua_w;
 for(uint32_t i=0u;i<ru_c1_audio_box.count;i++)
  ua_ring[(w+i)&(UA_N-1u)]=ru_c1_audio_box.output[i];
 ru_cpu1_uac_apply_history(rs.l,rs.r,&rs.w,&rs.ph,
                           ru_uac_block,ru_c1_audio_box.next_phase);
 RING_PUBLISH();
 ua_w=w+ru_c1_audio_box.count;
 ru_c1_audio_commits++;
 ru_c1_audio_seen++;
 ru_c1_audio_pending=0u;
 return 0;
}
#endif

/* Uses JIANT's core-number and DBG_MSG split; CPU0 faults still take the
 * original fatal recovery path. A CPU1 fault only retires its worker. */
void ru_cpu1_fault_dispatch(uint32_t *f)
{
 uint32_t core,dbg=FM1_DBG_MSG;
 __asm__ volatile("%0 = cnum":"=r"(core));
 if(!core&&((dbg&RU_C1_DBG_C0)||!(dbg&RU_C1_DBG_C1))){fm1_fault_c(f);return;}
 ru_cpu1_halt();
 ru_c1_faults++;
 ru_c1_online=0u;
 ru_c1_retired=RU_C1_FAULT;
 ru_c1_boot.barred=1u;ru_c1_boot.started=0u;
 if(!(FM1_DBG_WR_EN&1u))FM1_DBG_WR_EN=0xe7u;
 FM1_DBG_MSG_CLR=dbg&RU_C1_DBG_C1;
 RU_C1_SYNC();
 if(core)for(;;)__asm__ volatile("nop");
}
#endif
