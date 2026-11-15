#!/usr/bin/env python3
"""Package RUPTURE V1.96 with unchanged trusted FM1 OTA/bootheader."""
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
previous = ROOT / 'reference/firmware/RUPTURE-V1.95.fwsc'
assert fw.verify_fwsc(trusted)[1] == []
assert fw.verify_fwsc(previous)[1] == []
assert not fw.validate_app_binary(APP)
assert b'V1.96' in APP
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
target = ROOT / 'RUPTURE-V1.96.fwsc'
target.write_bytes(pkg)
assert fw.verify_fwsc(target)[1] == []
entries = {e['name']: e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert entries['ota.bin'] == prev_entry['ota.bin']
assert entries['flash.bin'][:0x4000] == prev_entry['flash.bin'][:0x4000]
assert entries['flash.bin'] == flash

for name in ('app.bin', 'app.bin.audit.json', 'native-build-options.json'):
    shutil.copy2(APP_DIR / name, ROOT / 'native-build' / name)
options = json.loads((ROOT / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V1.96'
assert options['app_sha256'] == sha(APP)
assert options['ota_writes'] and options['storage_writes']

manifest_path = ROOT / 'RELEASE-MANIFEST.json'
manifest = json.loads(manifest_path.read_text())
manifest.update(
    version='1.96', firmware_file=target.name,
    firmware_bytes=len(pkg), firmware_sha256=sha(pkg),
    app_bytes=len(APP), app_sha256=sha(APP),
    physical_fm1_verified=False, hardware_verified=False, release_ready=False,
    release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',
    release_feature='V1.96 STRICT scale guide: out-of-scale keys dark and silent without audio DSP changes',
    release_note='V1.96 adds STRICT after GUIDE in the UI: OFF, GUIDE, STRICT, LOCK. '
                 'Off-scale physical and MIDI note-ons are rejected, and keys stay dark. '
                 'STRICT stores as keylock 3, so old keylock 2 LOCK projects load unchanged. '
                 'The 27-key mask is calculated once for DSP and once for MIDI output '
                 'per DMA block (only in STRICT); '
                 'the PCM sample processing and sound quality remain unchanged. '
                 'V1.95 PCM samples are identical to V1.94 across 1x/8x of all 10 FX types, '
                 'including slot toggles, reorder, legacy projects and hidden modulation. '
                 'Independent WAHs cache attack/resonance control conversion, '
                 'and WAH uses an exact integer math identity instead of division. '
                 'Every insert retains four separate stored knobs, including duplicates. '
                 'Each nonlinear/small FX has separate realtime state. Delay and '
                 'Chorus share their one original PCM history, with per-slot tap '
                 'time/rate and wet level, and Delay feedback is combined in one '
                 'shared buffer. Reverb uses a shared FDN tank and each '
                 'slot has a separate wet level, but repeats do NOT have separate '
                 'decay/topology states. The first active reverb controls the tank. '
                 'Single-effect PCM is sample-identical after parameter migration. '
                 'No new PCM rings; additional simultaneous processing does use '
                 'additional CPU. Hardware CPU/DMA validation is still required.',
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
manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')

for installer_name in ('WebInstaller.html', 'index.html'):
    installer_path = ROOT / installer_name
    before = installer_path.read_text()
    assert 'Stop at the FIRST NUL' in before and 'read-only identity queries' in before
    assert 'V1.95' in before or 'V1.96' in before
    package_match = re.search(r'(<script id="firmware"[^>]*>)(.*?)(</script>)', before, re.S)
    assert package_match
    old_pkg = base64.b64decode(package_match.group(2))
    # Safe to rerun packaging after a native build fix: an already embedded
    # V1.96 candidate may differ from the final one. Still forbid replacing
    # a payload with an unrelated loader or protected bootheader.
    old_entries = {e['name']: e['data'] for e in fw.parse_ufw(old_pkg)['entries']}
    assert old_entries['ota.bin'] == entries['ota.bin']
    assert old_entries['flash.bin'][:0x4000] == flash[:0x4000]
    old_hash = re.search(r"const PKG_HASH='([0-9a-f]{64})'", before).group(1)
    old_len = re.search(r'const PKG_LEN=(\d+)', before).group(1)
    assert old_hash == sha(old_pkg) and int(old_len) == len(old_pkg)
    after = before.replace('V1.95', 'V1.96')
    after, n = re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',
        lambda m: m.group(1) + base64.b64encode(pkg).decode('ascii') + m.group(2),
        after, count=1, flags=re.S)
    assert n == 1
    after, n = re.subn(r"const PKG_HASH='[0-9a-f]{64}'",
        f"const PKG_HASH='{sha(pkg)}'", after)
    assert n == 1
    after, n = re.subn(r'const PKG_LEN=\d+', f'const PKG_LEN={len(pkg)}', after)
    assert n == 1
    # Remove only the differences a version refresh is allowed to introduce.
    normalized_after = after.replace('V1.96', 'V1.95')
    normalized_after = normalized_after.replace(base64.b64encode(pkg).decode('ascii'), package_match.group(2))
    normalized_after = normalized_after.replace(sha(pkg), old_hash)
    normalized_after = normalized_after.replace(f'const PKG_LEN={len(pkg)}', f'const PKG_LEN={old_len}')
    normalized_before = before.replace('V1.96', 'V1.95')
    assert normalized_after == normalized_before, f'{installer_name}: OTA/connection logic changed'
    assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>', after, re.S).group(1)) == pkg
    installer_path.write_text(after)

print('PASS V1.96 firmware:', len(pkg), sha(pkg))
print('PASS V1.96 app:', len(APP), sha(APP))
print('PASS unchanged OTA loader:', sha(entries['ota.bin']))
print('PASS unchanged protected bootheader:', sha(flash[:0x4000]))
print('PASS both Web MIDI installers retain their original OTA/connection logic')
