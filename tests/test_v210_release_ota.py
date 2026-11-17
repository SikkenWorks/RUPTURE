#!/usr/bin/env python3
"""RUPTURE V2.0.14: retain EQ hotfix and trusted OTA with expression return."""
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
assert manifest['version'] == '2.0.14'
assert manifest['firmware_file'] == 'RUPTURE-V2.0.14.fwsc'
assert manifest['growl_performance_v214']['assignment_ids'] == [94, 95]
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
assert b'V2.0.14' in app
options = json.loads((R / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V2.0.14'
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
    'audio_target.c': '4181b7fbc35f1fdcef68c99c0f44b42234ceb8a3c0faadf535fb40728d44e2d4',
    'ota.c': 'c77217b9a650beda39827a2754e76a8502ee97c8244ce07087e05c87dc9e34e6',
    'native_panel_midi.c': '29e5d831f453c723e1999cefb95abeddb69d7760c1f1056519fecc2bef90aa2a',
    'usb.c': '3e484359d55c68e7510b3b588315e1e781ca30d3946f7d0fea4288895cb6dbef',
    'ota_target.c': '1d9a9ceb0489d98211d9757eafcd9ed021b9f7d57efa4e0ef3631824f2a44c5a',
    'wave_store_target.c': '2027c9d68ddc801da4541c6fbfd1339824c51db2699ffbb4ffa7124459131813',
    'hal/fm1_flash_id.h': 'c77ff4febd1e1efc1cea73e0080aedb7846be431d44dc4983862f3e2c34a0aac',
}.items():
    # OTA and wave-store source hashes may be refreshed by targeted source tests,
    # but native startup artwork, runtime identity and flash IDs must stay intact.
    assert sha((SRC/name).read_bytes()) == expected, f'Unrelated source changed: {name}'

# Pin unchanged CXS / SUB / FX / EQ code. audio_core.c intentionally
# changes only MAIN GROWL coefficient caching and the new processor call.
for name, expected in {
    'native_fx.c': '14ee2663a0d89d2eff1eeae93fdfe355b28fefd747bcd90c68a487fc6d03b578',
    'native_cxs_q15.c': '86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
    'rupture_subosc.h': '056a5cf04b79a7ab77966297748ced3eb529d17aae0af1fec28ff0d7b0bfb2e0',
    'native_performance.c': '47d8a7d8d5d9c971deadaf89e0ee94252e041d0195fc378b177b2130fc9e49e0',
    'native_fx.h': '4548bbd8df9b6bbdc8b2db55de63a329d58a86323ff7f5935786c1dc6f27911a',
}.items():
    assert sha((SRC/name).read_bytes()) == expected, f'Audio source changed: {name}'

assert 'ru_growl_read(s->cxs.phase,pitch_inc)' in (SRC/'audio_core.c').read_text()
assert 'ru_growl_make_params(c->cxs_pressure' in (SRC/'audio_core.c').read_text()
assert 'ru_growl_process(&s->growl,growl,&growl_params)' in (SRC/'audio_core.c').read_text()
assert '"GROWL"' in (SRC/'rupture_ui.c').read_text()
assert sha((SRC/'audio_core.h').read_bytes()) == 'a5947492d780c323e5f24e38b126afef6b06084ac46eda7a2431ab8696a62660'
assert sha((SRC/'rupture_growl_waves.h').read_bytes()) == '812f8a09aa9a78af3c51cbcbf882647aa25781587b0140e560560ab81e7efc39'

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
assert 'ru_v2_breach_decode(ru_bits,frame)' in ui
assert 'ru_v2_breach_ms[f]' in bringup
assert 'ru_v2_breach_frame_at(elapsed)' in bringup
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
    assert f"const PKG_HASH='{sha(new_fw)}'" in html
    assert f'const PKG_LEN={len(new_fw)}' in html
    assert 'V2.0.14' in html
    assert 'Stop at the FIRST NUL' in html and 'read-only identity queries' in html
    if name == 'index.html':
        for tag, source in (('fallback-readme','README.md'),
                            ('fallback-changelog','CHANGELOG.md')):
            m=re.search(r'<script id="'+tag+r'"[^>]*>(.*?)</script>',html,re.S)
            assert m and base64.b64decode(m.group(1)) == (R/source).read_bytes()
        m=re.search(r'<script id="site-release-meta"[^>]*>(.*?)</script>',html,re.S)
        site_meta=json.loads(m.group(1))
        assert site_meta['version']=='2.0.14'
        assert site_meta['sha256']==sha(new_fw) and site_meta['bytes']==len(new_fw)

print('PASS V2.0.14 FWSC/flash decryption and both web installer payloads')
print('PASS trusted OTA, protected bootheader, POPFIX and both flash IDs')
print('PASS new boot sprites, untouched GLIDE and unaffected native subsystems')
