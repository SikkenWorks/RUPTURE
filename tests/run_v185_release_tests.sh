#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
cc="${CC:-cc}"
dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT
sh tests/test_v185_realtime.sh
for t in test_v184_hardware_integration test_v1841_led_softstart test_v1842_analog_led_arbiter; do
 "$cc" -std=c11 -O2 -Wall -Wextra -Werror "tests/$t.c" -o "$dir/$t"
 "$dir/$t"
done
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/test_v183_mono_divider.c -o "$dir/mono" && "$dir/mono"
python3 tests/test_v183_audio_parity.py
python3 tests/test_v181_full_audio_parity.py
