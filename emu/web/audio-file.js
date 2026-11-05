// Minimal PCM16 stereo WAV wrapper for the deterministic CLI benchmark.
// The browser AudioWorklet does NOT export recordings.
export function pcm16StereoWav(pcm, sampleRate=44100) {
  if (!Buffer.isBuffer(pcm) || pcm.length % 4 !== 0) throw new Error('stereo PCM16 buffer required');
  if (!Number.isInteger(sampleRate) || sampleRate < 8000 || sampleRate > 192000) throw new Error('invalid sample rate');
  const header = Buffer.alloc(44);
  header.write('RIFF', 0); header.writeUInt32LE(36 + pcm.length, 4);
  header.write('WAVEfmt ', 8); header.writeUInt32LE(16, 16);
  header.writeUInt16LE(1, 20); header.writeUInt16LE(2, 22);
  header.writeUInt32LE(sampleRate, 24); header.writeUInt32LE(sampleRate * 4, 28);
  header.writeUInt16LE(4, 32); header.writeUInt16LE(16, 34);
  header.write('data', 36); header.writeUInt32LE(pcm.length, 40);
  return Buffer.concat([header,pcm]);
}
