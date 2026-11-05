import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readPcm16Wav,compareWavs,calibrationText} from './calibration.js';

function wav(signal,sr=44100,channels=1){
  const data=signal.length*channels*2;
  const a=new Uint8Array(44+data),v=new DataView(a.buffer);
  const tag=(at,s)=>{for(let i=0;i<s.length;i++)a[at+i]=s.charCodeAt(i);};
  tag(0,'RIFF');v.setUint32(4,36+data,true);tag(8,'WAVE');tag(12,'fmt ');
  v.setUint32(16,16,true);v.setUint16(20,1,true);v.setUint16(22,channels,true);
  v.setUint32(24,sr,true);v.setUint32(28,sr*channels*2,true);
  v.setUint16(32,channels*2,true);v.setUint16(34,16,true);tag(36,'data');v.setUint32(40,data,true);
  for(let i=0;i<signal.length;i++)for(let j=0;j<channels;j++)v.setInt16(44+(i*channels+j)*2,Math.max(-32768,Math.min(32767,Math.round(signal[i]*32767))),true);
  return a;
}
function material(sr=8000,sec=5){
  const v=new Float32Array(sr*sec);
  // Make onset pattern nonperiodic so lag has a single useful solution.
  const gates=[[.7,1.1],[1.55,2.2],[2.47,2.9],[3.13,4.1]];
  for(let i=0;i<v.length;i++){
    const t=i/sr;
    if(gates.some(([s,e])=>t>=s&&t<e)){
      v[i]=.55*(Math.sin(2*Math.PI*233*t)+.28*Math.sin(2*Math.PI*457*t));
    }
  }
  return v;
}
test('PCM16 mono and stereo WAV decode with correct sample-rate metadata',()=>{
  const s=new Float32Array([0,.5,-.5,1]);
  for(const c of [1,2]){
    const r=readPcm16Wav(wav(s,44100,c));
    assert.equal(r.sampleRate,44100);assert.equal(r.channels,c);assert.equal(r.frames,4);
    assert.ok(Math.abs(r.samples[1]-.5)<.0001);
  }
});
test('reject malformed/truncated, unsupported codec, invalid layout, duration or rate',()=>{
  assert.throws(()=>readPcm16Wav(new Uint8Array(10)),/WAV/);
  const v=wav(new Float32Array(512));
  const truncated=v.subarray(0,70);
  assert.throws(()=>readPcm16Wav(truncated),/Truncated/);
  const float=v.slice();new DataView(float.buffer).setUint16(20,3,true);
  assert.throws(()=>readPcm16Wav(float),/16-bit PCM/);
  const bad=v.slice();new DataView(bad.buffer).setUint16(32,5,true);
  assert.throws(()=>readPcm16Wav(bad),/layout/);
});
test('delayed and quieter emulator recording aligns to physical reference',()=>{
  const sr=8000,ref=material(sr), delay=Math.round(sr*.042);
  const sim=new Float32Array(ref.length);
  for(let i=delay;i<sim.length;i++)sim[i]=ref[i-delay]*.6;
  const result=compareWavs(readPcm16Wav(wav(ref,sr)),readPcm16Wav(wav(sim,sr)),{skipSeconds:0,maxLagMs:250});
  assert.ok(Math.abs(result.alignmentLagMs-42)<1.5,`lag=${result.alignmentLagMs}`);
  assert.ok(result.correlation>.97,`correlation=${result.correlation}`);
  assert.ok(Math.abs(result.emulatorRmsVsPhysicalDb-20*Math.log10(.6))<.3);
  assert.ok(result.normalizedResidualRms<.1);
  assert.match(calibrationText(result),/proof of hardware accuracy/);
});
test('polarity-inverted waveform is identified; positive lag remains interpretable',()=>{
  const sr=8000,ref=material(sr),delay=Math.round(sr*.027),sim=new Float32Array(ref.length);
  for(let i=delay;i<sim.length;i++)sim[i]=-ref[i-delay]*.8;
  const result=compareWavs(readPcm16Wav(wav(ref,sr)),readPcm16Wav(wav(sim,sr)),{skipSeconds:.5,maxLagMs:250});
  assert.ok(Math.abs(result.alignmentLagMs-27)<1.5);
  assert.equal(result.phaseInverted,true);
  assert.ok(result.correlation<-.97);
});
test('refuses mostly silent input, incompatible sample rates and too-short files',()=>{
  const ref=readPcm16Wav(wav(material(8000),8000));
  assert.throws(()=>compareWavs(ref,readPcm16Wav(wav(new Float32Array(8000*5),8000))),/nearly silent/);
  assert.throws(()=>compareWavs(ref,readPcm16Wav(wav(material(11025),11025))),/Sample rates/);
  assert.throws(()=>compareWavs(ref,readPcm16Wav(wav(new Float32Array(30),8000))),/too short/);
  assert.throws(()=>compareWavs(ref,ref,{maxLagMs:9999}),/Invalid comparison/);
});
