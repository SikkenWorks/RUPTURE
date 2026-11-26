#!/usr/bin/env python3
"""CORE2 poll backoff never weakens completion, timeout or fail-closed rules."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
code=(native/'rupture_cpu1_lab.h').read_text()
collect=code.split('uint32_t ru_cpu1_fx_collect(void){',1)[1].split('\n}\n',1)[0]
assert 'while((uint32_t)(fm1_ticks()-ru_c1_fx_at)<RU_C1_FX_TIMEOUT_TICKS)' in collect
assert 'RU_C1_SYNC();' in collect
assert 'if(done==seq)' in collect
assert 'if(reply==RU_FX_REPLY_COMPLETE)' in collect
assert 'if(done!=seq-1u||ru_c1_fx_box.bad){corrupt=1u;break;}' in collect
assert '#define RU_C1_FX_POLL_NOP_COUNT 32u' in code
assert 'for(uint8_t n=0u;n<RU_C1_FX_POLL_NOP_COUNT;n++)__asm__ volatile("nop");' in collect
assert collect.index('if(done!=seq-1u||ru_c1_fx_box.bad)') < collect.index('for(uint8_t n=0u;n<RU_C1_FX_POLL_NOP_COUNT;n++)')
assert 'ru_cpu1_retire(corrupt?RU_C1_BAD_OUTPUT:RU_C1_TIMEOUT);' in collect
assert 'return ru_fx_safe_progress(ru_c1_fx_box.begin,end,ru_c1_fx_box.progress);' in collect
assert '#define RU_C1_FX_TIMEOUT_TICKS 26000u' in code
assert 'ru_cpu1_fx_publish(ETCH_NATIVE_FRAMES);' in (native/'audio_core_core2.c').read_text()
print('PASS V2.0.58 CORE2 FX mailbox: 32-NOP local relax; timeout/fault/fallback unchanged')
