import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { isFirmwareLocked, verifyPinnedPackage, assertAllowedSave, assertFirmwareImportAllowed } from './firmware-policy.js';
const catalog = JSON.parse(readFileSync(new URL('./firmwares.json',import.meta.url)));
const document = readFileSync(new URL('./index.html',import.meta.url), 'utf8');
const fixed = isFirmwareLocked({ dataset: { edition: 'rupture' } });
const digest = bytes => Promise.resolve(createHash('sha256').update(bytes).digest('hex'));
test('edition flag is locked only in RUPTURE edition', () => {
  assert.equal(fixed,true);
  assert.equal(isFirmwareLocked({dataset:{edition:'general'}}),false);
});
test('RUPTURE blocks firmware import, arbitrary saves and switching', () => {
  assert.throws(() => assertFirmwareImportAllowed(true),/disabled/);
  assert.doesNotThrow(() => assertFirmwareImportAllowed(false));
  assert.throws(() => assertAllowedSave(true,'a'.repeat(64),{hash:'b'.repeat(64)}),/RUPTURE/);
  assert.doesNotThrow(() => assertAllowedSave(true,'a'.repeat(64),{hash:'a'.repeat(64)}));
});
test('actual bundled FWSC file matches its pinned SHA-256', async () => {
  const entry = catalog.firmwares[0];
  assert.equal(catalog.default, 'rupture');
  assert.equal(catalog.firmwares.length,1);
  assert.equal(entry.package, './RUPTURE.fwsc');
  const actual = new Uint8Array(readFileSync(new URL('./RUPTURE.fwsc',import.meta.url)));
  assert.equal(await verifyPinnedPackage(entry,actual,digest),entry.sha256);
  const tampered = actual.slice(); tampered[123] ^= 1;
  await assert.rejects(()=>verifyPinnedPackage(entry,tampered,digest),/SHA-256 mismatch/);
});
test('HTML does not expose firmware importing and retains credits', () => {
  assert.match(document,/data-edition="rupture"/);
  assert.doesNotMatch(document,/id="open"/);
  assert.doesNotMatch(document,/id="import-stage"/);
  assert.doesNotMatch(document,/Import \.fwsc/);
  assert.match(document,/github.com\/joakimeriksson\/fm1-emu/);
  assert.match(document,/Start RUPTURE/);
});
