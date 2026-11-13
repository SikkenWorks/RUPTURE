#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
src=vendor/etch-v1.88-source/firmware/fm1-native
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
cc -std=c11 -O2 -DRUPTURE_BUILD=1 -I"$src" tests/test_v186_bend_audio.c \
 "$src/audio_core.c" "$src/native_fx.c" "$src/native_cxs_q15.c" \
 "$src/rupture_eq2.c" "$src/wave_editor.c" "$src/native_performance.c" \
 "$src/native_wave_fx.c" -lm -o "$tmp/bend-audio"
"$tmp/bend-audio"
