/* SPDX-License-Identifier: GPL-3.0-only
 * SikkenWorks RUPTURE - Cycle Exchange Synthesis (CXS)
 * Portable DSP research core. FM1 integration and fixed-point port pending.
 */
#ifndef RUPTURE_CXS_H
#define RUPTURE_CXS_H
#include <stdint.h>
#define CXS_SEGMENTS 8
#define CXS_MAX_HELD 48
#define CXS_SAMPLE_RATE 44100.0f
/* All parameters 0..100. */
typedef struct {float pressure,balance,memory,density;float attack,decay,sustain,release;float cutoff,resonance,drive;float lfo_rate,lfo_depth;} CXSParams;
typedef struct {float d[CXS_SEGMENTS];float flux[CXS_SEGMENTS];float edge[CXS_SEGMENTS];float levels[CXS_SEGMENTS];float phase,freq,env,low,band,prev,adc;uint32_t cycle,seed;int stage;int note,velocity;int gate;int held[CXS_MAX_HELD],held_count;CXSParams knob; } CXSEngine;
void cxs_init(CXSEngine *e);
void cxs_note_on(CXSEngine *e,int midi_note,int velocity,int retrigger);
void cxs_note_off(CXSEngine *e,int midi_note);
void cxs_panic(CXSEngine *e);
void cxs_set(CXSEngine *e, const CXSParams *p);
float cxs_tick(CXSEngine *e);
int cxs_verify(const CXSEngine *e);
#endif
