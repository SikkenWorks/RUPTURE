#!/usr/bin/env python3
"""Audit three independent Note OFF safety nets in BOTH firmware variants."""
from pathlib import Path
import hashlib

src=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native'
target=(src/'audio_target.c').read_text()
guard=(src/'rupture_note_release_guard.h').read_text()
assert 'ru_midi_capture_attack(native_group_capture_active,status,velocity)' in target
assert 'etch_native_midi(&native_audio,status,note,velocity);' in target
assert 'if(native_group_capture_active && ((status&0xf0u)==0x80u' not in target
assert '(status&0xf0u)==0x90u && velocity!=0u' in guard
assert 'm.id<27u && !(keys&(1u<<m.id))' in guard
assert 'm.id>=128u&&m.id<2176u' in guard

for variant in ('audio_core.c','audio_core_core2.c'):
    core=(src/variant).read_text()
    assert core.count('if(ru_mono_panel_reconcile(s,keys)){')==1,variant
    assert 'if(ru_note_panel_repairs<UINT32_MAX)++ru_note_panel_repairs;' in core
    assert core.count('s->sustain=0u;ru_mono_midi_panic(s);')==1,variant
    assert core.count('mono_resume(s); /* An interrupted MIDI stream')==1,variant
    assert core.count('if(s->mono_count>ETCH_MONO_STACK)')>=3,variant
    for entry,end in (('static void mono_drop_channel(', 'static void mono_perf_clear('),
                      ('static void mono_perf_clear(', 'static uint8_t mono_perf_tick(')):
        body=core.split(entry,1)[1].split(end,1)[0]
        check='if(s->mono_count>ETCH_MONO_STACK){s->mono_count=0u;mono_resume(s);return;}'
        assert body.count(check)==1 and body.index(check)<body.index('for('),variant

sha=lambda b:hashlib.sha256(b).hexdigest()
assert sha((src/'native_cxs_q15.c').read_bytes())=='86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f'
assert sha((src/'native_fx.c').read_bytes())=='d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a'
print('PASS V2.0.58: Note OFF, panel levels, MIDI panic and bounded CC120/CC121/SEQ-clear on both cores')
