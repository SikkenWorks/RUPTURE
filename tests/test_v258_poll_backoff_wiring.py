#!/usr/bin/env python3
"""Only the bounded CORE0-local mailbox poll spacing may change."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
src=(root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_cpu1_lab.h').read_text()
sha=lambda x:hashlib.sha256(x.encode()).hexdigest()
define='#define RU_C1_FX_POLL_NOP_COUNT 32u /* CORE0-local; no CPU1/shared SRAM access */\n'
current='''  /* CPU0 otherwise executes csync plus shared-SRAM mailbox reads as fast
   * as the bus allows. Thirty-two register-only NOPs between inconclusive
   * polls reduce speculative shared-bus traffic while CPU1 works. A known
   * failure is still retired IMMEDIATELY after its mailbox read; the same
   * wall-clock timeout, fault detection and exact PCM fallback remain. */
  for(uint8_t n=0u;n<RU_C1_FX_POLL_NOP_COUNT;n++)__asm__ volatile("nop");
'''
prior='''  /* CPU0 otherwise executes csync plus two shared-SRAM mailbox reads as
   * fast as the bus allows while CPU1 works. Sixteen register-only NOPs
   * lower the poll rate and intercore bus contention without postponing
   * any detected fault beyond this tiny bounded interval. No audio or FX
   * state, DMA address, timeout budget or completion rule is changed. */
  for(uint8_t n=0u;n<16u;n++)__asm__ volatile("nop");
'''
assert src.count(define)==1 and src.count(current)==1
before=src.replace(define,'').replace(current,prior).replace('V2.0.58','V2.0.57')
assert sha(before)=='c0beaedcb4605570f2ea3f2112c32f2d6e52214974803afca3b9164a29d751f1', 'Another CORE2 protocol detail changed'
assert '#define RU_C1_FX_TIMEOUT_TICKS 26000u' in src
assert 'if(done!=seq-1u||ru_c1_fx_box.bad){corrupt=1u;break;}' in src
assert 'ru_cpu1_retire(corrupt?RU_C1_BAD_OUTPUT:RU_C1_TIMEOUT)' in src
assert 'ru_c1_fx_avg_wait_ticks=ru_fx_wait_ema_next(' in src
# Synthetic poll timing: one event can be noticed at most 31 local NOPs
# after the preceding unsuccessful poll (not a measured FM1 bus latency).
for event in range(4097):
 delay=(-event)%32
 assert 0<=delay<=31
print('PASS V2.0.58: only CORE0 local NOP gap 16->32, retire/timeout/PCM untouched')
