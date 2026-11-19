#!/usr/bin/env python3
"""Second DSP core: FIR is an isolated shadow probe, never an audio dependency."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
legacy=root.parents[0]/'RUPTURE-FM1-V2.0.32'/'vendor/etch-v1.88-source/firmware/fm1-native'
for f in ('native_cxs_q15.c','native_fx.c','uac_fir.h'):
 if legacy.exists():
  assert (src/f).read_bytes()==(legacy/f).read_bytes(), f+' changed original audio'
# V243 deliberately adds Note OFF/overflow guards to both audio cores. The
# sound-generating CXS/FX algorithms above must remain byte-identical.
assert 'ru_mono_panel_reconcile(s,keys)' in (src/'audio_core.c').read_text()
# V238 adds read-only ALNK0 note-edge telemetry in audio_target.c. The
# independent V238 source-parity audit restores and hashes its V237 body.
boot=(src/'bringup.c').read_text()
lab=(src/'rupture_cpu1_lab.h').read_text()
fir=(src/'rupture_cpu1_fir_probe.h').read_text()
assert boot.index('#include "usb.c"')<boot.index('#include "rupture_cpu1_lab.h"')
assert 'ru_cpu1_fir_process(&ru_c1_box,seq)' in lab
assert 'ru_cpu1_fir_verify(&ru_c1_box,ru_c1_wait_seq)' in lab
assert '!uac.alt&&!native_seq_run&&!native_seq_recording' in boot
assert '!native_blend_audio_busy()&&!native_missed_streak' in boot
assert 'ru_c1_wait_kind' in lab and 'ru_c1_box.kind!=ru_c1_wait_kind' in lab
assert 'ru_c1_retired=reason;' in lab
assert 'ru_c1_fir_good++' in lab and 'number(ru_c1_fir_good,124,218)' in boot
assert 'UAC_FIR[ph]' in fir and 'UAC_FIR[UAC_FIR_L-1u-ph]' in fir
assert 'm->input[i]!=ru_cpu1_expected_input(seed,i)' in fir
assert 'm->output[i]!=packed' in fir
assert '#if ETCH_CPU1_LAB' in boot
print('PASS V2.0.33: CPU1 shadow FIR is isolated, independently checked and quiet-gated')
