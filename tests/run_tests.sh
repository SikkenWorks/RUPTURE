#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
cc="${CC:-gcc}"
python3 tests/test_v258_dual_fwsc.py
python3 tests/test_v258_webinstaller_choice.py
python3 tests/test_v258_clean_project.py
python3 tests/test_v258_ota_progress_wiring.py
python3 tests/test_v258_core2_poll_wiring.py
python3 tests/test_v258_poll_backoff_wiring.py
python3 tests/test_v258_click_fastpath_wiring.py
python3 tests/test_v258_gain_fastpath_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
 tests/test_v258_gain_fastpath.c -o /tmp/rupture_v258_gain_fastpath && /tmp/rupture_v258_gain_fastpath
python3 tests/test_v258_core2_fx_wait_ema_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_core2_fx_wait_ema.c -o /tmp/rupture_v258_fx_wait_ema && /tmp/rupture_v258_fx_wait_ema
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all tests/test_v258_click_fastpath.c -o /tmp/rupture_v258_click_fastpath && /tmp/rupture_v258_click_fastpath
if command -v node >/dev/null 2>&1; then node tests/test_v258_ota_progress_runtime.js; fi
python3 tests/test_v258_core2_fx_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror \
 -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_core2_fx_stream.c -o /tmp/rupture_v258_core2_stream && /tmp/rupture_v258_core2_stream
python3 tests/test_v258_led_intro_wiring.py
python3 tests/test_v258_welcome_dsp_wiring.py
python3 tests/test_v258_png_branding.py
python3 tests/test_v258_note_release_wiring.py
python3 tests/test_v258_core2_stability_wiring.py
python3 tests/test_v258_core2_pressure_wiring.py
python3 tests/test_v258_pressure_wiring.py
python3 tests/test_v258_pressure_ui_wiring.py
python3 tests/test_v258_midi_expression_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation -Werror \
 -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_midi_expression.c -o /tmp/rupture_v258_midi_expression && /tmp/rupture_v258_midi_expression
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation -Werror \
 -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_profiles.c -o /tmp/rupture_v258_profiles && /tmp/rupture_v258_profiles
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_profile_persistence.c vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm \
 -o /tmp/rupture_v258_profile_store && /tmp/rupture_v258_profile_store
"$cc" -std=c11 -O2 -w -fsanitize=undefined -fno-sanitize-recover=all \
 tests/test_v258_key_pressure.c -o /tmp/rupture_v258_pressure && /tmp/rupture_v258_pressure
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_touch_persistence.c vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm \
 -o /tmp/rupture_v258_store && /tmp/rupture_v258_store
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Werror \
 -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_core2_pressure_guard.c -o /tmp/rupture_v258_core2_pressure && /tmp/rupture_v258_core2_pressure
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Werror \
 -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_core2_fx_guard.c -o /tmp/rupture_v258_core2_guard && /tmp/rupture_v258_core2_guard
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation -Werror \
 -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_note_release.c -o /tmp/rupture_v258_note_release && /tmp/rupture_v258_note_release
"${CC:-gcc}" -std=c11 -O2 -w -DRUPTURE_BUILD \
 -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_note_release_runtime.c \
 vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_editor.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_wave_fx.c \
 -lm -o /tmp/rupture_v258_note_runtime && /tmp/rupture_v258_note_runtime
"${CC:-gcc}" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_CPU1_FX_LIVE_LAB=1 \
 -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_note_release_runtime.c \
 vendor/etch-v1.88-source/firmware/fm1-native/audio_core_core2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_editor.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_wave_fx.c \
 -lm -o /tmp/rupture_v258_note_core2_runtime && /tmp/rupture_v258_note_core2_runtime
"${CC:-gcc}" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_CPU1_FX_LIVE_LAB=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_core2_fx_pcm.c \
 vendor/etch-v1.88-source/firmware/fm1-native/audio_core_core2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_editor.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_wave_fx.c \
 -lm -o /tmp/rupture_v258_core2_fx_pcm && /tmp/rupture_v258_core2_fx_pcm
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v258_intro.c -o /tmp/rupture_v258_intro && /tmp/rupture_v258_intro
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Werror \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v239_note_fx_lcd_guard.c -o /tmp/rupture_v239_note_fx && /tmp/rupture_v239_note_fx
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Werror \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v239_note_dma_meter.c -o /tmp/rupture_v239_note_dma && /tmp/rupture_v239_note_dma
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Werror \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v239_note_scope_guard.c -o /tmp/rupture_v239_note_scope && /tmp/rupture_v239_note_scope
python3 tests/test_v239_note_dma_wiring.py
python3 tests/test_v239_scope_wiring.py
python3 tests/test_v235_offload_wiring.py
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Werror \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v235_offload_guard.c -o /tmp/rupture_v235_guard && /tmp/rupture_v235_guard
python3 tests/test_v234_live_wiring.py
"${CC:-gcc}" -std=c11 -O2 -Wall -Wextra -Werror -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v234_live_uac_fir.c -o /tmp/rupture_v234_live_uac && /tmp/rupture_v234_live_uac
python3 tests/test_v239_source_parity.py
python3 tests/test_v233_cpu1_wiring.py
cc="${CC:-gcc}"
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v233_cpu1_fir_probe.c -o /tmp/rupture_v233_cpu1_fir && /tmp/rupture_v233_cpu1_fir
python3 tests/test_v232_cpu1_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v232_cpu1_protocol.c -o /tmp/rupture_v232_c1proto && /tmp/rupture_v232_c1proto
python3 tests/test_v231_usb_rate_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v231_usb_descriptor.c -o /tmp/rupture_v231_usb_descriptor && /tmp/rupture_v231_usb_descriptor
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v231_usb_preference.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v231_usb_preference && /tmp/rupture_v231_usb_preference
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v230_uac_packed_pcm.c -o /tmp/rupture_v230_uac_packed && /tmp/rupture_v230_uac_packed
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v230_scope_guard.c -o /tmp/rupture_v230_scope_guard && /tmp/rupture_v230_scope_guard
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v228_fx_fade_math.c -o /tmp/rupture_v228_fade_math && /tmp/rupture_v228_fade_math
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v228_note_warmup_gate.c -o /tmp/rupture_v228_note_gate && /tmp/rupture_v228_note_gate
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror -DRUPTURE_BUILD -DRU_FX_FADE_64_REF=1 \
 tests/test_v228_mosfet_distortion_reverb.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -lm -o /tmp/rupture_v228_mdr_ref
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror -DRUPTURE_BUILD \
 tests/test_v228_mosfet_distortion_reverb.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -lm -o /tmp/rupture_v228_mdr_candidate
v228_mdr_ref=$(/tmp/rupture_v228_mdr_ref)
v228_mdr_candidate=$(/tmp/rupture_v228_mdr_candidate)
[ "$v228_mdr_ref" = "$v228_mdr_candidate" ] || { printf 'V2.0.28 MOSFET/DIST/REVERB PCM mismatch\n'; exit 1; }
printf 'PASS V2.0.28 MOSFET/DIST/REVERB/SUB1 exact: %s\n' "$v228_mdr_candidate"
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror -DRUPTURE_BUILD \
 -DRU_FX_FADE_64_REF=1 tests/test_v226_fx_combinations.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -lm -o /tmp/rupture_v226_matrix_reference
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror -DRUPTURE_BUILD \
 tests/test_v226_fx_combinations.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -lm -o /tmp/rupture_v226_matrix_candidate
v226_ref=$(/tmp/rupture_v226_matrix_reference | sed -n '1p')
v226_candidate=$(/tmp/rupture_v226_matrix_candidate | sed -n '1p')
[ "$v226_ref" = "$v226_candidate" ] || { printf 'V2.0.28 PCM regression: %s != %s\n' "$v226_ref" "$v226_candidate"; exit 1; }
printf 'PASS V2.0.28 sample-exact FX combinations: %s\n' "$v226_candidate"
"$cc" -std=c11 -O2 -Wall -Wextra -Werror \
 tests/test_v225_growl_mip_cache.c -o /tmp/rupture_v225_mip && /tmp/rupture_v225_mip
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v224_distortion_coeff_pcm.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v224_distortion && /tmp/rupture_v224_distortion
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v223_fx_control_cache_pcm.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v223_fx_cache && /tmp/rupture_v223_fx_cache
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v222_packed_fx_pcm.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v222_packed_fx && /tmp/rupture_v222_packed_fx
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v221_fx_span_pcm.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v221_fx_span && /tmp/rupture_v221_fx_span
python3 tests/test_v221_fx_span_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v220_preset_audit.c -o /tmp/rupture_v220_audit && /tmp/rupture_v220_audit
python3 tests/test_v220_preset_audit_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra src/cxs.c tests/test_cxs.c -lm -o /tmp/rupture_test_cxs && /tmp/rupture_test_cxs
"$cc" -std=c11 -O2 -Wall -Wextra src/native_cxs_q15.c tests/test_q15.c -o /tmp/rupture_test_q15 && /tmp/rupture_test_q15
"$cc" -std=c11 -O2 -Wall -Wextra src/rupture_data.c src/rupture_midi.c tests/test_data_midi.c -o /tmp/rupture_test_data_midi && /tmp/rupture_test_data_midi
"$cc" -std=c11 -O2 -Wall -Wextra src/rupture_music.c tests/test_music.c -o /tmp/rupture_test_music && /tmp/rupture_test_music
"$cc" -std=c11 -O2 -Wall -Wextra src/rupture_first_boot.c src/rupture_data.c tests/test_first_boot.c -o /tmp/rupture_test_first_boot && /tmp/rupture_test_first_boot
"$cc" -std=c11 -O2 -Wall -Wextra -Werror src/rupture_note_memory.c tests/test_note_memory.c -o /tmp/rupture_test_note_memory && /tmp/rupture_test_note_memory
python3 tests/export_native_screens.py

"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function tests/test_native_ui.c -o /tmp/rupture_native_ui && /tmp/rupture_native_ui

# Native compressed project pattern archive test, no simulated flash writes.
"$cc" -std=c11 -O2 -w -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_native_sequence_slots.c vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c -o /tmp/rupture_test_native_sequence_slots
/tmp/rupture_test_native_sequence_slots

# V0.9: Four active post-mix effects, channel EQ and battery ADC threshold guards.
"$cc" -std=c11 -O2 -Wno-unused-function -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v09_fx.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -o /tmp/rupture_test_v09_fx
/tmp/rupture_test_v09_fx
"$cc" -std=c11 -O2 tests/test_v09_battery.c -o /tmp/rupture_test_v09_battery
/tmp/rupture_test_v09_battery
# RUPTURE-specific K4 HIGH AIR boost and real post-EQ DSP behavior.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -Wno-unused-function -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v11_highcut.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_test_highcut
/tmp/rupture_test_highcut

"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_trs_midi.c -o /tmp/rupture_test_trs && /tmp/rupture_test_trs

"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_cxs_patch_state.c -o /tmp/rupture_test_cxs_patch && /tmp/rupture_test_cxs_patch

# End-to-end native legacy-sized flash codec for new CXS macro markers.
"$cc" -std=c11 -O2 -w -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_cxs_project_roundtrip.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_cxs_codec && /tmp/rupture_cxs_codec
# V1.3 full CXS native LFO+shaping sample renders; no graphics-only settings.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v13_lfo_shape.c vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c -o /tmp/rupture_v13_lfo_shape
/tmp/rupture_v13_lfo_shape
# V1.4: exact per-preset storage without expanding PJT32 records.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v14_sound_shape_storage.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v14_shape_storage
/tmp/rupture_v14_shape_storage

# V1.5: native-owned flash A/B test with twelve catalog slots and old four-slot migration.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v15_twelve_projects.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v15_flash
/tmp/rupture_v15_flash
# V1.6: Shared actual native mono policy and per-sample glide mathematics.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v16_mono_policy.c -o /tmp/rupture_v16_mono && /tmp/rupture_v16_mono

"$cc" -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation tests/test_v17_seq_autosave.c -o /tmp/rupture_v17_autosave && /tmp/rupture_v17_autosave

"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v18_named_libraries.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v18_names && /tmp/rupture_v18_names

# V1.9: dual suboscillator Q15 engine, independent pitch and gain headroom.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v19_dual_sub.c vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c -o /tmp/rupture_v19_sub && /tmp/rupture_v19_sub

"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror tests/test_v110_sub_mix_ui.c vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c -o /tmp/rupture_v110_sub_mix_ui
/tmp/rupture_v110_sub_mix_ui

# V1.11: dual-sub per-preset PJT32 roundtrip, old-project bypass, invalid tags.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v111_sub_storage.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v111_sub_storage
/tmp/rupture_v111_sub_storage
# V1.12: native PJT32 project snapshot persistence for Note Memory + Master.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v112_performance_memory.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v112_performance
/tmp/rupture_v112_performance
# V1.13: separate device-global master volume NOR journal, across projects.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v113_global_master.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v113_global
/tmp/rupture_v113_global
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -DRUPTURE_BUILD \
 tests/test_v114_fx_bypass.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -o /tmp/rupture_v114_fx_bypass
/tmp/rupture_v114_fx_bypass
# V1.15: compressor and folder exposed; three additional state-only DSP effects.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -Wall -Wextra -Wno-unused-function \
 tests/test_v115_five_fx.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v115_five_fx
/tmp/rupture_v115_five_fx
# V1.16: Sandpaper native DSP spectral/softness/bypass regression.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -Wall -Wextra -Wno-unused-function \
 tests/test_v116_sandpaper.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v116_sandpaper
/tmp/rupture_v116_sandpaper
# V1.17: project-scoped effect parameters + all FX bypass bits in PJT33.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v117_fx_memory.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v117_fx_memory
/tmp/rupture_v117_fx_memory

# V1.17: full 12-slot NOR flash journal + PJT33 FX memory + torn writes.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v117_flash_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v117_flash_fx
/tmp/rupture_v117_flash_fx
# V1.18: actual all-ten-FX maximum-parameter PCM headroom stress.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -Wall -Wextra -Wno-unused-function \
 tests/test_v118_full_fx_stress.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v118_stress
/tmp/rupture_v118_stress

# V1.19: per-phase on-device read-only DMA diagnostics and safe scope deferral.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v119_dma_diagnostics.c -o /tmp/rupture_v119_diag
/tmp/rupture_v119_diag

# V2.0.1: exact 1-bit source art and eleven reproducible in-between frames.
python3 tests/test_v201_boot_art.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v202_subosc_playback.c -o /tmp/rupture_v202_sub_fear && /tmp/rupture_v202_sub_fear
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v203_subosc_stare.c -o /tmp/rupture_v203_sub_stare && /tmp/rupture_v203_sub_stare
python3 tests/test_v202_subosc_art.py

# V1.21: shared firmware SAVE routing, including both SUB HOME pages.
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -Werror tests/test_v121_save_home.c -o /tmp/rupture_v121_save_home
/tmp/rupture_v121_save_home

# V1.22: individual FX response, fuzz loudness and Sandpaper click regression.
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function -DRUPTURE_BUILD \
 tests/test_v122_fx_quality.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v122_quality
/tmp/rupture_v122_quality

"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function tests/test_v122_subblend.c vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c -o /tmp/rupture_v122_subblend
/tmp/rupture_v122_subblend

# V1.23: bounded step sustain, 10 scrollable HELP cards, project+sequence names and duration persistence.
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation -Werror tests/test_v123_seq_sustain.c -o /tmp/rupture_v123_sustain && /tmp/rupture_v123_sustain
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation -Wno-unused-function -Werror tests/test_v123_help_cards.c -o /tmp/rupture_v123_help && /tmp/rupture_v123_help
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v123_name_sustain_storage.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v123_storage && /tmp/rupture_v123_storage
# V1.24: genuine classic post-filter path, dual-sub gain slews, all active FX stress.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -Wall -Wextra -Wno-unused-function tests/test_v124_sub_fx_headroom.c vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v124_audio && /tmp/rupture_v124_audio
"$cc" -std=c11 -O2 -Wno-unused-function tests/test_v124_ui_spacing.c -o /tmp/rupture_v124_ui && /tmp/rupture_v124_ui
# V1.25: every user-facing fix wired in the native source, plus real I/O phases
# exercised by the twelve-slot NOR emulator above.
python3 tests/test_v125_runtime_integrity.py

# V1.26: physical controls semantics, name-entry double-press guard, actual OTA stage percentage.
python3 tests/test_v126_workflow.py
"$cc" -std=c11 -O2 -Wno-unused-function tests/test_v126_workflow.c -o /tmp/rupture_v126_workflow
/tmp/rupture_v126_workflow

# V1.27: device-global setting survives all global volume / project flows.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v127_global_settings.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v127_settings
/tmp/rupture_v127_settings
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function tests/test_v127_native_ui.c -o /tmp/rupture_v127_ui && /tmp/rupture_v127_ui
python3 tests/test_v127_source_contract.py

# V1.28: persistent global preferences, bounded legacy storage wipe,
# native project catalogs, packed sequence edits and pixel-accurate UI.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v128_storage.c vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_test_v128_storage
/tmp/rupture_test_v128_storage
"$cc" -std=c11 -O2 -w tests/test_v128_ui.c -o /tmp/rupture_test_v128_ui
/tmp/rupture_test_v128_ui
"$cc" -std=c11 -O2 -w tests/test_v128_sequences.c vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c -lm -o /tmp/rupture_test_v128_sequences
/tmp/rupture_test_v128_sequences

# V1.29 / V1.30 UI and native live catalog / legacy sequence regression.
"$cc" -std=c11 -O2 -Wno-unused-function tests/test_v129_ui.c -o /tmp/rupture_v129_ui && /tmp/rupture_v129_ui
python3 tests/test_v129_settings_contract.py
"$cc" -std=c11 -O2 -Wno-unused-function tests/test_v130_ui.c -o /tmp/rupture_v130_ui && /tmp/rupture_v130_ui
python3 tests/test_v130_source_contract.py
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v130_legacy_sequences.c vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v130_legacy && /tmp/rupture_v130_legacy

# V1.32 / V1.33: screensaver controls; full per-sequence archive persistence
# and UI active-slot markers in the native sources.
"$cc" -std=c11 -O2 -Wno-unused-function tests/test_v132_ui.c -o /tmp/rupture_v132_ui && /tmp/rupture_v132_ui
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v133_sequence_roundtrip.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v133_roundtrip
/tmp/rupture_v133_roundtrip
python3 tests/test_v133_source_contract.py

# V1.34 true RBJ EQ2 audio, Q bandwidth, fixed headroom and PJT34 storage.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -D_DEFAULT_SOURCE -Wno-unused-function -Wall -Wextra -Werror \
 tests/test_v134_eq2.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v134_eq2 && /tmp/rupture_v134_eq2
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -D_DEFAULT_SOURCE -Wno-unused-function -Wall -Wextra -Werror \
 tests/test_v134_eq2_engine.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v134_eq2_engine && /tmp/rupture_v134_eq2_engine
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v134_eq2_storage.c vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v134_storage && /tmp/rupture_v134_storage
# V1.38: exact reference comparisons for cached native LP/BP/HP/LADDER coefficients.
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation tests/test_v138_all_filters.c -o /tmp/rupture_v138_all_filters
/tmp/rupture_v138_all_filters

# V1.41: EQ2 headroom and bypass correctness, cross-band changes and clipping guard.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -D_DEFAULT_SOURCE -Wno-unused-function \
 tests/test_v141_eq2_regressions.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v141_eq2 && /tmp/rupture_v141_eq2

# V1.42: OCTAVE FUZZ must remain silent at zero, click-free on extreme controls,
# exactly dry after OFF, and free of variable-integer division in its IRQ loop.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD -Wno-unused-function \
 tests/test_v142_fuzz_stability.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v142_fuzz && /tmp/rupture_v142_fuzz
python3 tests/test_v142_reciprocal.py

# V1.43: cross-firmware OTA staging and stock bootheader safety.
fwsc_name=$(python3 -c 'import json;print(json.load(open("RELEASE-MANIFEST.json"))["firmware_file"])')
"$cc" -std=c11 -O2 -Wno-unused-function tests/test_v143_ota_reclaim.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_digest.c -o /tmp/rupture_v143_ota
/tmp/rupture_v143_ota "$fwsc_name"

# V1.44: official V15 eight-entry container native replay, when the exact
# upstream vendor file and prior RUPTURE build are provided by the test runner.
# Never ship proprietary stock binaries as part of the source ZIP.
if [ -n "${RUPTURE_STOCK_V15_FWSC:-}" ] && [ -n "${RUPTURE_V143_FWSC:-}" ]; then
 "$cc" -std=c11 -O2 -Wno-unused-function tests/test_v144_stock_v15.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_digest.c -o /tmp/rupture_v144_stock_v15
 /tmp/rupture_v144_stock_v15 "$RUPTURE_STOCK_V15_FWSC" "$RUPTURE_V143_FWSC"
fi

# V1.45: banklimited integer oscillators, backwards-compatible PJT34 state,
# and UI-only reverse/pause/loop screensaver from the same native code.
"$cc" -std=c11 -O2 -DRUPTURE_BUILD tests/test_v145_osc_saver.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c -o /tmp/rupture_v145_osc && /tmp/rupture_v145_osc
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v145_project_roundtrip.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v145_codec && /tmp/rupture_v145_codec

# V1.48: ALL ordinary page SAVE routing and alias guard, including UBSan.
"$cc" -std=c11 -O2 -Wno-unused-function tests/test_v148_save_global.c -o /tmp/rupture_v148_save && /tmp/rupture_v148_save
"$cc" -std=c11 -O2 -fsanitize=undefined -fno-sanitize-recover=all tests/test_v148_saw_alias.c -o /tmp/rupture_v148_saw && /tmp/rupture_v148_saw

# V1.54: Exact original PNG pixel parity + dedicated 20-frame UI animator.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_dam_anim.c -o /tmp/rupture_v154_dam_anim && /tmp/rupture_v154_dam_anim
python3 tests/test_dam_sprites.py

# V1.55: Native CXS PRESSURE must alter the sustained audio at all dial regions,
# retain exact oscillator period, and survive live sweeps and extreme settings.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
 tests/test_v155_cxs_pressure.c vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -o /tmp/rupture_v155_pressure && /tmp/rupture_v155_pressure

# V1.60: first-run confirmation is legible and SEL-acknowledged; V1.59 NOR storage continuity.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v160_welcome.c -o /tmp/rupture_v160_welcome && /tmp/rupture_v160_welcome
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v159_welcome_storage.c vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v159_store && /tmp/rupture_v159_store

# V1.63: real 240x240 UI rendering and RAM-only undo gesture policy.
"$cc" -std=c11 -O2 tests/test_v163_edit_safety.c -o /tmp/rupture_v163_edit_safety
/tmp/rupture_v163_edit_safety

# V1.68: step 1/zero-index note removal, directed HOLD cleanup,
# unmodified other steps/locks, audio-led to LCD playhead synchronization.
"$cc" -std=c11 -O2 tests/test_v168_seq_entry_playhead.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 -o /tmp/rupture_v168_seq_entry && /tmp/rupture_v168_seq_entry
"$cc" -std=c11 -O2 tests/test_v168_seq_renderer.c -lm \
 -o /tmp/rupture_v168_seq_render && /tmp/rupture_v168_seq_render

# V1.74: complete live sustain, lossless 256-compatible circular rotation,
# separate stored first-run wizard vs returning welcome, control docs.
"$cc" -std=c11 -O2 -w tests/test_v174_seq_shift_sustain.c vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c -o /tmp/rupture_v174_seq && /tmp/rupture_v174_seq
"$cc" -std=c11 -O2 -w tests/test_v174_welcome.c -lm -o /tmp/rupture_v174_welcome && /tmp/rupture_v174_welcome
python3 tests/test_v173_mapping_docs.py

# V1.76: real first downbeat after countdown, timestamp-aware nearest-step
# quantization, lossless sustain with future/past snapped notes, LED scan
# frequency, native OTA/storage/HAL compatibility.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v176_countin_bridge.c -o /tmp/rupture_v176_first && /tmp/rupture_v176_first
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation tests/test_v176_quant_led.c vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c -o /tmp/rupture_v176_quant && /tmp/rupture_v176_quant
python3 tests/test_v176_source_contract.py

# V1.78: distinct global DSP topologies, compatible legacy project decoder,
# and project/preset reverb-type retention without expanded storage records.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v178_three_reverbs.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v178_engines && /tmp/rupture_v178_engines
# V1.79: no signal-quality compromises. Compare original V1.78 FX source
# against new source PCM; verify exact per-sample SUB DRIFT and audio-priority UI.
python3 tests/test_v179_pcm_parity.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v179_drift_exact.c vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c -o /tmp/rupture_v179_sub && /tmp/rupture_v179_sub
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v179_ui_headroom.c -o /tmp/rupture_v179_ui && /tmp/rupture_v179_ui

# V1.82 first-boot migration and NOR integrity regression
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v182_welcome_version.c vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v182_store && /tmp/rupture_v182_store

# V1.88.3: complete 20-frame ENV opening, 8 smoke-settling hold frames,
# fixed exposed hole and pixel-exact reverse on note release. OTA is intact.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v1883_env_animation.c -o /tmp/rupture_v1883_env && /tmp/rupture_v1883_env
python3 tests/test_v165_png_parity.py
python3 tests/test_v170_hold_png_parity.py
python3 tests/test_v210_release_ota.py

# V2.0.9: native paired-register SUB pitch multiplication must remain
# exact for ALL normal/saturating detune, drift and octave combinations.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v209_sub_inc_parity.c -o /tmp/rupture_v209_sub_inc && /tmp/rupture_v209_sub_inc
python3 tests/test_v210_target_codegen.py
# V2.0.11: reference harmonic shape, bandlimited mips, live GROWL motion,
# continuous sustained PCM and bounded output. SUB SAW and CXS remain old.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v211_growl.c \
 -lm -o /tmp/rupture_v211_growl && /tmp/rupture_v211_growl
# V2.0.12: extreme expressiveness (all eight knobs), 1536 held-note cases,
# output headroom, and block-cached parameter conversion.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v212_growl_expression.c \
 -lm -o /tmp/rupture_v212_growl && /tmp/rupture_v212_growl
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
 tests/test_v213_expression.c -o /tmp/rupture_v213_expression && /tmp/rupture_v213_expression
python3 tests/test_v213_knob_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v214_growl_expression.c \
 -lm -o /tmp/rupture_v214_growl && /tmp/rupture_v214_growl
# V2.0.15: revised reference harmonics, 0..100 vowel expression,
# amplitude headroom across four notes, block-level motion transient.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v215_growl_reference.c \
 -lm -o /tmp/rupture_v215_growl && /tmp/rupture_v215_growl
# V2.0.16: deterministic free-running LFO, smooth timbre drift,
# manual priority, continuous GROWL PCM and preserved quiet expression.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v216_growl_auto.c \
 -lm -o /tmp/rupture_v216_growl && /tmp/rupture_v216_growl
# V2.0.10: minimum EQ frequency, negative/positive gain and HIGH AIR must
# bypass the empty FX rack and preserve the V2.0.9 guarded EQ response.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -D_DEFAULT_SOURCE \
 tests/test_v210_eq_pcm_benchmark.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v210_eq && /tmp/rupture_v210_eq
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -D_DEFAULT_SOURCE \
 tests/test_v210_sub_eq_reverb_stress.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -lm -o /tmp/rupture_v210_stress && /tmp/rupture_v210_stress
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -D_DEFAULT_SOURCE \
 tests/test_v211_growl_eq_reverb_stress.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -lm -o /tmp/rupture_v211_growl_stress && /tmp/rupture_v211_growl_stress
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -D_DEFAULT_SOURCE \
 tests/test_v213_expression_eq_reverb_stress.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -lm -o /tmp/rupture_v213_expression_stress && /tmp/rupture_v213_expression_stress
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v206_led_wave.c -o /tmp/rupture_v206_wave && /tmp/rupture_v206_wave
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function tests/test_v206_popup_ui.c -o /tmp/rupture_v206_popup && /tmp/rupture_v206_popup
python3 tests/test_v206_preset_wiring.py
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v207_preset_runtime.c -o /tmp/rupture_v207_preset && /tmp/rupture_v207_preset
python3 tests/test_v207_reachable_input.py
# V2.0.8: compare every PCM sample and the complete CXS state against the
# actual V2.0.7 source, including oscillator-cycle exchanges and live edits.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 -Drupture_q15_init=old_rupture_q15_init -Drupture_q15_params=old_rupture_q15_params \
 -Drupture_q15_sample=old_rupture_q15_sample -Drupture_q15_valid=old_rupture_q15_valid \
 -c reference/native_cxs_q15-V2.0.7.c -o /tmp/rupture_v208_cxs_old.o
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v208_cxs_exchange_pcm.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 /tmp/rupture_v208_cxs_old.o -o /tmp/rupture_v208_cxs_pcm && /tmp/rupture_v208_cxs_pcm
"$cc" -std=c11 -O3 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v208_mixed_reverb_pcm.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v208_mixed_pcm && /tmp/rupture_v208_mixed_pcm
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -Wno-unused-function tests/test_v205_cxs_note_start.c -o /tmp/rupture_v205_cxs && /tmp/rupture_v205_cxs
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -Wno-unused-function tests/test_v205_preset_menu.c -o /tmp/rupture_v205_menu && /tmp/rupture_v205_menu
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v205_preset_global.c vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v205_global && /tmp/rupture_v205_global
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -DETCH_P05_MULTI=1 -DETCH_STORE_OWNED_ENABLED=1 \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v213_dual_knob_storage.c vendor/etch-v1.88-source/firmware/fm1-native/wave_store.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm \
 -o /tmp/rupture_v213_knob && /tmp/rupture_v213_knob
bash tests/pop_fix/run_pop_tests.sh

# V1.90: ordered fixed-memory insert chain, per-patch GLIDE and step SLIDE,
# PJT35 write/read compatibility with the complete PJT34 archive.
python3 tests/test_v217_capture_wiring.py
python3 tests/test_v219_preset_load_wiring.py
"$cc" -std=c11 -O3 -Wall -Wextra -Werror \
 tests/test_v219_preset_load_rt.c -o /tmp/rupture_v219_preset_rt && /tmp/rupture_v219_preset_rt
"$cc" -std=c11 -O3 -Wall -Wextra -Werror -DRUPTURE_BUILD \
 tests/test_v218_sub_realtime_expression.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 -lm -o /tmp/rupture_v218_sub && /tmp/rupture_v218_sub
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v217_growl_sub_distortion_save.c vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm \
 -o /tmp/rupture_v217_coldboot && /tmp/rupture_v217_coldboot
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v190_fx_chain.c vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -o /tmp/rupture_v190_fx && /tmp/rupture_v190_fx
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v190_glide.c -o /tmp/rupture_v190_glide && /tmp/rupture_v190_glide
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v204_glide_fx_order.c -o /tmp/rupture_v204_glide_fx && /tmp/rupture_v204_glide_fx
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v190_project_roundtrip.c vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c -lm -o /tmp/rupture_v190_codec && /tmp/rupture_v190_codec

# V1.91: test the fixed-size ROM MOSFET shaper under UBSan and the full eight-FX
# audio loop with runtime reorder, enable/disable and bounded dry recovery.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
 tests/test_v191_mosfet_rt.c -o /tmp/rupture_v191_mosfet && /tmp/rupture_v191_mosfet
"$cc" -std=c11 -O2 -w -fsanitize=undefined -fno-sanitize-recover=all -DRUPTURE_BUILD \
 -Ivendor/etch-v1.88-source/firmware/fm1-native tests/test_v191_full_fx_rt.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -o /tmp/rupture_v191_full_fx && /tmp/rupture_v191_full_fx

# V1.92: ten distinct algorithms in eight slots; Distortion and auto-Wah
# produce real, reorderable PCM and persist knobs in the existing PJT35 codec.
"$cc" -std=c11 -O2 -w -fsanitize=undefined -fno-sanitize-recover=all \
 -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v192_distortion_wah.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v192_fx && /tmp/rupture_v192_fx
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v192_project_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v192_codec && /tmp/rupture_v192_codec

# V1.93: same effect may occupy all eight slots. Large time FX advance
# their global PCM tank once/frame; every placement still changes output.
"$cc" -std=c11 -O2 -w -fsanitize=undefined -fno-sanitize-recover=all \
 -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v193_repeat_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v193_repeat && /tmp/rupture_v193_repeat

"$cc" -std=c11 -O2 -w -fsanitize=undefined -fno-sanitize-recover=all \
 -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v194_independent_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v194_independent && /tmp/rupture_v194_independent

"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v194_unique_pcm_parity.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v194_parity && /tmp/rupture_v194_parity

# Exact V1.94 PCM oracle: each of the ten FX types in 1 and 8 slots,
# each with independent parameter banks; bypass + reorder while sounding.
"$cc" -std=c11 -O3 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v195_pcm_oracle.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v195_oracle_test && /tmp/rupture_v195_oracle_test

"$cc" -std=c11 -O3 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v195_legacy_hidden_pcm.c \
 reference/native_fx-V2.0.3.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -DRU_V204_BASELINE -lm -o /tmp/rupture_v195_hidden_baseline && /tmp/rupture_v195_hidden_baseline

"$cc" -std=c11 -O3 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v195_legacy_hidden_pcm.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v195_hidden_test && /tmp/rupture_v195_hidden_test

"$cc" -std=c11 -O3 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v195_live_wah_knobs.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v195_knobs_test && /tmp/rupture_v195_knobs_test

# V1.96: new STRICT after GUIDE, legacy LOCK compatibility, LEDs, hardware
# key suppression, MIDI note suppression and unaltered audible GUIDE notes.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v196_strict_scale.c \
 vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 vendor/etch-v1.88-source/firmware/fm1-native/wave_editor.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_wave_fx.c \
 -lm -o /tmp/rupture_v196_strict && /tmp/rupture_v196_strict

# V1.97: GLIDE-only camera shake, bit-exact 224x30 panels and visual isolation.
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function \
 tests/test_v197_glide_tunnel.c -o /tmp/rupture_v197_glide && /tmp/rupture_v197_glide

# V1.98: immediate per-preset GLIDE staging and the existing manual/autosave
# PJT35 codec roundtrip, without any new storage records or audio work.
"$cc" -std=c11 -O2 -w -DRUPTURE_BUILD -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/test_v198_glide_save_autosave.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_pattern_bank.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_performance.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v198_glide_save && /tmp/rupture_v198_glide_save

# V1.99: GLIDE sprites/camera freeze without a sounding note; resume on a
# real voice gate, plus exact 1-bit GUIDE text in the advertised mode order.
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function \
 tests/test_v199_glide_gate_guide_labels.c \
 -o /tmp/rupture_v199_glide_guide && /tmp/rupture_v199_glide_guide
