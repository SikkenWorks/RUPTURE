import { test } from 'node:test';
import assert from 'node:assert/strict';
import { pcm16StereoWav } from './audio-file.js';
test('write standard PCM16 stereo WAV with exact RIFF/data sizes', () => {
  const pcm = Buffer.from([0,0,255,127,255,255,0,128]);
  const wav = pcm16StereoWav(pcm, 44100);
  assert.equal(wav.toString('ascii',0,4),'RIFF');
  assert.equal(wav.readUInt32LE(4),44);
  assert.equal(wav.toString('ascii',8,12),'WAVE');
  assert.equal(wav.readUInt16LE(22),2);
  assert.equal(wav.readUInt32LE(24),44100);
  assert.equal(wav.readUInt32LE(28),176400);
  assert.equal(wav.readUInt16LE(34),16);
  assert.equal(wav.readUInt32LE(40),8);
  assert.deepEqual(wav.subarray(44),pcm);
});
test('refuse malformed PCM buffers', () => {
  assert.throws(() => pcm16StereoWav(Buffer.from([1,2])),/stereo PCM16/);
  assert.throws(() => pcm16StereoWav(Buffer.alloc(8),0),/sample rate/);
});
