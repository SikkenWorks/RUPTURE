#!/usr/bin/env python3
"""Diagnostic sampling must be limited to successful FX completion."""
from pathlib import Path

root=Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
worker=(native/'rupture_cpu1_lab.h').read_text()
ui=(native/'bringup.c').read_text()
header=(native/'rupture_core2_fx_wait_ema.h').read_text()

assert '#include "rupture_core2_fx_wait_ema.h"' in worker
assert 'static volatile uint32_t ru_c1_fx_max_wait_ticks,ru_c1_fx_avg_wait_ticks;' in worker
success=worker.split('if(reply==RU_FX_REPLY_COMPLETE){',1)[1].split('corrupt=1u;break;',1)[0]
assert success.count('ru_fx_wait_ema_next(')==1
assert success.index('ru_fx_wait_ema_next(')<success.index('ru_c1_fx_commits++')
assert 'ru_c1_fx_commits!=0u' in success
assert 'if(waited>ru_c1_fx_max_wait_ticks)' in success
failure=worker.split('if(!corrupt)ru_c1_fx_timeout++;',1)[1].split('#endif',1)[0]
assert 'ru_fx_wait_ema_next(' not in failure
assert 'ru_cpu1_retire(corrupt?RU_C1_BAD_OUTPUT:RU_C1_TIMEOUT)' in failure
assert 'ru_fx_safe_progress(' in failure
assert 'int32_t delta=(int32_t)measured-(int32_t)previous;' in header

assert 'label("FX US AVG",10,203' in ui
assert 'number(ru_c1_fx_avg_wait_ticks/24u,106,203)' in ui
assert 'label("MAX",151,203' in ui
assert 'number(ru_c1_fx_max_wait_ticks/24u,191,203)' in ui
assert 'label("FX TIMEOUT",10,222' in ui
assert 'label("FX JOBS",10,218' in ui
print('PASS V2.0.58: CORE2 FX average and maximum displayed, faults never sampled')
