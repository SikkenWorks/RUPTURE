import { test } from 'node:test';
import assert from 'node:assert/strict';
import { createPanelScheduler } from './render-scheduler.js';
test('one frame per invalidation, including bursts of 30Hz device messages', () => {
  const callbacks = []; let paints = 0;
  const scheduler = createPanelScheduler({ requestFrame: f => callbacks.push(f), render: () => { paints++; return false; } });
  for (let i = 0; i < 50; i++) scheduler.invalidate();
  assert.equal(callbacks.length, 1);
  callbacks.shift()();
  assert.equal(paints, 1);
  assert.equal(scheduler.isPending(), false);
  scheduler.invalidate(); callbacks.shift()();
  assert.equal(paints, 2);
});
test('encoder animation ends without a permanent animation loop', () => {
  const callbacks = []; let paints = 0;
  const scheduler = createPanelScheduler({ requestFrame: f => callbacks.push(f), render: () => ++paints < 5 });
  scheduler.invalidate();
  while (callbacks.length) callbacks.shift()();
  assert.equal(paints, 5);
  assert.equal(callbacks.length, 0);
});
test('invalidation in a draw callback is not lost', () => {
  const callbacks = []; let paints = 0;
  let scheduler;
  scheduler = createPanelScheduler({ requestFrame: f => callbacks.push(f), render: () => {
    if (++paints === 1) scheduler.invalidate();
    return false;
  } });
  scheduler.invalidate();
  while (callbacks.length) callbacks.shift()();
  assert.equal(paints, 2);
});
