#!/usr/bin/env python3
"""RUPTURE V2.0.58: both FWSCs retain trusted OTA/PJT35 and sample identity."""
from pathlib import Path
import base64
import hashlib
import json
import re
import sys

R = Path(__file__).resolve().parents[1]
N = R / 'vendor/etch-v1.88-source'
SRC = N / 'firmware/fm1-native'
sys.path.insert(0, str(N / 'tools'))
import fm1_fwsc as fw

sha = lambda data: hashlib.sha256(data).hexdigest()
manifest = json.loads((R / 'RELEASE-MANIFEST.json').read_text())
assert manifest['version'] == '2.0.58'
intro=manifest['sikkenworks_intro_v258']
assert intro['frames']==86 and intro['duration_ms']==5160
assert intro['existing_41_key_led_boot_wave_preserved']
assert intro['logo_grayscale8_bytes']==57600 and intro['logo_scaling']=='crop_transparent_gutters_aspect_fit_nearest_neighbor'
assert intro['logo_unique_gray_levels']==242 and intro['logo_dithered'] is False
assert intro['fade']=='16_alpha_levels_applied_to_exact_grayscale_on_RGB565_LCD'
assert intro['logo_source']=='assets/boot-sikkenworks-photo-v258.png'
assert intro['case_sensitive_intro_font'] and intro['all_other_text_pixels_static_during_word_flicker']
assert not intro['before_audio_engine_start'] and intro['dsp_pcm_unchanged']
assert intro['synth_and_codec_warmup_during_intro']
assert intro['welcome_back_after_intro_ms']==1100
assert intro['live_dsp_max_excludes_one_time_intro_spikes']
assert intro['cumulative_dma_lates_preserved']
assert manifest['firmware_file'] == 'RUPTURE-V2.0.58-STABLE.fwsc'
assert manifest['experimental_firmware_file'] == 'RUPTURE-V2.0.58-CORE2-EXPERIMENTAL.fwsc'
assert manifest['note_release_guard_v258']['group_capture_preserves_note_off']
assert manifest['note_release_guard_v258']['panel_mono_stack_reconciled_per_pcm_block']
assert not manifest['note_release_guard_v258']['hardware_verified']
assert manifest['mono_stack_corruption_v258']['six_note_stack_bounded_on_midi_cc120_cc121_cc123']
assert manifest['mono_stack_corruption_v258']['six_note_stack_bounded_before_performance_clear']
assert manifest['mono_stack_corruption_v258']['host_regression_reproduced_failure_before_fix']
assert manifest['mono_stack_corruption_v258']['host_regression_after_fix_stable_and_core2']
assert not manifest['mono_stack_corruption_v258']['per_sample_dsp_overhead_added']
assert manifest['mono_stack_corruption_v258']['physical_hardware_verified'] is False
touch=manifest['key_pressure_v258']
assert touch['sensed_key_range']=='F3_to_C5_20_of_27'
assert touch['unsensed_top_seven_fixed_neutral']
assert touch['note_on_off_still_from_original_digital_matrix']
assert touch['analog_row_restored_before_digital_scan']
assert touch['adc_timeout_disables_only_expression']
assert touch['smoothing_foreground_not_pcm_sample_loop']
assert touch['configurable_targets']==['OFF','FILTER','GROWL','MOSFET','DISTORTION','REVERB','SUB1','WAH']
assert touch['sensitivity_steps']==4 and touch['default']=='OFF_MEDIUM'
dual=manifest['dual_expression_v258']
assert dual['engines']==['CXS','GROWL']
assert dual['modes']==['PRESS','DRIFT','RANDOM','VELOCITY']
assert dual['midi_usb_and_trs_share_note_event_observer']
assert dual['midi_velocity_mode_ignores_aftertouch']
assert dual['autosave_off_never_commits_expression_profiles_automatically']
assert dual['manual_project_save_authorizes_expression_profile_journal']
assert dual['adjustable_min_max_percent']==[0,100]
assert dual['pressure_sensitivities']==4 and dual['drift_speeds']==4
assert dual['autonomous_drift_works_on_unsensed_top_seven_keys']
assert dual['reverb_mix_min_is_persisted_fx_chain_baseline']
assert dual['reverb_auto_inserts_only_in_first_empty_slot']
assert dual['reverb_full_chain_preserves_existing_effects']
assert dual['two_new_crc_checked_preference_records']
assert not dual['physical_fm1_verified']
assert manifest['fx_dispatch_v221']['trailing_empty_skipped_once_per_block']
assert manifest['fx_dispatch_v221']['host_stereo_frames_exact'] == 294912
assert manifest['fx_dispatch_v222']['interior_empty_positions_skipped']
assert manifest['fx_dispatch_v222']['host_stereo_frames_exact'] == 983040
assert manifest['fx_control_cache_v223']['host_stereo_frames_exact'] == 368640
assert manifest['fx_control_cache_v223']['hidden_legacy_modulation_forces_full_validation']
assert manifest['distortion_coeff_cache_v224']['host_stereo_frames_exact'] == 524288
assert manifest['distortion_coeff_cache_v224']['no_extra_pcm_buffers']
assert manifest['growl_mip_cache_v225']['host_carrier_formant_frames_exact'] == 1048576
assert manifest['growl_mip_cache_v225']['generated_waves_unchanged']
assert manifest['fx_matrix_sand_v226']['host_configurations'] == 48064
assert manifest['fx_matrix_sand_v226']['host_stereo_frames_sample_exact'] == 7532032
assert manifest['fx_matrix_sand_v226']['real_sub1_cxs_frames'] == 491520
assert not manifest['fx_matrix_sand_v226']['physical_fm1_dma_improvement_verified']
sram = manifest['fx_sram_hotpath_v227']
assert sram['xip_to_sram_only_no_dsp_math_change']
assert sram['ram_text_region_bytes'] == 16384
assert sram['ram_text_budget_bytes'] == 13312
assert len(sram['hot_functions']) == 7
assert sram['cxs_main_source_unchanged'] and sram['fx_algorithm_unchanged']
assert sram['boot_copy_precedes_irqs'] and sram['pc_guard_includes_ram_text']
assert not sram['physical_fm1_dma_margin_verified']
recovery = manifest['fx_deadline_recovery_v228']
assert recovery['experimental_partial_sram_fx_disabled']
assert recovery['large_program_native_fx_flag_removed']
assert recovery['fx_fade_32bit_bounded_exact_division']
assert recovery['fade_64bit_reference_retained_for_host_parity']
assert recovery['first_held_key_not_started_during_analog_mute']
assert recovery['all_currently_playing_audio_effects_retained']
assert recovery['cxs_engine_source_unchanged']
assert not recovery['physical_fm1_no_dma_lates_verified']
usb229 = manifest['usb_uac_foreground_v229']
assert usb229['negotiated_rate_observed_on_device']
assert usb229['usb_44100_direct_path_unchanged']
assert usb229['usb_48000_fir_coefficients_unchanged']
assert usb229['usb_48000_fir_24tap_q30_stereo_outside_audio_isr']
assert usb229['host_fir_output_pcm_bit_exact']
assert usb229['fx_audible_pcm_unchanged'] and usb229['cxs_main_source_unchanged']
assert not usb229['physical_fm1_no_dma_lates_verified']
assert not usb229['physical_garageband_usb_capture_verified']
usb230 = manifest['usb_packed_scope_v230']
assert usb230['usb_48000_fir_q30_math_unchanged']
assert usb230['usb_48000_packed_pcm_byte_exact']
assert usb230['usb_ram_bytes_saved'] == 512
assert usb230['scope_output_decision_unchanged']
assert usb230['all_audio_and_effects_preserved']
assert not usb230['physical_dma_lates_zero_verified']
usb231 = manifest['usb_rate_settings_v231']
assert usb231['sample_rates_hz'] == [44100, 48000]
assert usb231['format_descriptor_length_bytes_unchanged']
assert usb231['usb_reenumerates_after_change']
assert usb231['persistent_device_global_nor_preference']
assert usb231['legacy_presets_and_project_format_unchanged']
assert usb231['cxs_pcm_unchanged'] and usb231['fx_pcm_unchanged']
assert not usb231['second_dsp_core_enabled']
assert not usb231['ios_garageband_44100_48000_physically_verified']
assert not usb231['hardware_dma_lates_zero_verified']
cpu1 = manifest['cpu1_ram_integrity_v232']
assert not cpu1['default_firmware_cpu1_enabled']
assert cpu1['lab_app_cpu1_enabled']
assert not cpu1['lab_app_firmware_fwsc_exported']
assert not cpu1['cpu1_executes_audio_dsp']
assert cpu1['host_fault_injection_cases'] == 5162
assert cpu1['core0_recomputes_each_output']
assert cpu1['bad_read_or_timeout_retires_until_poweroff']
assert cpu1['cxs_and_fx_pcm_unchanged']
assert not cpu1['physical_cpu1_startup_tested']
probe = manifest['cpu1_shadow_fir_v233']
assert probe['opt_in_lab_flag']=='--dual-core-lab'
assert probe['uses_unchanged_usb_fir_24_taps_q30']
assert probe['cpu0_recalculates_every_stereo_sample']
assert probe['host_probe_cases']==7243
assert not probe['cpu1_live_usb_offload_enabled']
assert not probe['physical_cpu1_verified']
live = manifest['cpu1_live_uac_v234']
assert live['lab_only_live_cpu1_offload']
assert not live['standard_fwsc_cpu1_enabled']
assert live['cpu0_full_parity_probation_blocks']==64
assert live['cpu0_periodic_full_parity_interval_blocks']==256
assert live['cpu0_live_spot_checks_per_block']==8
assert live['cpu0_failure_fallback_original_fir']
assert live['host_checked_output_frames']==142664
assert live['host_outputs_sample_exact']
assert not live['single_corrupted_sample_guaranteed_detected']
assert not live['physical_fm1_verified']
guard = manifest['cpu1_auto_recovery_v235']
assert guard['lab_only'] and guard['normal_fwsc_single_core_only']
assert guard['active_usb_rate_hz'] == 48000
assert guard['core0_detects_new_dma_late'] and guard['core0_detects_new_dma_guard']
assert guard['core0_detects_new_usb_underrun'] and guard['core0_detects_new_usb_stage_drop']
assert guard['core0_detects_audio_stop']
assert guard['live_cpu1_response_deadline_ticks_24mhz'] == 36000
assert guard['early_abort_min_usb_pcm_ring_frames'] == 64
assert guard['retire_cpu1_for_session_after_pressure']
assert guard['pending_core1_result_never_accepted_after_retirement']
assert guard['fallback_original_cpu0_fir']
assert guard['sound_algorithms_and_coefficients_unchanged']
assert not guard['isolated_corrupt_cpu1_pcm_guaranteed_detected']
assert not guard['physical_fm1_verified'] and not guard['no_dma_lates_guaranteed']
assert not guard['host_underrun_already_occurred_reversible']
both=manifest['cpu1_dual_fwsc_v239']
assert both['fwsc_count_per_iteration']==2
assert both['stable_filename']==manifest['firmware_file']
assert both['experimental_filename']==manifest['experimental_firmware_file']
assert not both['stable_cpu1_enabled'] and both['experimental_cpu1_enabled']
assert not both['experimental_live_usb_fir_enabled']
assert manifest['cpu1_full_fx_v258']['experimental_full_fx_owner']=='core2_after_probation'
wait=manifest['core2_fx_wait_ema_v258']
poll=manifest['cpu1_mailbox_poll_v258']
assert poll['register_only_nops_between_inconclusive_polls']==32
assert poll['prior_register_only_nops']==16
assert poll['fallback_and_timeout_modified'] is False
assert poll['physical_dma_improvement_verified'] is False
fast_gain=manifest['core2_fx_post_gain_fastpath_v258']
assert fast_gain['normal_gain_upper_inclusive']==32768
assert fast_gain['clamp_checks_skipped_per_normal_stereo_frame']==2
assert fast_gain['out_of_range_gain_uses_original_clamping']
assert fast_gain['full_fx_chain_stays_on_cpu1']
assert fast_gain['cxs_growl_sub_fx_sample_equations_modified'] is False
assert fast_gain['physical_dma_or_dsp_reduction_verified'] is False
assert wait['first_valid_sample_seeds_average']
assert wait['timeout_or_corrupt_reply_updates_average'] is False
assert wait['original_maximum_and_timeout_remain_visible']
assert wait['cxs_fx_sample_formulas_modified'] is False
assert wait['fm1_dma_improvement_verified'] is False
assert manifest['cpu1_full_fx_v258']['intercore_chunk_frames']==64
assert manifest['cpu1_full_fx_v258']['intercore_mailboxes_per_block']==1
assert manifest['cpu1_full_fx_v258']['intercore_completed_sample_windows_per_block']==2
assert manifest['core2_stability_v258']['exclusive_fx_worker_after_128_integrity_probes_and_one_fir_probe']
assert manifest['core2_stability_v258']['cpu1_fx_mailbox_checks_expected_sequence_and_final_progress']
assert manifest['core2_stability_v258']['randomized_note_stress_stereo_frames_exact']==384000
assert manifest['core2_stability_v258']['physical_hardware_verified'] is False
assert manifest['core2_dma_failover_v258']['retire_cpu1_after_missed_dma_deadline_with_completed_fx_in_same_block']
assert manifest['core2_dma_failover_v258']['idle_or_unrelated_dma_pressure_does_not_retire_cpu1']
assert manifest['core2_dma_failover_v258']['retirement_reason_code']==7
assert manifest['core2_dma_failover_v258']['portable_pressure_guard_cases']==1000000
assert manifest['core2_dma_failover_v258']['physical_fm1_verified'] is False
assert both['both_fwscs_verified'] and both['same_trusted_ota_loader']
assert both['same_protected_bootheader']
assert both['embedded_web_installers_select_both']
assert both['web_installer_stable_default']
assert both['web_installer_core2_requires_extra_ack']
assert both['experimental_requires_physical_recovery_ready']
assert not both['experimental_physical_fm1_qualified']
assert both['clean_full_project_zip']
assert manifest['preset_audit_v220']['audio_pcm_engine_unchanged']
assert manifest['preset_audit_v220']['save_reference_ram_only']
assert not manifest['preset_audit_v220']['physical_fm1_preset_open_verified']
open219 = manifest['preset_open_realtime_v219']
assert open219['preset_wave_rebuild_deferred_while_voice_active']
assert open219['preset_ramp_pcm_bit_identical_to_v218']
assert not open219['physical_fm1_preset_open_verified']
sub218 = manifest['sub_realtime_expression_v218']
assert sub218['main_cxs_rate_hz'] == sub218['main_growl_rate_hz'] == 44100
assert sub218['sub_cxs_internal_rate_hz'] == 11025
assert sub218['sub_cxs_interpolated_output_rate_hz'] == 44100
assert not sub218['sub_cxs_pcm_bit_identical_to_v217']
assert not sub218['other_sub_modes_bit_identical_to_v217']
assert sub218['off_sub_pcm_bit_identical_to_v217']
assert sub218['active_sub_types_quarter_rate'] == ['CXS', 'CLASSIC', 'SQUARE', 'SAW']
assert sub218['expression_q8_fractional_formant_interpolation']
assert sub218['new_cxs_sub_state_bytes_bss'] == 12
assert not sub218['hardware_dma_late_zero_verified']
assert manifest['growl_performance_v214']['assignment_ids'] == [94, 95]
assert manifest['growl_reference_v215']['dominant_harmonics'] == [3, 4]
assert manifest['growl_reference_v215']['new_per_sample_pcm_operations'] == 0
saved = manifest['growl_preset_reboot_v217']
assert saved['save_project_format'] == 35
assert saved['previous_buggy_save_project_format'] == 34
assert saved['distortion_and_fx_chain_survive_reboot']
assert saved['dual_sub_and_blend_survive_reboot']
assert not saved['audio_processing_changed']
assert saved['new_per_sample_pcm_operations'] == 0
auto = manifest['growl_automation_v216']
assert auto['enabled_by_default'] and auto['foreground_only_lfo_computation']
assert auto['new_per_sample_pcm_operations'] == 0
assert auto['manual_knob_priority_fade_ms'] == 450
assert auto['expression_modes_retained'] == ['EXPRESSION LATCH', 'EXPRESSION RETURN']
assert auto['expression_zero_remains_audible'] and not auto['pitch_drift']
assert manifest['growl_performance_v213']['growl_default']=='EXPRESSION'
assert manifest['growl_performance_v213']['two_independent_device_global_preset_knob_assignments']
assert manifest['growl_performance_v213']['new_per_sample_pcm_operations']==0
assert manifest['growl_expression_v212']['params_cached_per_pcm_block']
assert manifest['growl_expression_v212']['host_extreme_parameter_sustained_cases'] == 1536
growl = manifest['reference_growl_main_v211']
assert growl['dominant_harmonics'] == [2,3,4]
assert growl['wavetables'] == 7 and growl['table_length'] == 256
assert growl['former_main_saw_storage_bit_preserved']
assert growl['sub_oscillator_saw_unchanged'] and growl['cxs_pcm_unchanged']
assert not growl['audio_samples_embedded'] and not growl['hardware_dma_margin_verified']
eq = manifest['channel_eq_realtime_v210']
assert eq['standalone_eq_does_not_dispatch_empty_fx_rack']
assert eq['all_four_saved_eq_gains_supported']
assert eq['original_four_biquad_coefficient_math_unchanged']
assert eq['original_wet_soft_knee_applied_consistently']
assert eq['old_wet_buffer_cleanup_bounded_to_one_sample_index']
assert eq['eq_scenario_count'] == 8
assert eq['eq_default_and_saved_knob_pcm_samples_compared'] == 6400000
assert eq['dual_sub_low_eq_hall_pcm_stress']
assert not eq['hardware_dma_late_zero_verified']
assert 'realtime_pcm_exact_v208' not in manifest
recovery = manifest['realtime_cpu_recovery_v209']
assert recovery['v208_cxs_exchange_optimization_reverted']
assert recovery['v208_room_unroll_and_bypass_fade_reverted']
assert recovery['cxs_original_v207_assembly_restored']
assert recovery['fx_reverb_feedback_and_tails_unchanged']
assert recovery['reverb_ordered_q15_mix_guard_checks_removed']
assert recovery['reverb_mixed_pcm_golden_cases'] == 6
assert recovery['reverb_mix_isolated_pi32v2_code_bytes_before_after'] == [66, 18]
assert recovery['sub_pitch_increment_parity_cases'] == 2154880
assert recovery['sub_pitch_isolated_pi32v2_code_bytes_before_after'] == [202, 106]
assert not recovery['hardware_dma_late_zero_verified']
assert manifest['preset_runtime_input_fix_v207']['input_handled_before_rupture_continue']
assert manifest['preset_runtime_input_fix_v207']['parameter_popup_reachable']
assert not manifest['preset_runtime_input_fix_v207']['physical_fm1_verified']
assert manifest['preset_popup_and_led_wave_v206']['offscreen_parameter_value_popup']
assert manifest['preset_popup_and_led_wave_v206']['led_count'] == 41
assert not manifest['preset_popup_and_led_wave_v206']['audio_interrupt_changes']
assert not manifest['preset_popup_and_led_wave_v206']['physical_fm1_verified']
assert manifest['audio_fx_glide_v204']['bend_pitch_slew_sample_rate']
assert len(manifest['audio_fx_glide_v204']['glide_curves']) == 6
assert manifest['audio_fx_glide_v204']['cxs_segment_comparisons_before_after'] == [7,3]
assert manifest['audio_fx_glide_v204']['persisted_fx_ids_unchanged']
assert not manifest['audio_fx_glide_v204']['dma_lates_zero_on_physical_fm1_verified']
assert manifest['subosc1_stare_v203']['held_forward_stare_frame_index'] == 11
assert not manifest['subosc1_stare_v203']['sustain_face_frames_advance']
assert manifest['subosc1_stare_v203']['camera_shake_max_pixels'] == 1
assert manifest['subosc1_stare_v203']['subosc2_original_face_loop_unchanged']
assert not manifest['subosc1_stare_v203']['audio_dsp_changed']
assert not manifest['subosc1_stare_v203']['glide_and_autosave_changed']
assert manifest['boot_and_subosc_fear_v202']['boot_fast_frame_cadence_ms'] == 35
assert manifest['boot_and_subosc_fear_v202']['boot_last_frame_hold_ms'] == 1000
assert manifest['boot_and_subosc_fear_v202']['boot_nominal_duration_ms'] == 1770
assert manifest['boot_and_subosc_fear_v202']['frames_per_oscillator'] == 16
assert manifest['boot_and_subosc_fear_v202']['sprite_size_pixels'] == [224, 30]
assert manifest['boot_and_subosc_fear_v202']['note_off_resets_to_idle']
assert not manifest['boot_and_subosc_fear_v202']['extra_audio_dsp']
assert manifest['boot_signal_breach_smooth_v201']['frames'] == 23
assert manifest['boot_signal_breach_smooth_v201']['exact_source_keyframes'] == 12
assert manifest['boot_signal_breach_smooth_v201']['ordered_dither_intermediate_frames'] == 11
assert manifest['boot_signal_breach_smooth_v201']['compressed_rom_bytes'] == 96425
assert manifest['boot_signal_breach_smooth_v201']['cadence_ms'] == 50
assert manifest['boot_signal_breach_smooth_v201']['duration_ms'] == 1880
assert manifest['boot_signal_breach_smooth_v201']['glide_autosave_uses_existing_preset_snapshot']
assert manifest['boot_signal_breach_v200']['frames'] == 12
assert manifest['boot_signal_breach_v200']['width_pixels'] == 240
assert manifest['boot_signal_breach_v200']['height_pixels'] == 240
assert manifest['boot_signal_breach_v200']['duration_ms'] == 1880
assert manifest['boot_signal_breach_v200']['lossless_rle_rom_bytes'] == 46340
assert manifest['boot_signal_breach_v200']['uses_existing_ui_framebuffer']
assert manifest['glide_visual_and_guide_v199']['tunnel_moves_only_during_held_voice']
assert manifest['glide_visual_and_guide_v199']['idle_first_frame_and_shake_zero']
assert manifest['glide_visual_and_guide_v199']['guide_display_modes'] == ['OFF', 'GUIDE', 'STRICT', 'LOCK']
assert manifest['glide_visual_and_guide_v199']['guide_stored_modes'] == [0, 1, 3, 2]
assert manifest['glide_save_v198']['stored_per_preset']
assert manifest['glide_save_v198']['manual_and_autosave_share_snapshot_and_codec']
assert not manifest['glide_save_v198']['flash_writes_on_glide_knob_event']
assert manifest['glide_tunnel_v197']['frames'] == 12
assert manifest['glide_tunnel_v197']['width_pixels'] == 224
assert manifest['glide_tunnel_v197']['height_pixels'] == 30
assert manifest['glide_tunnel_v197']['extra_pcm_operations_per_sample'] == 0
assert manifest['scale_strict_v196']['ui_modes'] == ['OFF', 'GUIDE', 'STRICT', 'LOCK']
assert manifest['scale_strict_v196']['stored_mode_values']['LOCK'] == 2
assert manifest['scale_strict_v196']['stored_mode_values']['STRICT'] == 3
assert manifest['scale_strict_v196']['new_per_sample_pcm_dsp_operations'] == 0
assert manifest['fx_expansion_v192']['available_types'] == 10
assert manifest['fx_expansion_v192']['simultaneous_slots'] == 8
assert manifest['fx_repeat_v193']['duplicate_type_in_slots_allowed']
assert manifest['fx_repeat_v193']['new_pcm_buffers'] == 0
assert manifest['fx_independent_v194']['slot_parameter_banks'] == 8
assert not manifest['fx_independent_v194']['reverb_decay_topology_independent']
assert manifest['realtime_optimization_v195']['golden_oracle_sample_exact']
assert manifest['realtime_optimization_v195']['pcm_ring_buffer_delta_bytes'] == 0
assert not manifest['release_ready'] and not manifest['physical_fm1_verified']
app = (R / 'native-build/app.bin').read_bytes()
assert not fw.validate_app_binary(app)
assert sha(app) == manifest['app_sha256'] and len(app) == manifest['app_bytes']
assert app == (N / 'build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
assert b'V2.0.58' in app
options = json.loads((R / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V2.0.58'
assert options['storage_writes'] and options['ota_writes']
new_fw = (R / manifest['firmware_file']).read_bytes()
old_fw = (R / 'reference/firmware/RUPTURE-V2.0.9.fwsc').read_bytes()
assert sha(new_fw) == manifest['firmware_sha256']
assert len(new_fw) == manifest['firmware_bytes'] == len(old_fw) == 610086
assert fw.verify_fwsc(R / manifest['firmware_file'])[1] == []
entries = lambda data: {e['name']: e['data'] for e in fw.parse_ufw(data)['entries']}
new, old = entries(new_fw), entries(old_fw)
assert new['ota.bin'] == old['ota.bin']
assert new['flash.bin'][:0x4000] == old['flash.bin'][:0x4000]
assert len(new['flash.bin']) == len(old['flash.bin']) == 0x93000
key = fw.extract_stock_assets(R / 'reference/firmware/RUPTURE-V1.86.1.fwsc')['key']
encrypted = bytearray(new['flash.bin'][0x4000:0x4000+0x120+len(app)])
fw.sfc_xor(encrypted, 0, len(encrypted), 0, key)
assert encrypted[0x120:] == app
experimental_file=R/manifest['experimental_firmware_file']
experimental=experimental_file.read_bytes()
assert len(experimental)==manifest['experimental_firmware_bytes']==len(new_fw)
assert sha(experimental)==manifest['experimental_firmware_sha256']
assert fw.verify_fwsc(experimental_file)[1]==[]
live=entries(experimental)
assert live['ota.bin']==new['ota.bin']==old['ota.bin']
assert live['flash.bin'][:0x4000]==new['flash.bin'][:0x4000]
live_app=(R/'native-build/cpu1-fx-app.bin').read_bytes()
assert len(live_app)==manifest['experimental_app_bytes']
assert sha(live_app)==manifest['experimental_app_sha256']
assert live_app!=app and b'V2.0.58' in live_app
live_bin=bytearray(live['flash.bin'][0x4000:0x4000+0x120+len(live_app)])
fw.sfc_xor(live_bin,0,len(live_bin),0,key)
assert live_bin[0x120:]==live_app
live_options=json.loads((R/'native-build/cpu1-fx-build-options.json').read_text())
assert live_options['cpu1_audio_offload_enabled']
assert live_options['cpu1_full_fx_live_lab_enabled']
assert not live_options['cpu1_live_usb_fir_lab_enabled']
assert not options['cpu1_audio_offload_enabled']

for name, expected in {
    'hal/fm1_audio.h': '9e711bbff254561c06376bbfd2f875d2eadeedc55dd412ed16bedf9b0b71e5cf',
    'native_output_startup.h': '65e5202624054b94f55fd1df058e9156e52e7536aefcd8536ca0659fca9f1e28',
}.items():
    source = (SRC/name).read_text()
    normalized = re.sub(r'\s+', '', re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)).encode()
    assert sha(normalized) == expected, f'POPFIX source changed: {name}'

for name, expected in {
    'rupture_panel_sprites.h': 'becac33ca92ce1dea07268118aa6be13364ad3919f4390f82ade0389fa3079d3',
    'rupture_hold_sprites.h': 'f4c46311fd4982a0663069801af3547a3337d4f220bd783b38c4a0b71e32204a',
    'rupture_panel_anim.h': '61a06441a2174a8f1535ee44b31bff442e6abc41c65716a6a99c270fb8f87659',
    'rupture_bend_sprites.h': 'acf8d1a0966567d80ff94fa5767e896db71a8ca94439ebd68781e3dd53922781',
    'audio_target.c': '3043710cebb5de5096b22eb2a178585ab6b75d69648b7e085da7a37cb81405ea',
    'ota.c': '81d995be3122e1305cf93410b6331eb8126b04102b94ccb62d5d908618786981',
    'native_panel_midi.c': '29e5d831f453c723e1999cefb95abeddb69d7760c1f1056519fecc2bef90aa2a',
    'ota_target.c': '90b1e2af607b8fc197ac10c6f51384b2d24bd3717b5fea5bb366e6b992b10319',
    'hal/fm1_flash_id.h': 'c77ff4febd1e1efc1cea73e0080aedb7846be431d44dc4983862f3e2c34a0aac',
}.items():
    # V2.0.53 added genuinely byte-counted OTA CHECK/WRITE LCD progress.
    # Those two app-side sources match V2.0.55 byte-for-byte; the protected
    # independent OTA loader and bootheader remain pinned separately.
    # Audio_target V2.0.28 deliberately gates live keys during silent warm-up.
    assert sha((SRC/name).read_bytes()) == expected, f'Unrelated source changed: {name}'

store_source = (SRC/'wave_store_target.c').read_text()
assert 'native_project_cfg.source_version=35u;' in store_source
assert 'ru_fx_store_to_project();' in store_source
assert 'native_project_capture();' in store_source

# Pin unchanged CXS / SUB / EQ code. V2.0.22 updates only the native FX
# dispatch and audio callsite; pin the exact new source alongside the fully
# untouched CXS/SUB source to detect accidental drift in later releases.
for name, expected in {
    'native_fx.c': 'd7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
    'native_cxs_q15.c': '86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
    'rupture_subosc.h': '056a5cf04b79a7ab77966297748ced3eb529d17aae0af1fec28ff0d7b0bfb2e0',
    'native_performance.c': '47d8a7d8d5d9c971deadaf89e0ee94252e041d0195fc378b177b2130fc9e49e0',
    'native_fx.h': '139cfb967b8bcff1ca1236cc0823a1c165ccec64c825c8767fb67bec9498223c',
}.items():
    assert sha((SRC/name).read_bytes()) == expected, f'Audio source changed: {name}'
assert sha((SRC/'audio_core.c').read_bytes()) == '0e5588d24d41f69be5e3065acf4218795a4b55916f448a1b595d984fde7eb235'
assert sha((SRC/'rupture_growl_mip_cache.h').read_bytes()) == '7be268d9414f93ca8b33ed8e204e7f4569d77a8dcab3fa73c76b8fd9ca9ceae4'
assert sha((SRC/'rupture_fx_chain.h').read_bytes()) == 'db9927aad8399301157de624c065882703c990d5bf72fbb15115d95e5f863fe6'

assert 'ru_growl_read_cached(s->cxs.phase,pitch_inc,&growl_mip_cache)' in (SRC/'audio_core.c').read_text()
assert 'ru_growl_make_params(c->cxs_pressure' in (SRC/'audio_core.c').read_text()
assert 'ru_growl_process(&s->growl,growl,&growl_params)' in (SRC/'audio_core.c').read_text()
assert '"GROWL"' in (SRC/'rupture_ui.c').read_text()
assert sha((SRC/'audio_core.h').read_bytes()) == 'a5947492d780c323e5f24e38b126afef6b06084ac46eda7a2431ab8696a62660'
assert sha((SRC/'rupture_growl_waves.h').read_bytes()) == 'd19ee1953eedf0a1e98cf43a9e632fd704fb9bab7eba8d041943ed202e1725aa'

flash_id = (SRC/'hal/fm1_flash_id.h').read_text()
assert '0x856014u' in flash_id and '0xB36014u' in flash_id
assert 'panel_keys&=etch_harmony_live_key_mask(c.octave,&c.harmony)' in (SRC/'audio_target.c').read_text()
bringup = (SRC/'bringup.c').read_text()
assert 'ETCH_OUTPUT_CODEC_PRE_LED_MS' in (SRC/'native_output_startup.h').read_text()
assert 'ru_fx_sync_chain' in bringup and 'ru_fx_migrate_legacy' in bringup
assert 'RU_PAGE_GLIDE' in bringup and 'RU_PAGE_FX_PICK' in bringup
assert 'ru_fx_page_for_selection(native_project_cfg.v190.chain.type[ru_fx_selected])' in bringup
assert 'control==3u' in bringup and 'chain->enabled|=flag' in bringup
atomic_chain = bringup.split('static void ru_fx_sync_chain(void){', 1)[1].split('static void ru_fx_apply_extra(void){', 1)[0]
assert 'fm1_irq_off();' in atomic_chain and 'fm1_irq_enable_all();' in atomic_chain
assert 'native_controls.fx_chain=*c;' in atomic_chain
assert 'ru_mosfet_shaper' in (SRC/'native_fx.c').read_text()
assert 'RU_FX_DISTORTION' in (SRC/'native_fx.c').read_text()
assert 'RU_FX_WAH' in (SRC/'native_fx.c').read_text()
assert 'RU_PAGE_DISTORTION' in bringup and 'RU_PAGE_WAH' in bringup
assert 'RU_FX_REMOVE' in bringup
assert 'rupture_chain_fade[RU_FX_SLOTS]' in (SRC/'native_fx.h').read_text()
assert 'uint16_t shared_fx_done=0u;' in (SRC/'native_fx.c').read_text()
assert 'SAME FX MAY FILL MANY SLOTS' in (SRC/'rupture_tutorial.h').read_text()
ui = (SRC/'rupture_ui.c').read_text()
assert 'ru_center("PORTAMENTO",120,82,1)' in ui
assert '#include "rupture_v2_signal_breach_sprites.h"' in ui
assert '#include "rupture_splash_bitmap.h"' not in ui
assert '#include "rupture_intro_timeline.h"' in ui
assert 'ru_intro_frame_desc(index)' in ui
assert 'RU_INTRO_FRAME_MS' in bringup
assert 'ru_intro_frame_at(elapsed)' in bringup
assert 'ru_sub_fear_play_step(etch_ms,held,' in bringup
assert 'ru_sub1_stare_step(etch_ms,held,' in bringup
assert '&ru_sub_fear_shake' in bringup
assert 'v.sub_fear_frame=ru_sub_fear_frame;' in bringup
assert 'ru_panel_blit_shaken(ru_sub_fear_sprites' in (SRC/'rupture_ui.c').read_text()
assert 'stare?sub1_shake_x[phase]:0' in ui
assert 'stare?sub1_shake_y[phase]:0' in ui
assert 'native_preset_snapshot(native_patch_slot);' in bringup
assert 'ru_autosave_active=automatic;' in bringup
assert 'if(ru_autosave_enabled&&rupture_autosave_due(' in bringup
assert 'native_project_browser_save(0u);' in bringup
assert 'ru_panel_blit_shaken(ru_tunnel_sprites' in ui
assert 'ru_page==RU_PAGE_GLIDE &&' in bringup
assert 'ru_glide_tunnel_play_step(etch_ms,glide_playing' in bringup
assert 'native_audio.voice[vi].active && native_audio.voice[vi].held' in bringup
assert 'ru_panel_blit_shaken(ru_tunnel_sprites' in ui
assert '(v->page==RU_PAGE_GUIDE||v->page==RU_PAGE_KEYLOCK)&&i==2' in ui
assert '"OFF","GUIDE","LOCK","STRICT"' in ui
motion = (SRC/'rupture_glide_tunnel_playback.h').read_text()
assert 'if(!note_held)' in motion and '*shake_tick=0u' in motion and '*frame=0u' in motion
assert 'now_ms-*last_tick_ms)<85u' in motion
assert '(*shake_tick%3u)==0u' in motion
assert 'ru_glide_stage_preset(native_patch_slot);' in bringup
assert 'ru_glide_staged_mask|=1u<<native_patch_slot;' in bringup
assert 'ru_glide_staged_mask=0u;' in bringup
assert 'if(old_time!=ru_note_glide_ms || old_mode!=ru_glide_mode' in bringup
assert 'ru_page!=RU_PAGE_GLIDE)ru_mark_dirty();' in bringup
assert 'native_preset_snapshot(native_patch_slot);' in bringup
assert 'native_project_browser_save(0u);' in bringup
assert '(void)ru_store_project(1u);' in bringup
assert 'MONO PORTAMENTO' not in ui and 'PITCH ONLY / NO EXTRA VOICE' not in ui

for name in ('WebInstaller.html', 'index.html'):
    html = (R/name).read_text()
    firmware_b64 = re.search(r'<script id="firmware"[^>]*>(.*?)</script>', html, re.S).group(1)
    assert base64.b64decode(firmware_b64) == new_fw
    experimental_b64 = re.search(r'<script id="firmware-experimental"[^>]*>(.*?)</script>', html, re.S).group(1)
    assert base64.b64decode(experimental_b64) == experimental
    assert f"const PKG_HASH='{sha(new_fw)}'" in html
    assert f'const PKG_LEN={len(new_fw)}' in html
    assert f"const EXP_PKG_HASH='{sha(experimental)}'" in html
    assert f'const EXP_PKG_LEN={len(experimental)}' in html
    assert '<option value="stable" selected>' in html
    assert '<option value="core2">' in html
    assert 'id="experimental-ack"' in html
    assert "if(pkg.edition==='core2')" in html
    assert 'V2.0.58' in html
    assert 'Stop at the FIRST NUL' in html and 'read-only identity queries' in html
    if name == 'index.html':
        for tag, source in (('fallback-readme','README.md'),
                            ('fallback-changelog','CHANGELOG.md')):
            m=re.search(r'<script id="'+tag+r'"[^>]*>(.*?)</script>',html,re.S)
            assert m and base64.b64decode(m.group(1)) == (R/source).read_bytes()
        m=re.search(r'<script id="site-release-meta"[^>]*>(.*?)</script>',html,re.S)
        site_meta=json.loads(m.group(1))
        assert site_meta['version']=='2.0.58'
        assert site_meta['sha256']==sha(new_fw) and site_meta['bytes']==len(new_fw)

print('PASS V2.0.58 TWO FWSCs decrypted exactly; unchanged loader and dual-choice web installers')
print('PASS trusted OTA, protected bootheader, POPFIX and both flash IDs')
print('PASS new boot sprites, untouched GLIDE and unaffected native subsystems')
