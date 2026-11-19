#!/usr/bin/env python3
"""V239 scope guard must never gate PCM render, output DMA or USB MIDI."""
from pathlib import Path

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
audio=(src/'audio_target.c').read_text()
guard=(src/'rupture_note_scope_guard.h').read_text()
boot=(src/'bringup.c').read_text()

assert '#include "rupture_note_scope_guard.h"' in audio
assert '#include "rupture_note_paint_budget.h"' in boot
assert boot.index('#include "rupture_note_paint_budget.h"')<boot.index('#include "audio_target.c"')
assert 'ru_note_fx_active_count(&c.fx_chain,c.fx_chain_active)' in audio
assert 'ru_note_scope_capture_allow(&native_note_scope_guard,' in audio
assert 'if(normal_scope&&!prepare_scope&&native_scope_deferred<UINT32_MAX)' in audio
assert 'native_note_scope_guard.quiet_halves=0u' in audio
assert audio.index('etch_native_render_stereo(')<audio.index('ru_note_scope_capture_allow(')
assert audio.index('ru_note_scope_capture_allow(')<audio.index('if(prepare_scope)native_mono[i]=')
assert 'if(uac.feed)uac_tap(dst,ETCH_NATIVE_FRAMES);' in audio
assert 'fm1_audio_ack_half();++native_halves;' in audio
assert 'native_midi_drain(native_output_enabled?1u:0u)' in audio
assert 'RU_SCOPE_FX_QUIET_HALVES=24u' in guard
assert 'previous_slack<=half_budget/5u' in guard
assert 'ordinary_scope_allow&&!quiet&&!low_slack' in guard
assert 'V2.0.43' in (src/'build.py').read_text()
print('PASS V2.0.39: only optional scope deferred; audio, USB and MIDI untouched')
