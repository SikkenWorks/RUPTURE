"""MIDI expression observes original queue; autosave OFF never writes profiles."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
bringup=(native/'bringup.c').read_text()
target=(native/'audio_target.c').read_text()
ui=(native/'rupture_ui.c').read_text()
header=(native/'rupture_midi_expression.h').read_text()

assert 'ru_midi_expr_event(&ru_midi_expr,status,note,velocity);' in target
assert target.index('etch_native_midi(&native_audio,status,note,velocity);') < target.index('ru_midi_expr_event(&ru_midi_expr,status,note,velocity);')
assert target.index('native_midi_drain(native_output_enabled?1u:0u);') < target.index('ru_touch_apply_profile(&c,&touch_profile,expr_level);')
assert 'ru_midi_expr_reset(&ru_midi_expr);' in target
assert 'ru_touch_midi_held=ru_midi_expr_held(&native_audio);' in target
assert 'fm1_in.notes|(ru_touch_midi_held?1u:0u)' in bringup
assert 'RU_TOUCH_MODE_VELOCITY' in (native/'rupture_touch_profile.h').read_text()
assert 'kind==0xa0u' in header and 'kind==0xd0u' in header
assert 'RU_TOUCH_MODE_VELOCITY' in ui and '"MIDI VELOCITY"' in ui
assert '"PRESS","DRIFT","RANDOM","VELOCITY"' in ui
assert '"SOFT","LINEAR","HARD","EXTRA HARD"' in ui
assert 'ru_touch_manual_commit=1u' in bringup
assert 'if(ru_autosave_enabled){ru_global_gain_dirty=1u;' in bringup
assert 'if((ru_autosave_enabled||ru_touch_manual_commit)&&' in bringup
assert 'if(!ru_touch_profile_dirty)ru_touch_manual_commit=0u;' in bringup
assert 'if(ru_autosave_enabled&&rupture_autosave_due' in bringup
for name,old in {
 'audio_core.c':'0e5588d24d41f69be5e3065acf4218795a4b55916f448a1b595d984fde7eb235',
 'audio_core_core2.c':'a7302c0c51fadfa695cee5fc2875cdaad37443a35bf957ee40f41f00fd9002bd',
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
}.items():
 assert hashlib.sha256((native/name).read_bytes()).hexdigest()==old,name
print('PASS V2.0.58 USB/TRS MIDI expression via ALNK, save authorization, native PCM unchanged')
