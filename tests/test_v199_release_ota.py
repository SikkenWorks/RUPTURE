#!/usr/bin/env python3
"""RUPTURE V1.99: GLIDE note-gate, GUIDE labels, protected OTA and DSP parity."""
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
assert manifest['version'] == '1.99'
assert manifest['firmware_file'] == 'RUPTURE-V1.99.fwsc'
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
assert b'V1.99' in app
options = json.loads((R / 'native-build/native-build-options.json').read_text())
assert options['version'] == 'RUPTURE V1.99'
assert options['storage_writes'] and options['ota_writes']
new_fw = (R / manifest['firmware_file']).read_bytes()
old_fw = (R / 'reference/firmware/RUPTURE-V1.98.fwsc').read_bytes()
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

# The user explicitly requested a GLIDE-only visual update; require exact
# V1.96 source bytes for the audio callback, all FX and the core synth engine.
for name, expected in {
    'audio_core.c': 'bdf74aacaffa4a697def7dc266b3e162fd53baf858887187d72c255a6a632755',
    'native_fx.c': 'ce1477749f765b3e16a9516009fce304f2c8ded9317eec5eb40a47e642803187',
    'native_cxs_q15.c': '8625829401e27711263b2a66dd18cd6180b2b590752f49214b9d036389a851c1',
    'native_performance.c': '47d8a7d8d5d9c971deadaf89e0ee94252e041d0195fc378b177b2130fc9e49e0',
    'native_fx.h': '64d3a77453d32197bf3938cadcc3d191b6b9a6b471d1305c5c34f5e71fa10ed5',
}.items():
    assert sha((SRC/name).read_bytes()) == expected, f'Audio source changed: {name}'

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
    assert 'V1.99' in html
    assert 'Stop at the FIRST NUL' in html and 'read-only identity queries' in html

print('PASS V1.99 FWSC/flash decryption and both web installer payloads')
print('PASS trusted OTA, protected bootheader, POPFIX and both flash IDs')
print('PASS untouched animation sprites and unaffected native subsystems')
