// V2.2 Hardware Calibration Lab: offline, read-only comparison of two PCM WAVs.
// Reference is the physical FM-1, comparison is the emulated SAME firmware/patch.
// This does not alter emulation timing or assume a numerical fidelity threshold.
const MAX_SECONDS = 60;
const MAX_BYTES = 48 * 1024 * 1024;
const round = (n, digits=4) => Number(n.toFixed(digits));

export function readPcm16Wav(buffer) {
  const bytes = buffer instanceof Uint8Array ? buffer : new Uint8Array(buffer);
  if (bytes.byteLength < 44 || bytes.byteLength > MAX_BYTES) throw new Error('WAV must be between 44 bytes and 48 MB');
  const dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const str = (p, n=4) => String.fromCharCode(...bytes.subarray(p,p+n));
  if (str(0) !== 'RIFF' || str(8) !== 'WAVE') throw new Error('Only little-endian RIFF/WAVE is supported');
  const total = dv.getUint32(4, true) + 8;
  if (total > bytes.byteLength || total < 44) throw new Error('Truncated RIFF file');
  let fmt = null, dataStart = -1, dataBytes = 0;
  for (let p = 12; p + 8 <= total;) {
    const tag = str(p), len = dv.getUint32(p + 4, true);
    if (len > total - p - 8) throw new Error('Invalid WAV chunk size');
    if (tag === 'fmt ' && fmt === null) {
      if (len < 16) throw new Error('Invalid PCM format chunk');
      fmt = {codec: dv.getUint16(p+8, true), channels: dv.getUint16(p+10, true),
        sampleRate: dv.getUint32(p+12, true), byteRate: dv.getUint32(p+16, true),
        blockAlign: dv.getUint16(p+20, true), bits: dv.getUint16(p+22, true)};
    }
    if (tag === 'data' && dataStart < 0) { dataStart = p+8; dataBytes = len; }
    const next = p + 8 + len + (len & 1);
    if (next > total && !(next === total+1 && (len&1))) throw new Error('Invalid WAV padding');
    p = next;
  }
  if (!fmt || dataStart < 0) throw new Error('Missing PCM format or data chunk');
  if (fmt.codec !== 1 || fmt.bits !== 16 || ![1,2].includes(fmt.channels))
    throw new Error('Convert each recording to uncompressed 16-bit PCM mono/stereo WAV');
  if (fmt.sampleRate < 8000 || fmt.sampleRate > 96000) throw new Error('WAV sample rate must be 8–96 kHz');
  if (fmt.blockAlign !== fmt.channels * 2 || fmt.byteRate !== fmt.sampleRate * fmt.blockAlign || dataBytes % fmt.blockAlign)
    throw new Error('Invalid PCM frame layout');
  const frames = dataBytes / fmt.blockAlign;
  if (!frames || frames > fmt.sampleRate * MAX_SECONDS) throw new Error('Use a recording between 1 frame and 60 seconds');
  const mono = new Float32Array(frames);
  if (fmt.channels === 1) {
    for (let i=0;i<frames;i++) mono[i] = dv.getInt16(dataStart+2*i,true) / 32768;
  } else {
    for (let i=0;i<frames;i++) mono[i] = (dv.getInt16(dataStart+4*i,true)+dv.getInt16(dataStart+4*i+2,true)) / 65536;
  }
  return {sampleRate:fmt.sampleRate, channels:fmt.channels, frames, durationSeconds:frames/fmt.sampleRate, samples:mono};
}

function pearson(a,b,lag,step=1,limit=Infinity) {
  const xa = Math.max(0, -lag), yb = Math.max(0, lag);
  const length = Math.min(a.length-xa,b.length-yb,limit);
  if (length < step*32) return null;
  let sx=0,sy=0,sxx=0,syy=0,sxy=0,n=0;
  for (let i=0;i<length;i+=step) {
    const x=a[xa+i],y=b[yb+i];
    sx+=x;sy+=y;sxx+=x*x;syy+=y*y;sxy+=x*y;n++;
  }
  const xx=sxx-sx*sx/n, yy=syy-sy*sy/n;
  return xx>1e-10 && yy>1e-10 ? (sxy-sx*sy/n)/Math.sqrt(xx*yy) : null;
}
function envelope(samples, window) {
  const count=Math.floor(samples.length/window), out=new Float32Array(count);
  for(let i=0;i<count;i++){
    let sum=0;
    for(let j=0;j<window;j++){const x=samples[i*window+j];sum+=x*x;}
    out[i]=Math.sqrt(sum/window);
  }
  return out;
}
function peakRms(samples) {
  let square=0, peak=0;
  for (const x of samples) {square+=x*x;peak=Math.max(peak,Math.abs(x));}
  return {rms:Math.sqrt(square/samples.length),peak};
}
function bestLag(a,b,center,window,step,limit,allowNegative=true) {
  let score=-Infinity, lag=center, r=0;
  for(let offset=-window;offset<=window;offset+=step){
    const k=center+offset, found=pearson(a,b,k,1,limit);
    if(found===null) continue;
    const match=allowNegative?Math.abs(found):found;
    if(match>score+1e-9 || (Math.abs(match-score)<=1e-9 && Math.abs(k)<Math.abs(lag))){score=match;lag=k;r=found;}
  }
  return {lag, score, r};
}

export function compareWavs(physical, emulator, opts={}) {
  if (!physical?.samples || !emulator?.samples) throw new Error('Two valid WAV files are required');
  const sr=physical.sampleRate;
  if (sr !== emulator.sampleRate) throw new Error('Sample rates differ. Export both WAV files at 44,100 Hz before comparing.');
  const startSeconds = Number(opts.skipSeconds ?? 0.5);
  const durationSeconds = Number(opts.durationSeconds ?? 3);
  const maxLagMs = Number(opts.maxLagMs ?? 250);
  if (!Number.isFinite(startSeconds) || startSeconds<0 || startSeconds>20 ||
      !Number.isFinite(durationSeconds) || durationSeconds<0.5 || durationSeconds>6 ||
      !Number.isFinite(maxLagMs) || maxLagMs<10 || maxLagMs>1000) throw new Error('Invalid comparison settings');
  const start=Math.round(sr*startSeconds);
  const end=Math.round(sr*(startSeconds+durationSeconds+maxLagMs/1000));
  const a=physical.samples.subarray(start,Math.min(physical.samples.length,end));
  const b=emulator.samples.subarray(start,Math.min(emulator.samples.length,end));
  const target=Math.min(Math.round(durationSeconds*sr),a.length,b.length);
  if (target < sr/2) throw new Error('Recordings are too short. Capture at least 2 seconds of the same notes.');
  const maxLag=Math.floor(maxLagMs*sr/1000);
  const ar=peakRms(a.subarray(0,target)), br=peakRms(b.subarray(0,target));
  if (ar.rms < 0.0005 || br.rms < 0.0005) throw new Error('One recording is nearly silent. Record the same audible MIDI passage in both.');
  // Stage 1: compare five-millisecond RMS envelopes to locate note onsets,
  // avoiding the period ambiguity of a sustained oscillator tone.
  const win=Math.max(1,Math.round(sr*0.005));
  const ea=envelope(a,win),eb=envelope(b,win),maxEnvLag=Math.ceil(maxLag/win);
  const envWindow=Math.min(ea.length, Math.floor(target/win));
  let bestEnv={score:-Infinity,lag:0};
  for(let lag=-maxEnvLag;lag<=maxEnvLag;lag++) {
    const r=pearson(ea,eb,lag,1,envWindow);
    if (r!==null && r>bestEnv.score){bestEnv={score:r,lag};}
  }
  // Stage 2: waveform correlation (plus a global fallback for constant envelopes).
  const coarseStep=Math.max(1,Math.round(sr/1600));
  let center=bestEnv.score>=0.25 ? bestEnv.lag*win : 0;
  let span=bestEnv.score>=0.25 ? Math.min(maxLag,Math.round(sr*0.012)) : maxLag;
  let coarse={score:-Infinity,lag:0};
  for (let lag=Math.max(-maxLag,center-span);lag<=Math.min(maxLag,center+span);lag+=(bestEnv.score>=0.25?1:coarseStep)) {
    // Raw correlation decimated inside the Pearson routine to cap main-thread work.
    const r=pearson(a,b,lag,coarseStep,target);
    if(r!==null && Math.abs(r)>coarse.score){coarse={score:Math.abs(r),lag};}
  }
  let lag=coarse.lag, method='waveform correlation';
  if (coarse.score>=0.10) {
    const refine=bestLag(a,b,coarse.lag,coarseStep,1,Math.min(target,sr),true);
    lag=refine.lag;
  } else if(bestEnv.score>=0.25){
    lag=bestEnv.lag*win;
    method='RMS-envelope correlation (waveforms differ)';
  } else throw new Error('No usable alignment found. Check firmware, patch, MIDI passage and recording start.');
  // Limit the comparison interval so both signals cover the same time span.
  const aOffset=Math.max(0,-lag), bOffset=Math.max(0,lag);
  const count=Math.min(target,a.length-aOffset,b.length-bOffset);
  if(count<sr/2) throw new Error('Not enough common audio remains after alignment');
  let sx=0,sy=0,sxx=0,syy=0,sxy=0,peakA=0,peakB=0;
  for(let i=0;i<count;i++) {
    const x=a[aOffset+i], y=b[bOffset+i];
    sx+=x;sy+=y;sxx+=x*x;syy+=y*y;sxy+=x*y;
    peakA=Math.max(peakA,Math.abs(x));peakB=Math.max(peakB,Math.abs(y));
  }
  const xx=sxx-sx*sx/count, yy=syy-sy*sy/count, xy=sxy-sx*sy/count;
  if(xx<1e-8 || yy<1e-8) throw new Error('Audio does not contain enough energy for a valid comparison');
  const gain=xy/yy, corr=xy/Math.sqrt(xx*yy);
  const residual=Math.sqrt(Math.max(0,(xx-2*gain*xy+gain*gain*yy)/xx));
  const matchConfidence=bestEnv.score >=0.25 ? 'envelope-guided' : 'waveform-only';
  return {
    schema:'fm1-calibration-v1', emulatorVersion:'2.3', generated:new Date().toISOString(),
    method, alignmentBasis:matchConfidence,
    sampleRate:sr, analyzedSeconds:round(count/sr,3), skippedSeconds:startSeconds,
    maxLagMs, alignmentLagMs:round(1000*lag/sr,3),
    // Positive means the emulator capture's signal begins later than the physical recording.
    correlation:round(corr,6), phaseInverted:gain<0,
    emulatorRmsVsPhysicalDb:round(20*Math.log10(Math.sqrt(yy/xx)),3),
    gainToMatchPhysical:round(gain,6), normalizedResidualRms:round(residual,6),
    physicalPeak:round(peakA,6), emulatorPeak:round(peakB,6),
    physicalChannels:physical.channels, emulatorChannels:emulator.channels,
    notes:[
      'This is a measurement of two supplied recordings, NOT proof of CPU, LCD or peripheral cycle accuracy.',
      'Only meaningful when recordings use the same firmware image, patch, MIDI sequence and comparable signal path.',
      'Lag includes recording-interface, MIDI, clock and capture-start offsets; do not automatically add it to emulator timing.',
      'A low correlation can result from analog noise, effects, different waveforms or mismatched playback conditions.',
      'Analysis runs locally in your browser. Raw audio is not sent or included in exported JSON.',
    ]
  };
}

export function calibrationText(r) {
  if(!r) return 'No audio comparison yet.';
  return [
    `Sample rate: ${r.sampleRate} Hz · analyzed ${r.analyzedSeconds}s`,
    `Alignment (emulator vs physical): ${r.alignmentLagMs} ms`,
    `Waveform correlation: ${r.correlation}`,
    `Polarity inverted: ${r.phaseInverted?'yes':'no'}`,
    `Emulator level difference: ${r.emulatorRmsVsPhysicalDb} dB`,
    `Gain to match physical: ${r.gainToMatchPhysical}`,
    `Residual after gain alignment: ${r.normalizedResidualRms}`,
    `Physical / emulator peak: ${r.physicalPeak} / ${r.emulatorPeak}`,
    `Alignment: ${r.alignmentBasis}`,
    '',
    'Do not treat this as proof of hardware accuracy or automatically change CPU speed.',
  ].join('\n');
}
