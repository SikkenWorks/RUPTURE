#!/usr/bin/env python3
"""Realtime CPU1 FX exclusivity, fail-closed mailboxes and NOTE+CORE2 UI."""
from pathlib import Path
import hashlib

src=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native'
worker=(src/'rupture_cpu1_lab.h').read_text()
header=(src/'rupture_core2_fx_guard.h').read_text()
audio=(src/'audio_target.c').read_text()
stable=(src/'audio_core.c').read_text()
experimental=(src/'audio_core_core2.c').read_text()
ui=(src/'rupture_ui.c').read_text()
bringup=(src/'bringup.c').read_text()

start=worker.index('static inline void ru_cpu1_diag_service(')
end=worker.index('#if ETCH_CPU1_FX_LIVE_LAB\n/* Linked from audio_core',start)
service=worker[start:end]
assert 'if(ru_fx_probe_finished(ru_c1_good,ru_c1_fir_good))return;' in service
assert service.index('if(ru_fx_probe_finished(')<service.index('ru_c1_box.seq=seq;')
assert service.index('if(ru_c1_wait_seq){')<service.index('if(ru_fx_probe_finished(')
assert 'integrity>=128u&&fir>=1u' in header
assert 'progress==end' in header
assert 'if(!ru_c1_online)' in worker and 'ru_c1_fx_fallbacks++;' in worker
assert 'ru_c1_fx_box.progress' in worker
assert 'if(done!=seq-1u||ru_c1_fx_box.bad)' in worker
assert 'corrupt?RU_C1_BAD_OUTPUT:RU_C1_TIMEOUT' in worker
assert 'ru_cpu1_halt();' in worker
assert 'for(uint32_t f=finished;f<ETCH_NATIVE_FRAMES;f++)ru_fx_c2_process_one(f);' in experimental
for core in (stable,experimental):
    assert 'if(ru_note_panel_repairs<UINT32_MAX)++ru_note_panel_repairs;' in core

assert '++native_midi_panic_seen;' in audio
assert '++native_midi_on_seen;' in audio
assert '++native_midi_off_seen;' in audio
assert 'if(v->diag_view==4u)' in ui
assert 'READ ONLY / NOTE + CORE2' in ui
assert all(key in ui for key in ('KEY REPAIR','MIDI PANIC','MIDI ON','MIDI OFF',
                                'FX JOBS','FX FALLBACK','FX TIMEOUT','CORE2 ACTIVE'))
assert 'ru_diag_view+1u)%5u' in bringup
for field in ('diag_key_repairs','diag_midi_panics','diag_midi_on','diag_midi_off',
              'diag_fx_jobs','diag_fx_fallbacks','diag_fx_timeouts','diag_core_reason'):
    assert 'v.'+field+'=' in bringup

sha=lambda b:hashlib.sha256(b).hexdigest()
assert sha((src/'native_cxs_q15.c').read_bytes())=='86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f'
assert sha((src/'native_fx.c').read_bytes())=='d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a'
print('PASS V2.0.58: realtime FX core exclusive, partial commits validated, 5th diagnostic page wired')
