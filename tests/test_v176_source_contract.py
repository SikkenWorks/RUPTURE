from pathlib import Path
root=Path(__file__).resolve().parents[1]
n=root/'vendor/etch-v1.88-source/firmware/fm1-native'
s=(n/'bringup.c').read_text()
assert '#define FM1_LED_MID_N 2u' in s and '#define FM1_LED_MID_N_LO 3u' in s
assert 'ru_live_running,native_audio.perf_last_seq' in s
assert 'ru_countin_prekeys&fm1_in.notes' in s
assert 'ru_quantize_rec_step(' in s and 'ru_countin_clamp_first_step(' in s
assert 'ru_countin.force_seq=0u' in s
assert 'native_seq_run=1u' not in s[s.index('if((pressed&(1u<<13)) && ru_page==RU_PAGE_SEQ)'):s.index('/* In live REC ALGORITHM')]
# Fixed hashes from the V1.75 audited source; portable on Windows/Linux,
# without any private /mnt/data path or separate old workspace.
import hashlib
previous={
 'ota.c':'770c90f879f8944db1f5a39b67a28ca5cebba27269bf8360d547ff56da1da04d',
 'native_project_codec.c':'7071116cfb9f20c3c142532ff9f1aea5c37fabb2abac6e6b720a9679f5e2423c',
 'hal/fm1_input.h':'7c3239642816e7cfe75a93518cb7a2d5880d722901d4e9bf88cb7f25bc2d6090',
}
for path,expected in previous.items():
 assert hashlib.sha256((n/path).read_bytes()).hexdigest()==expected,path
print('PASS V1.76: count-in first step, faster half-bright LEDs, RUPTURE-only HAL override, storage/OTA/HAL source parity')
