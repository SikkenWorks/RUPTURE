#!/usr/bin/env python3
"""Welcome Back returns after intro; DSP MAX excludes one-time muted boot load."""
from pathlib import Path

src=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native'
bringup=(src/'bringup.c').read_text()
target=(src/'audio_target.c').read_text()
host=(src/'rupture_host_export.c').read_text()
gif=(src.parents[3]/'tools/render_v258_intro_gif.py').read_text()
boot=bringup.split('void fm1_cstart(void)',1)[1]

assert 'if(!ru_welcome_done)' in boot  # retain first-run wizard
assert 'welcome_preview.page=RU_PAGE_WELCOME;' in boot
assert 'welcome_preview.welcome_phase=RU_WELCOME_RETURN;' in boot
assert 'while((uint32_t)(etch_ms-welcome_since_ms)<1100u)' in boot
assert boot.index('ru_boot_intro_begin();')<boot.index('native_compile_edit();')
assert boot.index('native_start();')<boot.index('ru_splash_done=1u;')
assert boot.index('ru_splash_done=1u;')<boot.index('welcome_preview.page=RU_PAGE_WELCOME;')
assert boot.index('welcome_preview.page=RU_PAGE_WELCOME;')<boot.index('screen();')
assert boot.index('screen();')<boot.index('native_max_ticks=0u;')
assert 'native_max_ticks=0u;native_max_wall_ticks=0u;native_max_nested_ticks=0u;' in boot
assert 'native_min_dma_slack=ETCH_NATIVE_FRAMES*544u;' in boot
assert 'native_late=0' not in boot and 'native_dma_blocked=0' not in boot
assert 'if(measure.render_ticks>native_max_ticks)native_max_ticks=measure.render_ticks;' in target
assert 'etch_diag_cpu_permille(native_max_ticks,budget)' in bringup
assert 'ru_cpu1_fx_publish' in (src/'audio_core_core2.c').read_text()
assert 'v.welcome_phase=RU_WELCOME_RETURN;' in host
assert 'WELCOME_MS = 1100' in gif and "(49,'welcome-back')" in gif
print('PASS V2.0.58: WELCOME BACK 1.1s after intro; PLAY is ready; DSP MAX measures live audio')
