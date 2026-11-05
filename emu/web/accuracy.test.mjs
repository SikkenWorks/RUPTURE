import { test } from 'node:test';
import assert from 'node:assert/strict';
import { createAccuracySession, observeAccuracyFrame, summarizeAccuracy, accuracyReport, diagnosticSummaryText, percentile, MAX_DIAGNOSTIC_INTERVALS } from './accuracy.js';
const make = () => createAccuracySession({ firmware: 'ETCH-test', firmwareHash:'abcdef', mode:'accurate', mhz:240, sampleRate:44100 });
test('quantiles are exact nearest-rank and avoid changing source arrays', () => {
  const values = [3,1,2,7];
  assert.equal(percentile(values, .95), 7);
  assert.equal(percentile(values, .5), 2);
  assert.equal(percentile([], .95), null);
  assert.deepEqual(values, [3,1,2,7]);
});
test('audio diagnostics summarize callback time, peak, clipping and LCD changes', () => {
  const s = make();
  assert.equal(observeAccuracyFrame(s,{type:'frame',playing:false,quanta:12,frames:128}),false);
  assert.equal(observeAccuracyFrame(s,{type:'frame',playing:true,quanta:12,frames:128,short:3, busy:10, worst:2, lcd:true,
    diagnostics:{peak:.95,clipped:0,sumSquares:12,audioSamples:48,quantumMs:[1,2,3]}}),true);
  assert.equal(observeAccuracyFrame(s,{type:'frame',playing:true,quanta:12,frames:128,short:5,busy:12,worst:5, lcd:null,leds:true,
    diagnostics:{peak:1.2,clipped:3,sumSquares:12,audioSamples:48,quantumMs:[2,3,4]}}),true);
  const summary = summarizeAccuracy(s);
  assert.equal(summary.shortAudioQuanta,2);
  assert.equal(summary.lcdUpdates,1);
  assert.equal(summary.ledUpdates,1);
  assert.equal(summary.audioPeak,1.2);
  assert.equal(summary.outputSamplesAtOrAboveFullScale,3);
  assert.equal(summary.audioRms,0.5);
  assert.equal(summary.quantumP99Ms,4);
  assert.equal(summary.quantaOverAudioDeadline,3); // two 3ms callbacks and one 4ms callback
  assert.equal(accuracyReport(s).firmwareSha256,'abcdef');
  assert.match(diagnosticSummaryText(s),/not physical FM-1 equivalence/);
  assert.equal(accuracyReport(s).intervals.length,2);
  assert.equal(accuracyReport(s).notes.some(n => n.includes('No audio or MIDI content')),true);
});
test('bounded samples, reject invalid elapsed data, do not count untouched LCD', () => {
  const s = make();
  assert.equal(observeAccuracyFrame(s,{type:'snapshot'}),false);
  assert.equal(observeAccuracyFrame(s,{type:'frame',playing:true,quanta:NaN,frames:128}),false);
  for(let i=0;i<MAX_DIAGNOSTIC_INTERVALS+3;i++) observeAccuracyFrame(s,{type:'frame',playing:true,quanta:12,frames:128,short:i, busy:0.5, lcd:null});
  assert.equal(s.samples.length,MAX_DIAGNOSTIC_INTERVALS);
  assert.equal(s.lcdUpdates,0);
  assert.ok(s.observedSeconds>20);
});


test('48000 Hz browser diagnostics disclose native 44100 Hz source with conversion', () => {
  const s = createAccuracySession({firmware:'RUPTURE',mode:'performance',mhz:240,sampleRate:48000});
  assert.equal(summarizeAccuracy(s).outputResampling,true);
  assert.equal(summarizeAccuracy(s).fm1AudioRateHz,44100);
  assert.equal(summarizeAccuracy(s).browserOutputRateHz,48000);
  assert.match(diagnosticSummaryText(s),/44100 Hz → browser 48000 Hz \(resampled\)/);
  assert.equal(accuracyReport(s).emulatorVersion,'2.3');
});
