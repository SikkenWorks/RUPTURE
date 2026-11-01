/* SPDX-License-Identifier: GPL-3.0-only
 * ETCH freestanding native carrier/morph/phase-modulation bring-up core.
 */
#pragma once
#include <stdint.h>
#include "wave_editor.h"
#include "native_fx.h"
#include "native_fx_bypass_v181.h"
#include "native_performance.h"
#include "native_voice_budget.h"
#include "native_motion.h"
#include "native_evolution.h"
#include "native_organic.h"
#include "native_living.h"
#include "native_mod_matrix.h"
#include "native_group_router.h"
#include "native_patch.h"
#include "native_wave_amp.h"
#define ETCH_NATIVE_VOICES 8
extern struct etch_native_bank native_banks[3]; /* shared preallocated mip banks */
#define ETCH_NATIVE_FRAMES 128
#ifdef RUPTURE_BUILD
#define ETCH_MONO_STACK 6u /* exactly six notes remembered, one audible voice */
#include "native_cxs_q15.h"
#include "rupture_subosc.h"
#else
#define ETCH_MONO_STACK 48u
#endif
#define ETCH_TRANSITION_SAMPLES 192u
struct etch_native_voice {uint32_t p,q,inc,eg;int32_t low,band;
#ifdef RUPTURE_BUILD
 int32_t ladder[4]; /* four internal Q15 stages, one per mono voice */
#endif
int32_t pre_lp[2];int32_t env,last,tail;uint16_t fade,attack_fade;uint16_t note;uint8_t stage;uint32_t gate_left,birth;uint8_t active,held,down,velocity;};
/* Recent-note priority stack; stores original identities, not quantized note numbers. */
struct etch_mono_held {uint16_t id;uint8_t pitch,velocity,down;uint32_t gate_left;};
struct etch_native_audio {
 struct etch_mono_held mono_stack[ETCH_MONO_STACK];uint8_t mono_count;
#ifdef RUPTURE_BUILD
 RuptureCXSQ15 cxs; /* main CXS carrier */
 RuSubState sub[2]; /* two independent monophonic subs, not extra note voices */
 uint32_t cxs_lfo_phase;int32_t cxs_body_low;int32_t saw_low;uint16_t main_saw_mix;
 uint32_t mono_current_inc,mono_target_inc,mono_glide_left;
 uint16_t mono_glide_ms;uint8_t mono_priority,mono_legato,mono_bend_range;
#endif
 struct etch_native_voice voice[ETCH_NATIVE_VOICES];
 uint32_t keys;int32_t morph,depth;uint32_t alloc_serial;uint8_t steal;uint16_t sustain;uint32_t bend[16];
 /* 8 normally; 6/4 only when ALNK reports real missed deadlines. */
 uint8_t voice_limit;
 uint8_t mono_mode; /* 1 = one active voice; voice_limit remains CPU headroom */
 uint8_t overload_tail[ETCH_NATIVE_VOICES];
 int32_t overload_last[ETCH_NATIVE_VOICES];
 struct etch_native_fx fx;
 int32_t master_gain_q16; /* continuous sample-rate master gain smoothing */
 int32_t poly_div_q10; /* adaptive, sample-slewed headroom for 5..8 voices */
 int32_t c_fx_smooth;uint32_t idle_samples;int32_t idle_gain;uint32_t perf_clock,perf_gate,random_seed;
 uint8_t seq_lock_mask;uint16_t seq_locks[ETCH_SEQ_LOCKS];
 uint32_t motion_sample;uint8_t motion_was_playing;
 uint32_t clock_samples,clock_sum;uint8_t clock_count,clock_running,clock_bpm;
 struct etch_harmony_settings harmony;
 uint8_t perf_step,perf_index,perf_last_seq,perf_last_arp,perf_held_count;
 uint8_t perf_played,perf_current_step; /* actual audio-rendered step; not next step */
 uint8_t perf_order[27];
 const struct etch_native_bank *bank,*old_bank;uint16_t bank_fade;
};
struct etch_native_controls {uint16_t morph,depth,gain;
#ifdef RUPTURE_BUILD
 uint8_t cxs_pressure,cxs_balance,cxs_memory,cxs_density,main_wave;
 uint8_t cxs_weight,cxs_edge,cxs_bite,cxs_punch;
 uint8_t sub_type[2],sub_octave[2],sub_detune[2],sub_drift[2],sub_blend[2];
 uint8_t note_priority,note_legato,note_bend_range;uint16_t note_glide_ms;
#endif
int8_t octave;const struct etch_native_bank *bank;
 uint32_t attack_step,decay_step,release_step,mod_inc;
 uint16_t sustain_level,cutoff,resonance,drive,chorus,delay,reverb;
 struct etch_fx_settings fx_settings;
 uint32_t fx_bypass_mask; /* runtime-only, non-destructive */
 uint8_t env_enabled,filter_type,mod_target,mod_mode,mod_ratio,mono_mode;
 const struct etch_arp_program *arp_program;
 uint32_t riff_seed;uint16_t riff_variant;uint8_t riff_mutation,riff_density;uint8_t preview_wave; /* 0=normal, 1=A, 2=B, 3=C */
 uint8_t arp_mode,arp_rate,arp_octaves,arp_gate,seq_run,seq_rate,seq_gate,tempo,seq_swing,clock_follow;
 const struct etch_motion_clip *motion;uint8_t motion_play;
 struct etch_evolution_settings evolution;
 struct etch_organic_settings organic;
 struct etch_living_settings living;
 struct etch_mod_matrix matrix;
 uint8_t velocity_curve; /* 0 linear, 1 soft, 2 hard. */
 uint8_t wave_amp_mode; /* 0=off, 1=A, 2=B, 3=morph; no additional oscillator */
 uint16_t wave_amp_depth; /* Q15 from packed per-preset setting */
 uint16_t seq_length;
 struct etch_harmony_settings harmony;
 const struct etch_native_pattern *pattern;
 uint8_t seq_notes[64];
};
void etch_native_groups_patch_bank(const struct etch_native_patch *bank);
void etch_native_reset(struct etch_native_audio *s);
void etch_native_render(struct etch_native_audio *s,const struct etch_native_controls *c,uint32_t keys,int16_t *out,uint32_t frames);
void etch_native_render_stereo(struct etch_native_audio *s,const struct etch_native_controls *c,uint32_t keys,int16_t *interleaved,uint32_t frames);

int16_t etch_native_preview(uint16_t morph,uint16_t depth,uint32_t phase);

int16_t etch_native_bank_preview(const struct etch_native_bank *bank,uint16_t morph,uint16_t depth,uint32_t phase);

/* MIDI identifiers are separate from panel keys; all 16 channels accepted. */
void etch_native_midi(struct etch_native_audio *s,uint8_t status,uint8_t d1,uint8_t d2);
void etch_native_midi_panic(struct etch_native_audio *s);
/* One shared eight-voice pool. Groups carry two-bit ownership and select
 * distinct saved A/B/C wavetable banks plus per-voice preset DSP settings. */
void etch_native_groups_configure(const struct etch_key_groups *groups);
uint8_t etch_native_groups_playing(void);
uint8_t etch_native_voice_group(uint8_t slot);

/* Read-only offline diagnostics for filter smoothing; no changes to patch/storage. */
void etch_native_filter_ramp_state(int32_t *cutoff,int32_t *resonance);
