#!/usr/bin/env python3
"""Package V2.0.43: Note OFF recovery on both cores, same RGB565 branding."""
from pathlib import Path
import base64
import hashlib
import json
import re
import shutil
import sys

ROOT = Path(__file__).resolve().parents[1]
NATIVE = ROOT / 'vendor/etch-v1.88-source'
APP_DIR = NATIVE / 'build/fm1-native-storage-updater-uac48-p05'
APP = (APP_DIR / 'app.bin').read_bytes()
sha = lambda data: hashlib.sha256(data).hexdigest()
sys.path.insert(0, str(NATIVE / 'tools'))
import fm1_fwsc as fw

trusted = ROOT / 'reference/firmware/RUPTURE-V1.86.1.fwsc'
# A source-only checkout contains the pinned reference firmware but not
# necessarily the prior release FWSC. Both carry the same audited boot/OTA.
previous = ROOT / 'RUPTURE-V2.0.27.fwsc'
if not previous.is_file():
    previous = ROOT / 'reference/firmware/RUPTURE-V2.0.9.fwsc'
assert fw.verify_fwsc(trusted)[1] == []
assert fw.verify_fwsc(previous)[1] == []
assert not fw.validate_app_binary(APP)
assert b'V2.0.43' in APP
assets = fw.extract_stock_assets(trusted)
assert assets['product'] == 'FM-1_968'
prev_entry = {e['name']: e['data'] for e in fw.parse_ufw(previous.read_bytes())['entries']}
flash = fw.build_flash_image_from_assets(
    APP, assets['uboot'], assets['cfg_tool'], assets['eq_cfg'], assets['key'],
    isd_config=assets['isd_config'], flash_header_payload=assets['flash_header_payload'])
assert len(flash) == len(prev_entry['flash.bin']) == 0x93000
assert flash[:0x4000] == prev_entry['flash.bin'][:0x4000], 'Protected FM1 bootheader must not change'
assert assets['ota'] == prev_entry['ota.bin'], 'Trusted OTA loader must not change'
pkg = fw.build_fwsc(flash, assets['ota'], 'FM-1_968')
target = ROOT / 'RUPTURE-V2.0.43-STABLE.fwsc'
target.write_bytes(pkg)
assert fw.verify_fwsc(target)[1] == []
entries = {e['name']: e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert entries['ota.bin'] == prev_entry['ota.bin']
assert entries['flash.bin'][:0x4000] == prev_entry['flash.bin'][:0x4000]
assert entries['flash.bin'] == flash

for name in ('app.bin', 'app.bin.audit.json', 'native-build-options.json'):
    shutil.copy2(APP_DIR / name, ROOT / 'native-build' / name)
options = json.loads((ROOT / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V2.0.43'
assert options['app_sha256'] == sha(APP)
assert options['ota_writes'] and options['storage_writes']
LAB_DIR = NATIVE / 'build/fm1-native-storage-updater-uac48-p05-c1lab'
lab_options = json.loads((LAB_DIR / 'native-build-options.json').read_text())
assert lab_options['cpu1_ram_integrity_lab_enabled'] and lab_options['cpu1_shadow_fir_probe_enabled']
assert not lab_options['cpu1_audio_offload_enabled']
assert lab_options['hardware_verified'] is False
assert lab_options['app_sha256'] == sha((LAB_DIR / 'app.bin').read_bytes())
for name,source in (('cpu1-lab-app.bin','app.bin'),
                    ('cpu1-lab-app.bin.audit.json','app.bin.audit.json'),
                    ('cpu1-lab-build-options.json','native-build-options.json')):
    shutil.copy2(LAB_DIR / source, ROOT / 'native-build' / name)
LIVE_DIR = NATIVE / 'build/fm1-native-storage-updater-uac48-p05-c1fx'
live_options = json.loads((LIVE_DIR / 'native-build-options.json').read_text())
assert live_options['cpu1_full_fx_live_lab_enabled'] and live_options['cpu1_audio_offload_enabled']
assert not live_options['cpu1_live_usb_fir_lab_enabled'], 'USB FIR must stay on core0 while core2 owns FX'
assert live_options['cpu1_fx_chunk_frames'] == 64
assert not live_options['hardware_verified']
assert live_options['app_sha256'] == sha((LIVE_DIR / 'app.bin').read_bytes())
LIVE_APP=(LIVE_DIR/'app.bin').read_bytes()
assert LIVE_APP!=APP and b'V2.0.43' in LIVE_APP
assert not fw.validate_app_binary(LIVE_APP)
live_flash=fw.build_flash_image_from_assets(
    LIVE_APP,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],
    isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert len(live_flash)==len(flash)==0x93000
assert live_flash[:0x4000]==flash[:0x4000]
live_pkg=fw.build_fwsc(live_flash,assets['ota'],'FM-1_968')
live_target=ROOT/'RUPTURE-V2.0.43-CORE2-EXPERIMENTAL.fwsc'
live_target.write_bytes(live_pkg)
assert fw.verify_fwsc(live_target)[1]==[]
live_entries={e['name']:e['data'] for e in fw.parse_ufw(live_pkg)['entries']}
assert live_entries['ota.bin']==entries['ota.bin']
assert live_entries['flash.bin']==live_flash
assert live_entries['flash.bin'][:0x4000]==flash[:0x4000]
for name,source in (('cpu1-fx-app.bin','app.bin'),
                    ('cpu1-fx-app.bin.audit.json','app.bin.audit.json'),
                    ('cpu1-fx-build-options.json','native-build-options.json')):
    shutil.copy2(LIVE_DIR / source, ROOT / 'native-build' / name)

manifest_path = ROOT / 'RELEASE-MANIFEST.json'
manifest = json.loads(manifest_path.read_text())
manifest.update(
    version='2.0.43', firmware_file=target.name,
    firmware_bytes=len(pkg), firmware_sha256=sha(pkg),
    experimental_firmware_file=live_target.name,
    experimental_firmware_bytes=len(live_pkg),
    experimental_firmware_sha256=sha(live_pkg),
    experimental_app_bytes=len(LIVE_APP),
    experimental_app_sha256=sha(LIVE_APP),
    app_bytes=len(APP), app_sha256=sha(APP),
    physical_fm1_verified=False, hardware_verified=False, release_ready=False,
    release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',
    release_feature='V2.0.43: fixed MIDI Note OFF swallowed by GROUPS capture; independent mono release safeguards',
    release_note='V2.0.43 fixes a reproducible hung-note cause: GROUPS MIDI learning previously '
                 'discarded Note OFFs for already sounding notes. Capture now accepts only '
                 'real Note ON attacks and always forwards Note OFF/velocity-zero Note ON. '
                 'Both stable and core2 synth implementations reconcile the physical-key '
                 'mono stack with actual held contacts once per 128-frame PCM block, '
                 'and MIDI overflow panic drops only MIDI notes while retaining physical '
                 'and sequencer voices. No fixed timeout truncates genuinely held notes '
                 'or sustained MIDI. Stack counts are bounded before array access. '
                 'CXS and the FX source equations remain byte-identical to V2.0.42. '
                 'All fixes are HOST TESTED but NOT physically FM1 verified. '
                 'The 5.16-second boot intro still renders the actual uploaded '
                 '1402x1122 PNG portrait as 57600 exact 8-bit grayscale pixels, '
                 'transparent margins cropped and nearest-neighbor scaled. '
                 'No additional 1-bit or Bayer dithering: the existing RGB565 '
                 'LCD DMA stripe displays real gray levels during the fade. '
                 'The text/word-only RUPTURE flicker remains '
                 'unchanged. STABLE keeps the original full FX chain on CPU0 '
                 'and CSX/CXS/GROWL/SUB synthesis unchanged. The separate '
                 'CORE2 EXPERIMENTAL binary now executes ALL ordered FX slots '
                 'on CPU1 in 64-frame lanes: CPU0 synthesizes the next '
                 '64 samples concurrently, CPU1 operates the existing '
                 'single effect-tank state, and CPU0 applies gain/metronome '
                 'and publishes DMA. The CXS path and saved patches do not '
                 'change; no frame buffering or new voice is added. '
                 'CPU1 must qualify 128 integrity probes and one FIR probe '
                 'before DSP offload activates. If a lane fails or exceeds '
                 'the bounded wait, CPU1 is halted and CPU0 resumes the '
                 'unfinished FX samples. A hard fault mid-sample is not '
                 'perfectly recoverable. 48k USB FIR stays on CPU0 to '
                 'prevent simultaneous core2 jobs. Physical FM1 hardware '
                 'is UNVERIFIED: DMA lateness, isolated shared-SRAM faults, '
                 'crashes and possible recovery remain real risks. '
                 'Never flash CORE2 EXPERIMENTAL without a known recovery '
                 'route; benchmark with actual hardware before release.',
)
manifest['fx_chain_glide_v190'] = {
    'fx_slots': 8,
    'fx_processors': ['REVERB', 'DELAY', 'CHORUS', 'MOSFET', 'COMP', 'TILT', 'FUZZ', 'SAND'],
    'new_project_fx_slots_empty': True,
    'new_fx_enabled_by_default': True,
    'fx_reorder_changes_pcm': True,
    'fx_duplicate_instances_allowed': True,
    'glide_time_max_ms': 2000,
    'glide_modes': ['OFF', 'LEGATO', 'ALWAYS'],
    'glide_curves': ['LINEAR', 'EXP'],
    'step_slide': True,
    'project_codec_version': 35,
    'legacy_routing_preserved_until_edit': True,
    'trusted_loader_sha256': sha(entries['ota.bin']),
    'protected_bootheader_sha256': sha(flash[:0x4000]),
    'physical_fm1_verified': False,
}
manifest['realtime_v191'] = {
    'zero_slot_fast_dry': True,
    'bounded_old_tank_flush': True,
    'mosfet_division_free_runtime': True,
    'mosfet_transfer_rom_points': 257,
    'reuses_v189_four_fx_fastpath_when_safe': True,
    'fx_sel_opens_parameters': True,
    'fx_k4_on_off': True,
    'no_additional_audio_voices': True,
    'hardware_dma_margin_verified': False,
    'eight_fx_not_heavier_than_v189_verified': False,
}
manifest['fx_expansion_v192'] = {
    'available_types': 10,
    'simultaneous_slots': 8,
    'new_types': ['DISTORTION', 'WAH WAH'],
    'distortion_params': ['DRIVE', 'BIAS', 'TONE', 'MIX'],
    'wah_params': ['SENS', 'SPEED', 'RESONANCE', 'MIX'],
    'project_codec_version': 35,
    'reuses_existing_persisted_fx_fields': True,
    'no_new_pcm_rings': True,
    'new_fx_cpu_idle_when_not_inserted': True,
    'backward_firmware_can_read_new_type_ids': False,
    'real_fm1_live_verified': False,
}
manifest['fx_repeat_v193'] = {
    'simultaneous_slots': 8,
    'available_types': 10,
    'duplicate_type_in_slots_allowed': True,
    'slot_bypass_ramps_independent': True,
    'fx_type_parameters_shared': True,
    'chorus_delay_reverb_single_shared_tank': True,
    'time_fx_tank_updates_per_pcm_frame_max': 1,
    'shared_wah_comp_tone_state_updates_per_frame_max': 1,
    'new_pcm_buffers': 0,
    'unchanged_unique_fx_pcm_oracle': True,
    'glide_heading': 'PORTAMENTO',
    'glide_old_subtitle_removed': True,
    'hardware_cpu_margin_verified': False,
    'fully_independent_repeated_fx_implemented': False,
    'v192_can_load_projects_with_duplicate_fx_types': False,
}
limitation = (
    'V1.93 repeated effects share one parameter set per type. Repeat Chorus, '
    'Delay and Reverb inserts share one delay/chorus/reverb tank rather than '
    'being independent copies; this avoids additional SRAM but is not the '
    'same sound as two independent hardware effects. Each active insertion '
    'still adds processing; maximum FX CPU/DMA margin requires an FM1 test. '
    'V1.92 cannot decode PJT35 projects with repeated effect IDs.')
manifest['limitations'] = [msg for msg in manifest.get('limitations', [])
                          if not msg.startswith('V1.93 repeated effects share') and
                             not msg.startswith('V1.94 independent slot') and
                             not msg.startswith('V1.95 optimization')]
manifest['limitations'].append(
 'V1.95 optimization does not eliminate the cost of additional active FX. '
 'Delay and Chorus share one PCM history per type; multiple Reverb slots '
 'share one tank and cannot provide fully independent reverb decay/topology. '
 'FM1 hardware performance, sustained FX chain, and live stability '
 'are not yet verified. The older V1.93 cannot open newly saved PJT35 '
 'records with the V1.94 parameter extension.')
manifest['fx_independent_v194'] = {
    'slot_parameter_banks': 8,
    'parameters_per_bank': 4,
    'independent_small_effect_state': ['MOSFET', 'COMP', 'TILT', 'FUZZ', 'SAND', 'DISTORTION', 'WAH'],
    'individual_chorus_modulation_and_mix': True,
    'individual_delay_taps_and_mix': True,
    'reverb_wet_mix_independent': True,
    'reverb_decay_topology_independent': False,
    'new_pcm_ring_bytes': 0,
    'exact_single_fx_pcm_parity_test': True,
    'legacy_pjt35_readable': True,
    'v193_firmware_reads_new_extended_projects': False,
    'additional_cpu_when_multiple_effect_instances_active': True,
    'hardware_live_verified': False,
}
manifest['realtime_optimization_v195'] = {
    'golden_oracle_version': '1.94',
    'golden_oracle_effect_types': 10,
    'golden_oracle_instance_counts': [1, 8],
    'golden_oracle_frames_per_case': 35000,
    'golden_oracle_sample_exact': True,
    'legacy_parameter_cache_skipped_when_unused': False,
    'legacy_parameter_cache_kept_warm_for_live_hidden_modulation': True,
    'independent_wah_control_coefficients_cached': True,
    'additional_small_control_state_bytes_estimate': 96,
    'wah_alpha_exact_integer_reduction': True,
    'pcm_ring_buffer_delta_bytes': 0,
    'fx_slot_or_feature_removed': False,
    'physical_fm1_cpu_margin_verified': False,
}
manifest['scale_strict_v196'] = {
    'ui_modes': ['OFF', 'GUIDE', 'STRICT', 'LOCK'],
    'stored_mode_values': {'OFF': 0, 'GUIDE': 1, 'STRICT': 3, 'LOCK': 2},
    'out_of_scale_held_keys_silent': True,
    'out_of_scale_midi_note_on_filtered': True,
    'note_off_unfiltered': True,
    'out_of_scale_performance_leds_off': True,
    'sequencer_live_record_filters_forbidden_keys': True,
    'new_per_sample_pcm_dsp_operations': 0,
    'mask_calculated_at_most_twice_per_pcm_block_in_strict': True,
    'legacy_lock_2_preserved': True,
    'hardware_key_matrix_tested': False,
}
manifest['glide_tunnel_v197'] = {
    'width_pixels': 224,
    'height_pixels': 30,
    'frames': 12,
    'bits_per_pixel': 1,
    'sprite_rom_bytes': 10080,
    'camera_jitter_max_pixels': 2,
    'shake_tick_ms': 85,
    'forward_keyframe_every_ticks': 3,
    'draw_only_when_glide_visible': True,
    'extra_pcm_operations_per_sample': 0,
    'audio_core_changed': False,
    'physical_fm1_verified': False,
}
manifest['glide_save_v198'] = {
    'stored_per_preset': True,
    'glide_parameters': ['TIME_MS', 'MODE', 'CURVE', 'ENV_RETRIG'],
    'per_preset_storage_bytes_unchanged': True,
    'project_codec_version': 35,
    'manual_and_autosave_share_snapshot_and_codec': True,
    'edits_staged_before_autosave': True,
    'rapid_preset_switch_preserves_staged_glide': True,
    'legacy_project_uses_in_session_staging_until_first_save': True,
    'endpoint_knob_events_do_not_mark_glide_dirty': True,
    'flash_writes_on_glide_knob_event': False,
    'additional_audio_pcm_work': 0,
    'physical_fm1_verified': False,
}
manifest['glide_visual_and_guide_v199'] = {
    'tunnel_moves_only_during_held_voice': True,
    'notes_can_arrive_from': ['KEYS', 'USB_MIDI', 'TRS_MIDI', 'ARPEGGIATOR', 'SEQUENCER'],
    'out_of_scale_silent_keys_do_not_animate': True,
    'idle_first_frame_and_shake_zero': True,
    'last_voice_release_resets_scene': True,
    'shake_tick_ms': 85,
    'frame_advance_every_shake_ticks': 3,
    'guide_display_modes': ['OFF', 'GUIDE', 'STRICT', 'LOCK'],
    'guide_stored_modes': [0, 1, 3, 2],
    'audio_dsp_changed': False,
    'project_codec_changed': False,
    'physical_fm1_verified': False,
}
manifest['boot_signal_breach_v200'] = {
    'frames': 12,
    'width_pixels': 240,
    'height_pixels': 240,
    'bits_per_pixel': 1,
    'source_uncompressed_rom_bytes': 86400,
    'lossless_rle_rom_bytes': 46340,
    'uses_existing_ui_framebuffer': True,
    'duration_ms': 1880,
    'last_frame_hold_ms': 800,
    'audio_core_changed': False,
    'ota_loader_changed': False,
    'physical_fm1_verified': False,
}
manifest['boot_signal_breach_smooth_v201'] = {
    'frames': 23,
    'exact_source_keyframes': 12,
    'ordered_dither_intermediate_frames': 11,
    'width_pixels': 240,
    'height_pixels': 240,
    'bits_per_pixel': 1,
    'compressed_rom_bytes': 96425,
    'uses_existing_ui_framebuffer': True,
    'cadence_ms': 50,
    'duration_ms': 1880,
    'last_frame_hold_ms': 780,
    'glide_autosave_uses_existing_preset_snapshot': True,
    'audio_core_changed': False,
    'ota_loader_changed': False,
    'physical_fm1_verified': False,
}
manifest['boot_and_subosc_fear_v202'] = {
    'boot_frame_count': 23,
    'boot_fast_frame_cadence_ms': 35,
    'boot_last_frame_hold_ms': 1000,
    'boot_nominal_duration_ms': 1770,
    'sub_oscillator_pages': [1, 2],
    'frames_per_oscillator': 16,
    'sprite_size_pixels': [224, 30],
    'bits_per_pixel': 1,
    'original_sprite_rom_bytes': 26880,
    'source_pixels_changed': False,
    'idle_frame_index': 0,
    'turn_frame_indices': [1, 11],
    'eye_tremor_loop_indices': [12, 15],
    'note_gated': True,
    'note_off_resets_to_idle': True,
    'extra_audio_dsp': False,
    'project_codec_changed': False,
    'trusted_ota_loader_changed': False,
    'physical_fm1_verified': False,
}
manifest['subosc1_stare_v203'] = {
    'turn_frames': [1, 11],
    'held_forward_stare_frame_index': 11,
    'held_forward_stare_source_file': 'frame-12.png',
    'sustain_face_frames_advance': False,
    'camera_shake_max_pixels': 1,
    'shake_step_ms': 75,
    'note_off_resets_to_idle': True,
    'subosc2_original_face_loop_unchanged': True,
    'audio_dsp_changed': False,
    'glide_and_autosave_changed': False,
    'trusted_ota_loader_changed': False,
    'physical_fm1_verified': False,
}
manifest['audio_fx_glide_v204'] = {
    'bend_pitch_slew_sample_rate': True,
    'bend_slew_independent_of_glide_enabled': True,
    'glide_curves': ['LINEAR', 'EXP', 'LOG', 'SOFT', 'FAST', 'FIRM'],
    'glide_codec_bytes_per_preset_unchanged': True,
    'fx_browser_order': ['REVERB','DELAY','CHORUS','WAH WAH',
                         'MOSFET','DISTORTION','OCTAVE FUZZ','SANDPAPER',
                         'COMPRESSOR','TILT EQ'],
    'persisted_fx_ids_unchanged': True,
    'distortion': 'tight bass + high gain asymmetric hard clip + tone',
    'room_reverb': 'less damped four-line FDN',
    'hall_reverb': 'allpass corrected, five-tap feedback, brighter decay',
    'spring_reverb_unchanged': True,
    'additional_pcm_delay_buffers': 0,
    'sub_pcm_sample_exact_under_regression': True,
    'sub_pitch_and_blend_exact_value_caches': True,
    'cxs_segment_comparisons_before_after': [7, 3],
    'dma_lates_zero_on_physical_fm1_verified': False,
    'subjective_listening_on_physical_fm1_verified': False,
    'physical_fm1_verified': False,
}
manifest['preset_assignment_and_dma_v205'] = {
    'sequencer_populated_and_empty_layout_tested': True,
    'preset_assignment_target_count': 94,
    'categories': 9,
    'scale_guide_root_assignable': True,
    'default_assignment': 'TEMPO / BPM',
    'saved_independent_of_project': True,
    'global_nor_crc_journal': True,
    'retained_on_project_and_catalog_rewrite': True,
    'retained_on_global_settings_compaction': True,
    'torn_write_tested': True,
    'status_overlay_phases': ['WAITING', 'SAVING', 'SAVED', 'FAILED'],
    'note_on_initial_cxs_exchange_precomputed': True,
    'original_pcm_sample_exact_count': 262144,
    'physical_fm1_dma_lates_zero_verified': False,
    'hardware_verified': False,
}
manifest['preset_popup_and_led_wave_v206'] = {
    'physical_encoder_id': 6,
    'physical_encoder_gpio': 'PB7 via FM1 matrix row 5',
    'assigned_editor_path': 'ru_adjust(page, control, detents)',
    'offscreen_parameter_value_popup': True,
    'scale_guide_root_note_name': True,
    'popup_hold_ms': 1550,
    'popup_overlapping_text_prevented': True,
    'global_assignment_persistence_unchanged': True,
    'led_count': 41,
    'led_keyboard_count': 27,
    'led_function_count': 14,
    'led_wave_step_ms': 27,
    'led_wave_tail_ms': 162,
    'led_wave_total_ms': 1269,
    'physical_input_cancels_wave': True,
    'audio_interrupt_changes': False,
    'scan_interrupt_changes': False,
    'audio_engine_changes': False,
    'physical_fm1_verified': False,
}
manifest['preset_runtime_input_fix_v207'] = {
    'root_cause': 'RUPTURE main loop continued before reaching V2.0.6 PRESETS handler',
    'input_handled_before_rupture_continue': True,
    'physical_encoder_id': 6,
    'parameter_editor_called_on_detents': True,
    'scale_guide_root_assignment_reachable': True,
    'parameter_popup_reachable': True,
    'seq_plus_preset_pattern_rotate_reachable': True,
    'assignment_picker_guarded': True,
    'saver_wakes_on_preset_rotation': True,
    'native_dispatch_tested': True,
    'audio_engine_changed': False,
    'ota_loader_changed': False,
    'physical_fm1_verified': False,
}
manifest.pop('realtime_pcm_exact_v208', None)  # rolled back after hardware regression report
manifest['realtime_cpu_recovery_v209'] = {
    'v208_cxs_exchange_optimization_reverted': True,
    'v208_room_unroll_and_bypass_fade_reverted': True,
    'cxs_original_v207_assembly_restored': True,
    'fx_reverb_feedback_and_tails_unchanged': True,
    'reverb_ordered_q15_mix_guard_checks_removed': True,
    'reverb_mixed_pcm_golden_cases': 6,
    'reverb_mix_isolated_pi32v2_code_bytes_before_after': [66, 18],
    'sub_pitch_increment_old_new_pcm_equal': True,
    'sub_pitch_increment_parity_cases': 2154880,
    'sub_pitch_isolated_pi32v2_code_bytes_before_after': [202, 106],
    'sub_pitch_exact_per_sample_glide_detune_drift': True,
    'reverb_types': ['SPRING','HALL','ROOM'],
    'eight_fx_slots_unchanged': True,
    'additional_pcm_rings': 0,
    'effect_count_reduced': False,
    'sample_rate_and_oversampling_unchanged': True,
    'stored_project_codec_unchanged': True,
    'hardware_dma_late_zero_verified': False,
    'physical_fm1_verified': False,
}
manifest['channel_eq_realtime_v210'] = {
    'standalone_eq_does_not_dispatch_empty_fx_rack': True,
    'all_four_saved_eq_gains_supported': True,
    'original_four_biquad_coefficient_math_unchanged': True,
    'original_wet_soft_knee_applied_consistently': True,
    'old_wet_buffer_cleanup_bounded_to_one_sample_index': True,
    'eq_scenario_count': 8,
    'eq_default_and_saved_knob_pcm_samples_compared': 6400000,
    'reverb_to_eq_transition_pcm_matches_v209': True,
    'dual_sub_low_eq_hall_pcm_stress': True,
    'no_new_pcm_buffers': True,
    'project_codec_unchanged': True,
    'hardware_dma_late_zero_verified': False,
    'physical_fm1_verified': False,
}
manifest['reference_growl_main_v211'] = {
    'reference': 'User-supplied Seaboard Block video, 2026-11-16',
    'reference_note_hz': 36.7,
    'dominant_harmonics': [2, 3, 4],
    'audio_samples_embedded': False,
    'former_main_saw_storage_bit_preserved': True,
    'mode_label': 'GROWL',
    'wavetables': 7,
    'table_length': 256,
    'control_labels_unchanged': True,
    'sub_oscillator_saw_unchanged': True,
    'cxs_pcm_unchanged': True,
    'new_runtime_ram_bytes': 8,
    'floating_point_in_pcm': False,
    'new_pcm_rings': 0,
    'physical_fm1_verified': False,
    'hardware_dma_margin_verified': False,
}
manifest['growl_expression_v212'] = {
    'previous_version': '2.0.11',
    'user_recording_s': 5.3,
    'reference_repeated_pitch_hz': 36.7,
    'user_recording_noted_pitches_hz': [110, 184.7],
    'sound_target': 'stronger vocal snarl; distinctly more useful knob extremes',
    'two_state_formant_filter': True,
    'asymmetric_overdrive': True,
    'bounded_wavefold': True,
    'eight_controls_retained': True,
    'gain_control_zero_is_silent': True,
    'params_cached_per_pcm_block': True,
    'parameter_cache_stack_bytes': 20,
    'extra_oscillator_voices': 0,
    'extra_delay_buffers': 0,
    'new_pcm_64_bit_math': False,
    'new_pcm_division_or_float': False,
    'host_extreme_parameter_sustained_cases': 1536,
    'long_sustain_and_low_eq_dual_sub_reverb_tested': True,
    'physical_fm1_verified': False,
    'hardware_dma_margin_verified': False,
}
manifest['growl_performance_v213'] = {
    'two_independent_device_global_preset_knob_assignments': True,
    'cxs_default': 'TEMPO / BPM',
    'growl_default': 'EXPRESSION',
    'expression_range': [0, 100],
    'expression_units_per_encoder_detent': 5,
    'neutral_at_50_identical_to_v212': True,
    'voice_dimensions': ['FORMANT', 'BRIGHTNESS', 'BODY', 'PRESSURE', 'WAVEFOLD'],
    'individual_touch_axes_or_per_note_mpe': False,
    'preset_edits_or_flash_writes_while_turning_expression': False,
    'control_coefficients_processed_once_per_dma_block': True,
    'new_per_sample_pcm_operations': 0,
    'new_pcm_delay_buffers_or_oscillator_voices': 0,
    'new_flash_sectors_or_project_format_version': 0,
    'global_journal_record_bytes': 8,
    'legacy_assignment_records_supported': True,
    'power_loss_and_global_compaction_host_tested': True,
    'physical_fm1_dma_late_verified': False,
}
manifest['growl_performance_v214'] = {
    'growl_default': 'EXPRESSION LATCH',
    'neutral_expression': 50,
    'expression_zero_audible_when_level_nonzero': True,
    'negative_expression_gradual_presence_floor': True,
    'neutral_and_positive_sound_unchanged': True,
    'modes': ['EXPRESSION LATCH', 'EXPRESSION RETURN'],
    'assignment_ids': [94,95],
    'return_delay_ms': 125,
    'return_step_per_25ms': 5,
    'springback_main_loop_only': True,
    'global_preferences_record_unchanged_bytes': 8,
    'new_per_sample_pcm_operations': 0,
    'physical_fm1_dma_late_verified': False,
}
manifest['growl_reference_v215'] = {
    'source': 'user supplied 19-second ROLI Seaboard video',
    'method': 'spectral harmonic proportions, synthetic additive oscillator',
    'original_reference_audio_embedded': False,
    'dominant_harmonics': [3, 4],
    'expression_sweep': 'nonlinear closed-to-open-vowel-to-strained',
    'expression_zero_audible': True,
    'expression_neutral': 50,
    'latch_and_return_preserved': True,
    'motion_transient_cached_per_pcm_block': True,
    'new_per_sample_pcm_operations': 0,
    'extra_dsp_states': 0,
    'new_oscillators': 0,
    'new_delay_buffers': 0,
    'physical_fm1_dma_late_verified': False,
}
manifest['growl_automation_v216'] = {
    'enabled_by_default': True,
    'slow_expression_lfo_period_ms': 5400,
    'micro_expression_lfo_period_ms': 1700,
    'nonrepeating_interpolated_formant_drift_period_ms': 8000,
    'expression_offset_bounds': [-16, 16],
    'formant_drift_bounds': [-4, 4],
    'manual_knob_priority_fade_ms': 450,
    'expression_zero_remains_audible': True,
    'expression_modes_retained': ['EXPRESSION LATCH', 'EXPRESSION RETURN'],
    'foreground_only_lfo_computation': True,
    'pitch_drift': False,
    'new_per_sample_pcm_operations': 0,
    'physical_fm1_dma_late_verified': False,
}
manifest['growl_preset_reboot_v217'] = {
    'save_project_format': 35,
    'previous_buggy_save_project_format': 34,
    'tested_save_paths': ['PRESET SAVE', 'PROJECT SAVE', 'autosave'],
    'distortion_and_fx_chain_survive_reboot': True,
    'dual_sub_and_blend_survive_reboot': True,
    'legacy_pjt34_read_compatible': True,
    'old_pjt34_omitted_fx_unrecoverable': True,
    'audio_processing_changed': False,
    'new_per_sample_pcm_operations': 0,
    'physical_fm1_dma_late_verified': False,
}
manifest['sub_realtime_expression_v218'] = {
    'main_cxs_rate_hz': 44100,
    'main_growl_rate_hz': 44100,
    'sub_cxs_internal_rate_hz': 11025,
    'sub_cxs_interpolated_output_rate_hz': 44100,
    'sub_cxs_smoothing_matches_four_original_steps': True,
    'sub_cxs_pcm_bit_identical_to_v217': False,
    'active_sub_types_quarter_rate': ['CXS', 'CLASSIC', 'SQUARE', 'SAW'],
    'other_sub_modes_bit_identical_to_v217': False,
    'off_sub_pcm_bit_identical_to_v217': True,
    'host_sub_cxs_energy_ratio_nominal': 0.9992,
    'host_sub_cxs_correlation_nominal': 0.9887,
    'host_sub_active_energy_ratio_nominal_range': [0.9881, 1.0101],
    'host_sub_active_correlation_nominal_min': 0.9887,
    'host_sub_cxs_speedup_direction_observed': True,
    'main_cxs_coefficients_prepared_per_block': True,
    'expression_q8_fractional_formant_interpolation': True,
    'expression_slew_per_dma_block': True,
    'expression_one_detent_no_transient_trigger': True,
    'new_cxs_sub_state_bytes_bss': 12,
    'new_pcm_buffers': 0,
    'trusted_ota_unchanged': True,
    'hardware_dma_late_zero_verified': False,
}
manifest['preset_open_realtime_v219'] = {
    'preset_wave_rebuild_deferred_while_voice_active': True,
    'preset_wave_rebuild_after_voice_release': True,
    'live_preset_crossfade_samples': 256,
    'preset_ramp_no_integer_division_in_sample_loop': True,
    'preset_ramp_pcm_bit_identical_to_v218': True,
    'growl_cxs_main_dsp_unchanged': True,
    'sub_interpolator_unchanged': True,
    'fx_and_pjt35_persistence_unchanged': True,
    'physical_fm1_preset_open_verified': False,
}
manifest['preset_audit_v220'] = {
    'per_preset_cpu_wall_peak': True,
    'per_preset_cpu_render_peak': True,
    'per_preset_dma_lates': True,
    'per_preset_dma_guard_hits': True,
    'saved_core_parameters_hash': True,
    'saved_fx_parameters_hash': True,
    'save_reference_ram_only': True,
    'cold_boot_reference_unknown': True,
    'audio_pcm_engine_unchanged': True,
    'flash_project_codec_unchanged': True,
    'physical_fm1_preset_open_verified': False,
}
manifest['fx_dispatch_v221'] = {
    'trailing_empty_skipped_once_per_block': True,
    'scan_once_per_dma_frames': 128,
    'pcm_processors_unchanged': True,
    'ordered_slots_and_interior_holes_preserved': True,
    'fx_bypass_and_tail_semantics_preserved': True,
    'host_fx_scenarios_exact': 24,
    'host_stereo_frames_exact': 294912,
    'cxs_and_growl_timbre_unchanged': True,
    'project_codec_unchanged': True,
    'hardware_fm1_speedup_verified': False,
    'physical_fm1_preset_open_verified': False,
}
manifest['fx_dispatch_v222'] = {
    'ordered_slot_indices_prepared_per_dma_block': True,
    'interior_empty_positions_skipped': True,
    'bypassed_slots_retained_for_fades': True,
    'dense_chain_uses_sequential_scan': True,
    'old_public_fx_entrypoints_preserved': True,
    'host_layout_scenarios_exact': 120,
    'host_stereo_frames_exact': 983040,
    'effect_formulas_unchanged': True,
    'cxs_and_growl_timbre_unchanged': True,
    'physical_fm1_dma_margin_verified': False,
    'physical_fm1_preset_open_verified': False,
}
manifest['fx_control_cache_v223'] = {
    'published_control_frames': 32,
    'coefficient_comparisons_per_unchanged_segment': 1,
    'control_snapshot_invalidates_cache': True,
    'dry_compatibility_invalidates_cache': True,
    'hidden_legacy_modulation_forces_full_validation': True,
    'fx_reset_invalidates_cache': True,
    'host_stereo_frames_exact': 368640,
    'pcm_algorithms_unchanged': True,
    'public_fx_apis_unchanged': True,
    'physical_fm1_dma_margin_verified': False,
    'physical_fm1_preset_open_verified': False,
}
manifest['distortion_coeff_cache_v224'] = {
    'control_segment_frames': 32,
    'max_stack_bytes': 50,
    'per_slot_gain_bias_tone_cache': True,
    'independent_and_repeated_insert_safe': True,
    'fixed_point_equations_unchanged': True,
    'no_extra_pcm_buffers': True,
    'host_stereo_frames_exact': 524288,
    'physical_fm1_dma_margin_verified': False,
    'physical_fm1_preset_open_verified': False,
}
manifest['growl_mip_cache_v225'] = {
    'cache_scope': 'one audio DMA block',
    'max_stack_bytes': 12,
    'exact_pitch_change_invalidates_cache': True,
    'generated_waves_unchanged': True,
    'growl_phase_interpolation_unchanged': True,
    'growl_formant_processing_unchanged': True,
    'main_cxs_and_subs_unchanged': True,
    'host_carrier_formant_frames_exact': 1048576,
    'host_pitch_decisions_reused': 646656,
    'physical_fm1_dma_margin_verified': False,
    'physical_fm1_preset_open_verified': False,
}
manifest['fx_matrix_sand_v226'] = {
    'effect_types': 10,
    'simultaneous_slots': 8,
    'distinct_subsets_exhaustive': 1013,
    'ordered_short_chains_exhaustive': 5860,
    'ordered_pairs_including_repeats_exhaustive': 100,
    'reverb_topologies': ['SPRING', 'HALL', 'ROOM'],
    'independent_and_legacy_parameter_modes': True,
    'real_sub1_cxs_frames': 491520,
    'host_configurations': 48064,
    'host_stereo_frames_sample_exact': 7532032,
    'sandpaper_mix_target_cached': True,
    'sandpaper_signed_q15_division_exact': True,
    'sound_quality_changed': False,
    'all_possible_eight_slot_ordered_racks_exhaustive': False,
    'physical_fm1_dma_improvement_verified': False,
    'physical_fm1_sandpaper_reverb_verified': False,
}
manifest['fx_sram_hotpath_v227'] = {
    'xip_to_sram_only_no_dsp_math_change': True,
    'hot_functions': ['rupture_fx_process_ordered_implementation',
                      'ru_reverb_spring', 'ru_reverb_room', 'ru_reverb_hall',
                      'rupture_sandpaper_process', 'etch_fx_ring_eq',
                      'etch_fx_wet_safety'],
    'ram_text_region_bytes': 16384,
    'ram_text_budget_bytes': 13312,
    'boot_copy_precedes_irqs': True,
    'pc_guard_includes_ram_text': True,
    'large_program_calls_only_native_fx': True,
    'cxs_main_source_unchanged': True,
    'fx_algorithm_unchanged': True,
    'physical_fm1_dma_margin_verified': False,
    'physical_fm1_saved_preset_crackle_verified': False,
}
manifest['fx_deadline_recovery_v228'] = {
    'reason': 'physical AUDIO STOP reported with MOSFET + DISTORTION + REVERB',
    'experimental_partial_sram_fx_disabled': True,
    'large_program_native_fx_flag_removed': True,
    'fx_fade_32bit_bounded_exact_division': True,
    'fade_64bit_reference_retained_for_host_parity': True,
    'first_held_key_not_started_during_analog_mute': True,
    'all_currently_playing_audio_effects_retained': True,
    'cxs_engine_source_unchanged': True,
    'physical_fm1_no_dma_lates_verified': False,
    'physical_first_note_latency_verified': False,
}
manifest['usb_uac_foreground_v229'] = {
    'trigger': 'USB-C GarageBand iOS with CXS + SUB1 + REVERB DMA LATES',
    'negotiated_rate_observed_on_device': True,
    'usb_44100_direct_path_unchanged': True,
    'usb_48000_fir_coefficients_unchanged': True,
    'usb_48000_fir_24tap_q30_stereo_outside_audio_isr': True,
    'usb_48000_1024_frame_staging_fifo': True,
    'usb_48000_queue_overflow_counted_in_uac_overruns': True,
    'host_fir_output_pcm_bit_exact': True,
    'cxs_main_source_unchanged': True,
    'sub1_source_unchanged': True,
    'fx_audible_pcm_unchanged': True,
    'physical_fm1_no_dma_lates_verified': False,
    'physical_garageband_usb_capture_verified': False,
    'usb_48000_foreground_scheduling_margin_verified': False,
}
manifest['usb_packed_scope_v230'] = {
    'reason': 'continue realtime margin optimization without any synth/FX timbre change',
    'usb_48000_fir_q30_math_unchanged': True,
    'usb_48000_packed_pcm_byte_exact': True,
    'usb_48000_staged_fifo_frames': 1024,
    'usb_48000_q15_to_int32_to_q15_removed': True,
    'usb_ram_bytes_saved': 512,
    'scope_preparation_skipped_when_optional_capture_is_guaranteed_off': True,
    'scope_output_decision_unchanged': True,
    'usb_44100_direct_path_unchanged': True,
    'all_audio_and_effects_preserved': True,
    'cxs_source_sha256': '86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
    'physical_dma_lates_zero_verified': False,
    'physical_garageband_capture_verified': False,
    'hardware_cpu_savings_measured': False,
}
manifest['usb_rate_settings_v231'] = {
    'settings_menu': 'SETTINGS > USB-C AUDIO',
    'sample_rates_hz': [44100, 48000],
    'usb_rate_advertisement': 'UAC1 continuous rate range with min=max',
    'format_descriptor_length_bytes_unchanged': True,
    'usb_reenumerates_after_change': True,
    'bcd_device_changes_with_rate': True,
    'persistent_device_global_nor_preference': True,
    'legacy_presets_and_project_format_unchanged': True,
    'cxs_pcm_unchanged': True,
    'fx_pcm_unchanged': True,
    'second_dsp_core_enabled': False,
    'ios_garageband_44100_48000_physically_verified': False,
    'hardware_dma_lates_zero_verified': False,
}
manifest['cpu1_ram_integrity_v232'] = {
    'build_flag': '--dual-core-lab',
    'default_firmware_cpu1_enabled': False,
    'lab_app_cpu1_enabled': True,
    'lab_app_firmware_fwsc_exported': False,
    'cpu1_executes_audio_dsp': False,
    'cpu1_worker_uses_only_private_ram_buffers': True,
    'isolated_ram_bytes': 8192,
    'isolated_ram_used_bytes': 4640,
    'cpu1_stack_bytes': 4096,
    'deadline_ms': 4,
    'host_fault_injection_cases': 5162,
    'core0_recomputes_each_output': True,
    'bad_read_or_timeout_retires_until_poweroff': True,
    'soft_reset_after_cpu1_start_bars_next_start': True,
    'cxs_and_fx_pcm_unchanged': True,
    'physical_cpu1_startup_tested': False,
    'physical_dma_lates_zero_verified': False,
}
manifest['cpu1_shadow_fir_v233'] = {
    'opt_in_lab_flag': '--dual-core-lab',
    'standard_fwsc_cpu1_disabled': True,
    'shadow_fir_only_without_voices_sequence_or_usb_capture': True,
    'uses_unchanged_usb_fir_24_taps_q30': True,
    'cpu1_processes_synthetic_stereo_fir_jobs': True,
    'cpu0_recalculates_every_stereo_sample': True,
    'cpu1_reads_immutable_fir_coefficients_only': True,
    'cpu1_audio_buffer_write_enabled': False,
    'cpu1_live_usb_offload_enabled': False,
    'host_probe_cases': 7243,
    'host_corruption_rejected': True,
    'cpu1_bad_read_timeout_fault_retires': True,
    'physical_cpu1_verified': False,
    'physical_dma_zero_verified': False,
}
manifest['cpu1_live_uac_v234'] = {
    'live_offload_build_flag': '--dual-core-live-uac-lab',
    'requires': '--dual-core-lab --usb-audio-48k',
    'standard_fwsc_cpu1_enabled': False,
    'lab_only_live_cpu1_offload': True,
    'live_offload_block_frames': 64,
    'original_usb_48k_coefficients_and_rounding_retained': True,
    'cpu0_full_parity_probation_blocks': 64,
    'cpu0_periodic_full_parity_interval_blocks': 256,
    'cpu0_live_spot_checks_per_block': 8,
    'cpu0_failure_fallback_original_fir': True,
    'cpu1_response_deadline_ms': 1.5,
    'cpu1_direct_access_to_live_audio_buffers': False,
    'cpu0_updates_fir_history_atomically_after_validation': True,
    'host_checked_output_frames': 142664,
    'host_outputs_sample_exact': True,
    'single_corrupted_sample_guaranteed_detected': False,
    'physical_fm1_verified': False,
    'hardware_dma_lates_zero_verified': False,
    'garageband_ios_audio_unchanged_verified': False,
}
manifest['cpu1_auto_recovery_v235'] = {
    'lab_only': True,
    'normal_fwsc_single_core_only': True,
    'active_usb_rate_hz': 48000,
    'core0_detects_new_dma_late': True,
    'core0_detects_new_dma_guard': True,
    'core0_detects_new_usb_underrun': True,
    'core0_detects_new_usb_stage_drop': True,
    'core0_detects_audio_stop': True,
    'live_cpu1_response_deadline_ticks_24mhz': 36000,
    'early_abort_min_usb_pcm_ring_frames': 64,
    'retire_cpu1_for_session_after_pressure': True,
    'pending_core1_result_never_accepted_after_retirement': True,
    'fallback_original_cpu0_fir': True,
    'sound_algorithms_and_coefficients_unchanged': True,
    'isolated_corrupt_cpu1_pcm_guaranteed_detected': False,
    'physical_fm1_verified': False,
    'no_dma_lates_guaranteed': False,
    'host_underrun_already_occurred_reversible': False,
}
manifest['cpu1_dual_fwsc_v239']={
    'fwsc_count_per_iteration':2,
    'stable_filename':target.name,
    'experimental_filename':live_target.name,
    'stable_cpu1_enabled':False,
    'experimental_cpu1_enabled':True,
    'experimental_live_usb_fir_enabled':False,
    'both_fwscs_verified':True,
    'same_trusted_ota_loader':True,
    'same_protected_bootheader':True,
    'embedded_web_installers_select_both':True,
    'web_installer_stable_default':True,
    'web_installer_core2_requires_extra_ack':True,
    'experimental_requires_physical_recovery_ready':True,
    'experimental_physical_fm1_qualified':False,
    'clean_full_project_zip':True,
}
manifest['note_fx_lcd_arbitration_v239']={
    'enabled_fx_threshold':3,
    'per_note_cosmetic_lcd_quiet_ms':64,
    'sustained_heavy_fx_cosmetic_lcd_min_interval_ms':120,
    'separate_note_on_off_dma_diagnostics':True,
    'per_note_edge_isr_timestamp_reads_added':False,
    'panel_note_on_and_off_covered':True,
    'midi_note_state_edges_covered':True,
    'commands_and_fault_screen_never_deferred':True,
    'audio_render_or_effects_modified':False,
    'fx_chain_on_core2':True,
    'core2_live_48k_usb_fir_only':False,
    'hardware_dma_lates_zero_verified':False,
}
manifest['optional_fx_scope_budget_v239']={
    'active_fx_threshold':3,
    'note_edge_quiet_dma_halves':24,
    'previous_dma_slack_threshold_percent':20,
    'low_slack_applies_even_to_two_or_zero_active_fx':True,
    'only_optional_scope_averaging_and_capture_skipped':True,
    'existing_scope_deferred_counter_increments':True,
    'pcm_fx_cxs_midi_usb_deferred':False,
    'sram_or_heap_allocation_added':False,
    'hardware_qualified':False,
}
manifest['sikkenworks_intro_v243']={
    'frames':86,'frame_interval_ms':60,'duration_ms':5160,
    'logo_in_ms':420,'logo_out_ms':420,
    'text_in_ms':480,'text_out_ms':480,
    'existing_41_key_led_boot_wave_preserved':True,
    'logo_pixels':[240,240],'logo_grayscale8_bytes':57600,
    'logo_unique_gray_levels':242,
    'logo_source':'assets/boot-sikkenworks-photo-v243.png',
    'logo_source_sha256':'85d690dd543a5fdd1d3e1d0267760a37148b9a33b536b24560ef1a46966c0dff',
    'logo_scaling':'crop_transparent_gutters_aspect_fit_nearest_neighbor',
    'fade':'16_alpha_levels_applied_to_exact_grayscale_on_RGB565_LCD',
    'logo_dithered':False,
    'text':'And lo, the heavens fell silent, and the earth trembled beneath mankind.\n\nAnd then the Rupture began.',
    'case_sensitive_intro_font':True,
    'word_change':'Rupture -> RUPTURE',
    'all_other_text_pixels_static_during_word_flicker':True,
    'before_audio_engine_start':True,
    'dsp_pcm_unchanged':True,
    'physical_fm1_display_verified':False,
}
manifest['cpu1_full_fx_v243']={
    'stable_full_fx_owner':'core0',
    'experimental_full_fx_owner':'core2_after_probation',
    'experimental_fx_slots':8,
    'experimental_fx_types':10,
    'intercore_chunk_frames':64,
    'cpu0_cxs_growl_sub_pcm_unchanged':True,
    'core2_changes_fx_transfer_timing_not_algorithms':True,
    'fx_tanks_duplicate_copies':False,
    'simultaneous_usb_fir_on_core2':False,
    'cpu0_resume_from_completed_sample_on_timeout':True,
    'cpu1_halted_before_cpu0_touches_fx_state':True,
    'shared_ram_hardware_corruption_proven_absent':False,
    'host_audio_pcm_parity_verified':True,
    'host_pcm_ordered_fx_pairs':100,
    'host_pcm_heavy_four_fx_chains':6,
    'host_pcm_stereo_frames_exact':81408,
    'no_dma_lates_guaranteed':False,
    'hardware_verified':False,
}
manifest['note_release_guard_v243']={
    'group_capture_preserves_note_off':True,
    'note_on_zero_velocity_preserved_as_note_off':True,
    'panel_mono_stack_reconciled_per_pcm_block':True,
    'genuine_long_hold_never_time_limited':True,
    'midi_sustain_preserved':True,
    'midi_overflow_clears_only_midi_identities':True,
    'panel_and_sequencer_survive_midi_panic':True,
    'stack_bounds_guarded_on_both_core_variants':True,
    'cxs_and_fx_equations_changed':False,
    'host_ubsan_regressions_passed':True,
    'hardware_verified':False,
    'zero_hung_notes_guaranteed':False,
}
manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')

for installer_name in ('WebInstaller.html', 'index.html'):
    installer_path = ROOT / installer_name
    before = installer_path.read_text()
    # Offline index must show the actual current manual/changelog. Preserve
    # the entire surrounding HTML/JS and swap only these embedded data blocks.
    offline_before = {}
    offline_after = {}
    old_meta = None
    new_meta = None
    assert 'Stop at the FIRST NUL' in before and 'read-only identity queries' in before
    assert 'V2.0.40' in before or 'V2.0.43' in before
    package_match = re.search(r'(<script id="firmware"[^>]*>)(.*?)(</script>)', before, re.S)
    assert package_match
    old_pkg = base64.b64decode(package_match.group(2))
    # Safe to rerun packaging after a native build fix: an already embedded
    # V2.0.20 candidate may differ from the final one. Still forbid replacing
    # a payload with an unrelated loader or protected bootheader.
    old_entries = {e['name']: e['data'] for e in fw.parse_ufw(old_pkg)['entries']}
    assert old_entries['ota.bin'] == entries['ota.bin']
    assert old_entries['flash.bin'][:0x4000] == flash[:0x4000]
    old_hash = re.search(r"const PKG_HASH='([0-9a-f]{64})'", before).group(1)
    old_len = re.search(r'const PKG_LEN=(\d+)', before).group(1)
    assert old_hash == sha(old_pkg) and int(old_len) == len(old_pkg)
    exp_pattern=r'(<script id="firmware-experimental"[^>]*>)(.*?)(</script>)'
    exp_before=re.search(exp_pattern,before,re.S)
    old_exp_hash=re.search(r"const EXP_PKG_HASH='([0-9a-f]{64})'",before).group(1)
    old_exp_len=re.search(r'const EXP_PKG_LEN=(\d+)',before).group(1)
    if exp_before:
        exp_old_pkg=base64.b64decode(exp_before.group(2))
        exp_old_entries={e['name']:e['data'] for e in fw.parse_ufw(exp_old_pkg)['entries']}
        assert exp_old_entries['ota.bin']==entries['ota.bin']
        assert exp_old_entries['flash.bin'][:0x4000]==flash[:0x4000]
        assert old_exp_hash==sha(exp_old_pkg) and int(old_exp_len)==len(exp_old_pkg)
    else:
        assert before.count('id="firmware-experimental"')==0
    earlier_version = 'V2.0.40' if 'V2.0.40' in before else 'V2.0.43'
    after = before.replace('V2.0.40', 'V2.0.43')
    after, n = re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',
        lambda m: m.group(1) + base64.b64encode(pkg).decode('ascii') + m.group(2),
        after, count=1, flags=re.S)
    assert n == 1
    exp_tag='\n<script id="firmware-experimental" type="application/octet-stream">'+base64.b64encode(live_pkg).decode('ascii')+'</script>'
    if exp_before:
        after,n=re.subn(exp_pattern,
            lambda m:m.group(1)+base64.b64encode(live_pkg).decode('ascii')+m.group(3),
            after,count=1,flags=re.S)
        assert n==1
    else:
        stable_end=re.search(r'<script id="firmware"[^>]*>.*?</script>',after,re.S)
        assert stable_end
        after=after[:stable_end.end()]+exp_tag+after[stable_end.end():]
    after, n = re.subn(r"const PKG_HASH='[0-9a-f]{64}'",
        f"const PKG_HASH='{sha(pkg)}'", after)
    assert n == 1
    after, n = re.subn(r'const PKG_LEN=\d+', f'const PKG_LEN={len(pkg)}', after)
    assert n == 1
    after,n=re.subn(r"const EXP_PKG_HASH='[0-9a-f]{64}'",
        f"const EXP_PKG_HASH='{sha(live_pkg)}'",after)
    assert n==1
    after,n=re.subn(r'const EXP_PKG_LEN=\d+',f'const EXP_PKG_LEN={len(live_pkg)}',after)
    assert n==1
    if installer_name == 'index.html':
        for tag, source in (('fallback-readme', 'README.md'),
                            ('fallback-changelog', 'CHANGELOG.md')):
            pattern = r'(<script id="' + tag + r'"[^>]*>)(.*?)(</script>)'
            m = re.search(pattern, after, re.S)
            assert m is not None
            offline_before[tag] = m.group(2)
            offline_after[tag] = base64.b64encode((ROOT / source).read_bytes()).decode('ascii')
            after = after[:m.start(2)] + offline_after[tag] + after[m.end(2):]
        meta_match = re.search(r'(<script id="site-release-meta"[^>]*>)(.*?)(</script>)', after, re.S)
        assert meta_match is not None
        old_meta = meta_match.group(2)
        site_meta = json.loads(old_meta)
        site_meta.update(version='2.0.43',sha256=sha(pkg),bytes=len(pkg),
            experimental_sha256=sha(live_pkg),experimental_bytes=len(live_pkg),
            default_firmware='stable',available_firmwares=['stable','core2'])
        new_meta = json.dumps(site_meta)
        after = after[:meta_match.start(2)] + new_meta + after[meta_match.end(2):]
    # Remove only the differences a version refresh is allowed to introduce.
    normalized_after = after.replace('V2.0.43', earlier_version)
    if installer_name == 'index.html':
        # Restore the entire JSON payload BEFORE replacing global firmware SHA.
        # Otherwise new_meta's hash no longer matches for the reverse check.
        normalized_after = normalized_after.replace(new_meta, old_meta)
    normalized_after = normalized_after.replace(base64.b64encode(pkg).decode('ascii'), package_match.group(2))
    if exp_before:
        normalized_after=normalized_after.replace(base64.b64encode(live_pkg).decode('ascii'),exp_before.group(2))
    else:
        normalized_after=normalized_after.replace(exp_tag,'')
    normalized_after = normalized_after.replace(sha(pkg), old_hash)
    normalized_after = normalized_after.replace(f'const PKG_LEN={len(pkg)}', f'const PKG_LEN={old_len}')
    normalized_after = normalized_after.replace(sha(live_pkg),old_exp_hash)
    normalized_after = normalized_after.replace(f'const EXP_PKG_LEN={len(live_pkg)}',f'const EXP_PKG_LEN={old_exp_len}')
    if installer_name == 'index.html':
        for tag in offline_before:
            normalized_after = normalized_after.replace(offline_after[tag], offline_before[tag])
    normalized_before = before
    if normalized_after != normalized_before:
        for ci,(before_ch,after_ch) in enumerate(zip(normalized_before,normalized_after)):
            if before_ch != after_ch:
                print('Index first mismatch at',ci,'BEFORE:',
                      repr(normalized_before[max(0,ci-75):ci+100]),
                      'AFTER:',repr(normalized_after[max(0,ci-75):ci+100]))
                break
        print('Lengths',len(normalized_before),len(normalized_after))
    assert normalized_after == normalized_before, f'{installer_name}: OTA/connection logic changed'
    assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>', after, re.S).group(1)) == pkg
    assert base64.b64decode(re.search(exp_pattern,after,re.S).group(2))==live_pkg
    assert '<option value="stable" selected>' in after
    assert '<option value="core2">' in after
    assert 'id="experimental-ack"' in after
    installer_path.write_text(after)

print('PASS V2.0.43 STABLE FWSC:', len(pkg), sha(pkg))
print('PASS V2.0.43 CORE2 EXPERIMENTAL FWSC:',len(live_pkg),sha(live_pkg))
print('PASS V2.0.43 stable app:', len(APP), sha(APP))
print('PASS unchanged OTA loader:', sha(entries['ota.bin']))
print('PASS unchanged protected bootheader:', sha(flash[:0x4000]))
print('PASS both Web MIDI installers select SHA-checked STABLE/CORE2; OTA/connection logic unchanged')
