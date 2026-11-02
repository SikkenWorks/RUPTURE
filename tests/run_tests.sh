#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
cc="${CC:-gcc}"
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

# V1.20: pixel-perfect native 1-bit ROM intro, deterministic 24 real frames.
python3 tests/test_v120_boot_art.py

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
