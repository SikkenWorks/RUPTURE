#!/usr/bin/env python3
"""Check the active-CORE2-only DMA retirement handshake and unchanged DSP."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
target=(src/'audio_target.c').read_text()
guard=(src/'rupture_core2_pressure_guard.h').read_text()
worker=(src/'rupture_cpu1_lab.h').read_text()
assert 'jobs_before!=jobs_after' in guard
assert 'ru_core2_pressure_should_retire(ru_c1_online' in target
assert 'ru_fx_probe_finished(ru_c1_good,ru_c1_fir_good)' in target
assert 'core2_jobs_before,ru_c1_fx_commits' in target
assert target.count('native_budget_late(core2_jobs_before);')==2
assert 'const uint32_t core2_jobs_before=ru_c1_fx_commits;' in target
assert 'ru_cpu1_retire(RU_C1_PRESSURE);' in target
assert worker.index('ru_cpu1_halt();')<worker.index('ru_c1_online=0u;')
for name,expected in {
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
}.items():
 assert hashlib.sha256((src/name).read_bytes()).hexdigest()==expected
print('PASS V2.0.58: DMA LATE after real CORE2 FX retires only worker, unchanged CXS/FX source')
