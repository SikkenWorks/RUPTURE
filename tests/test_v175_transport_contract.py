from pathlib import Path
R=Path(__file__).resolve().parents[1]
n=R/'vendor/etch-v1.88-source/firmware/fm1-native'
s=(n/'bringup.c').read_text()
a=s.index(' if(pressed&(1u<<12)){',s.index('static void ru_handle_input('))
b=s.index(' /* In live REC ALGORITHM',a)
transport=s[a:b]
play=transport.split(' if((pressed&(1u<<13)) && ru_page==RU_PAGE_SEQ){')[0]
record=transport.split(' if((pressed&(1u<<13)) && ru_page==RU_PAGE_SEQ){')[1]
assert 'native_seq_run=1u' not in record, 'REC still explicitly starts playback'
assert 'native_seq_recording^=1u' in record
assert 'else if(native_seq_recording && !native_seq_run)' in play
assert 'ru_countin_begin(&ru_countin)' in play
assert 'if(ru_countin.active)' in play and 'ru_countin_cancel(&ru_countin)' in play
assert 'if(ru_countin.ready)' in s
assert 'native_controls.seq_run=1u' in s
assert 'new_keys && !(native_seq_recording && !ru_live_running)' in s
assert 'ru_live_running=(uint8_t)(native_seq_run||ru_countin.force_seq)' in s
assert 'b==11u&&ru_countin.active?ru_countin_pulse' in s
assert 'ru_metronome_record_algorithm(&ru_metronome_volume,&alg,' in s
assert 'etch_v130_bpm(native_tempo,preset)' in s
app=(n/'audio_core.c').read_text()
assert 'ru_countin_sample(&ru_countin,&ru_metronome_click,c->tempo)' in app
assert 'perf_control.seq_run=1u' in app
assert 'ru_metronome_sample(&ru_metronome_click,1u,ru_metronome_volume)' in app
assert (n/'ota.c').read_bytes()==(Path('/mnt/data/rupture-v174-work/vendor/etch-v1.88-source/firmware/fm1-native/ota.c')).read_bytes()
assert (n/'native_project_codec.c').read_bytes()==(Path('/mnt/data/rupture-v174-work/vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c')).read_bytes()
for file in ['README.md','CHEAT-SHEET.html']:
 doc=(R/file).read_text()
 assert 'two-bar' in doc.lower() and 'PRESETS' in doc and 'ALGORITHM' in doc
help=(n/'rupture_help_cards.h').read_text()
assert 'REC COUNT IN' in help and 'RECORDING STARTS ON STEP 1' in help
assert 'REC THEN PLAY: 2 BAR COUNT' in (n/'rupture_tutorial.h').read_text()
assert '"V1.76","FM-1_968"' in (n/'rupture_ui.c').read_text()
print('PASS V1.75: REC arm, PLAY count-in, already-playing REC, cancellation, no countdown edits, dual UI+LED pulse, BPM/volume mapping, OTA/storage unchanged, documentation')
