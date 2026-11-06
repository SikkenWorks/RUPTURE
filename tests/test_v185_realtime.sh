#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
cc="${CC:-cc}"
dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT
src='vendor/etch-v1.88-source/firmware/fm1-native'
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -I"$src" -Detch_panel_midi_reset=reference_panel_reset -Detch_panel_midi_update=reference_panel_update -c reference/native_panel_midi-V1.84.2.c -o "$dir/reference.o"
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -I"$src" tests/test_v185_panel_midi_parity.c "$src/native_panel_midi.c" "$dir/reference.o" -o "$dir/panel"
"$dir/panel"
"$cc" -std=c11 -O2 -Wall -Wextra -Werror -I"$src" tests/test_v185_midi_budget.c -o "$dir/budget"
"$dir/budget"
"$cc" -std=c11 -O2 -Wall -Wextra -Wno-unused-function tests/test_native_ui.c -o "$dir/ui"
"$dir/ui"
