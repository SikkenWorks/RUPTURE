#!/usr/bin/env python3
"""Exercise source-level connections of read-only note/DMA diagnostics."""
from pathlib import Path

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
audio=(src/'audio_target.c').read_text()
ui=(src/'rupture_ui.c').read_text()
boot=(src/'bringup.c').read_text()
paint=(src/'rupture_note_paint_budget.h').read_text()

assert '#include "rupture_note_dma_meter.h"' in audio
assert audio.count('ru_note_dma_record(&native_note_dma,nd_edges,render_elapsed,')==2
assert 'ru_note_dma_edge(nd_old_keys,native_audio.keys,nd_old_serial' in audio
assert 'native_output_enabled?' in audio
assert '(uint32_t)(fm1_ticks()-start),1u);' in audio
assert 'measure.wall_ticks,measure.late);' in audio
assert 'v->diag_view==3u' in ui and 'READ ONLY / NOTE EDGES' in ui
assert all(value in ui for value in ('NOTE ON','NOTE OFF','ON LATES','OFF LATES','ON DSP MAX'))
assert 'ru_diag_view+1u)%5u' in boot
assert all('v.diag_note_'+s in boot for s in ('on=','off=','on_late=','off_late=','on_render=','off_render='))
assert 'ru_note_paint_allow_fx_sustain(' in boot
assert 'native_audio.mono_count || fm1_in.notes' in boot
assert '(uint32_t)(now-last_paint)<120u' in paint
assert 'return ru_note_paint_allow_fx(s,now,last_paint,user_control,' in paint
assert 'RUPTURE V2.0.38' in (src/'build.py').read_text()
print('PASS V2.0.38 note-on/off DMA diagnostics and 3+ FX UI guard wiring')
