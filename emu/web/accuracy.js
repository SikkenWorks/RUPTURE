// FM-1 Emulator V2.1: bounded, read-only timing/audio diagnostics.
// No recording of microphone, MIDI payloads, keys or user samples.
// Output deliberately distinguishes emulator performance from hardware fidelity.
const VALID_NUMBER = v => typeof v === 'number' && Number.isFinite(v);
const finite = (v, fallback=0) => VALID_NUMBER(v) ? v : fallback;
const rounded = (v, places=3) => Number(finite(v).toFixed(places));
export const DIAGNOSTIC_SECONDS = 10;
export const MAX_DIAGNOSTIC_INTERVALS = 800;
export const MAX_QUANTUM_SAMPLES = 8192;

export function percentile(samples, quantile) {
  if (!samples.length) return null;
  const sorted = [...samples].sort((a,b) => a-b);
  return rounded(sorted[Math.max(0, Math.ceil(quantile * sorted.length)-1)]);
}

export function createAccuracySession({ firmware, firmwareHash, mode, mhz, sampleRate=44100, browser="", device="fm1" }) {
  return {
    schema: 'fm1-accuracy-v1', emulatorVersion: '2.3',
    started: new Date().toISOString(),
    firmware: String(firmware || 'Unknown'), firmwareHash: String(firmwareHash || 'unknown'),
    device, mode, mhz, sampleRate, browser,
    samples: [], observedSeconds: 0, lastShort: null,
    // Individual callback durations are bounded to keep browser memory predictable.
    quantum: [], totalCallbackMs: 0, totalAudioMs: 0, underruns: 0,
    peak: 0, clipped: 0, sumSquares: 0, audioSamples: 0,
    lcdUpdates: 0, ledUpdates: 0, padUpdates: 0,
  };
}

export function observeAccuracyFrame(s, m) {
  if (!s || !m || m.type !== 'frame' || s.samples.length >= MAX_DIAGNOSTIC_INTERVALS) return false;
  const quanta = Math.max(0, finite(m.quanta));
  const frames = Math.max(0, finite(m.frames));
  const audioMs = quanta * frames / s.sampleRate * 1000;
  const busy = Math.max(0, finite(m.busy));
  // Exclude reports from before the emulator has begun its audio DMA playback.
  if (!m.playing || !audioMs) return false;
  const interval = {
    emulatedMs: Math.max(0, finite(m.emulated)),
    audioMs: rounded(audioMs), busyMs: rounded(busy),
    worstMs: rounded(Math.max(0, finite(m.worst))),
    lcd: !!m.lcd, leds: !!m.leds, pads: !!m.pads,
  };
  s.samples.push(interval);
  s.observedSeconds += audioMs / 1000;
  s.totalCallbackMs += busy;
  s.totalAudioMs += audioMs;
  if (m.lcd) s.lcdUpdates++;
  if (m.leds) s.ledUpdates++;
  if (m.pads) s.padUpdates++;
  if (VALID_NUMBER(m.short)) {
    // m.short is cumulative since boot; on a reboot the session is explicitly stopped.
    if (s.lastShort !== null) s.underruns += Math.max(0, m.short - s.lastShort);
    s.lastShort = m.short;
  }
  const d = m.diagnostics;
  if (d) {
    s.peak = Math.max(s.peak, Math.max(0, finite(d.peak)));
    s.clipped += Math.max(0, finite(d.clipped));
    s.sumSquares += Math.max(0, finite(d.sumSquares));
    s.audioSamples += Math.max(0, finite(d.audioSamples));
    for (const time of d.quantumMs || []) {
      if (VALID_NUMBER(time) && time >= 0 && s.quantum.length < MAX_QUANTUM_SAMPLES) s.quantum.push(time);
    }
  }
  return true;
}

export function summarizeAccuracy(s) {
  const audioQuantumMs = 128 / s.sampleRate * 1000;
  const load = s.totalAudioMs ? s.totalCallbackMs / s.totalAudioMs : null;
  const quantumOverDeadline = s.quantum.filter(q => q > audioQuantumMs).length;
  return {
    observedSeconds: rounded(s.observedSeconds, 2),
    fm1AudioRateHz: 44100, browserOutputRateHz: s.sampleRate,
    outputResampling: s.sampleRate !== 44100,
    audioWorkletLoadPercent: load === null ? null : rounded(load * 100, 1),
    quantumBudgetMs: rounded(audioQuantumMs),
    quantumP50Ms: percentile(s.quantum, 0.5),
    quantumP95Ms: percentile(s.quantum, 0.95),
    quantumP99Ms: percentile(s.quantum, 0.99),
    quantumMaximumMs: percentile(s.quantum, 1),
    sampledQuanta: s.quantum.length,
    quantaOverAudioDeadline: quantumOverDeadline,
    shortAudioQuanta: s.underruns,
    audioPeak: rounded(s.peak, 5),
    audioRms: s.audioSamples ? rounded(Math.sqrt(s.sumSquares/s.audioSamples), 5) : null,
    outputSamplesAtOrAboveFullScale: s.clipped,
    lcdUpdates: s.lcdUpdates, ledUpdates: s.ledUpdates, padUpdates: s.padUpdates,
    reportedFrames: s.samples.length,
  };
}

export function accuracyReport(s) {
  if (!s) return null;
  return {
    schema: s.schema, emulatorVersion: s.emulatorVersion, started: s.started,
    firmware: s.firmware, firmwareSha256: s.firmwareHash,
    device: s.device, mode: s.mode, mhz: s.mhz, sampleRate: s.sampleRate,
    browser: s.browser,
    summary: summarizeAccuracy(s),
    intervals: s.samples,
    notes: [
      'Emulator performance report; NOT a measurement of agreement with physical FM-1 hardware.',
      'CPU workload includes the browser AudioWorklet; it is not the FM-1 CPU usage.',
      'Short quanta count excludes underruns already present at measurement start.',
      'LCD/LED update counts are observations at browser polling frequency, not physical transfer measurements.',
      'The FM-1 audio clock remains 44100 Hz even when the browser chooses another output rate.',
      'No audio or MIDI content is exported.',
    ],
  };
}

export function diagnosticSummaryText(s) {
  const x = summarizeAccuracy(s);
  const fmt = v => v == null ? 'not measured' : String(v);
  return [
    `${x.observedSeconds}s observed · ${s.mode} · ${s.mhz} MHz`,
    `Audio clock: FM-1 ${x.fm1AudioRateHz} Hz → browser ${x.browserOutputRateHz} Hz${x.outputResampling ? ' (resampled)' : ' (native)'}`,
    `Audio-thread load: ${fmt(x.audioWorkletLoadPercent)}%`,
    `Audio callback p50 / p95 / p99: ${fmt(x.quantumP50Ms)} / ${fmt(x.quantumP95Ms)} / ${fmt(x.quantumP99Ms)} ms`,
    `Callback budget / max: ${x.quantumBudgetMs} / ${fmt(x.quantumMaximumMs)} ms`,
    `Callbacks over budget: ${x.quantaOverAudioDeadline}/${x.sampledQuanta}`,
    `Short audio quanta: ${x.shortAudioQuanta}`,
    `Audio peak / RMS: ${fmt(x.audioPeak)} / ${fmt(x.audioRms)} (linear)`,
    `Samples >= full scale: ${x.outputSamplesAtOrAboveFullScale}`,
    `LCD / LED / pad updates observed: ${x.lcdUpdates} / ${x.ledUpdates} / ${x.padUpdates}`,
    '', 'This report measures emulator performance, not physical FM-1 equivalence.',
  ].join('\n');
}
