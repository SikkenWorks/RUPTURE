from pathlib import Path
R=Path(__file__).resolve().parents[1]
N=R/'vendor/etch-v1.88-source/firmware/fm1-native'
source=(N/'bringup.c').read_text()
assert source.count('ru_metronome_record_algorithm(&ru_metronome_volume,&alg,')==1
assert '((native_seq_recording && native_seq_run) ||' in source
assert 'ru_metronome_volume+preset' not in source
assert source.count('etch_v130_bpm(native_tempo,turns)')==1
assert 'if(preset && !(native_seq_recording && native_seq_run) &&' in source
assert '!native_draw && native_page==ETCH_UI_PRESETS' in source
help=(N/'rupture_help_cards.h').read_text()
tour=(N/'rupture_tutorial.h').read_text()
ui=(N/'rupture_ui.c').read_text()
assert 'PRESETS: ASSIGNED VALUE' in help and 'ALGORITHM: CLICK VOL 0-100' in help
assert 'ALG EXTENDS HOLD WHEN NOT REC' in help and 'HOLD SEQ TURN PRESETS' in help
assert 'SEQ+PRESETS: SHIFT ALL STEPS' in tour and 'REC HOLD KEY FOR SUSTAIN' in tour
assert 'PRESETS DEFAULT BPM / ALG CLICK' in tour
assert 'SEQ+K4 SLIDE / SAVE SEQUENCES' in ui
assert 'line_pitch=lesson->page==RU_PAGE_SEQ?8:11' in ui
readme=(R/'README.md').read_text()
cheat=(R/'CHEAT-SHEET.html').read_text()
assert '**PRESETS controls its globally assigned parameter**' in readme
assert '**ALGORITHM**' in readme and 'While live recording is armed or active' in readme
assert 'PRESETS adjusts its globally assigned parameter (BPM by default)' in cheat
assert 'ALGORITHM changes metronome volume 0–100' in cheat
print('PASS V1.75 REC metronome, SEQ shift, assignable PRESETS mapping, tutorial/HELP/docs')
