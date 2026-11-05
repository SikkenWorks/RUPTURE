// Firmware policy for the two V2.4 editions. Works with browser and Node tests.
// RUPTURE is pinned to the exact developer-supplied FWSC package.
export function isFirmwareLocked(root) {
  return root?.dataset?.edition === "rupture";
}
export async function verifyPinnedPackage(entry, bytes, digest) {
  if (!entry || typeof entry.sha256 !== "string" || !/^[a-f0-9]{64}$/i.test(entry.sha256)) {
    throw new Error("Bundled firmware checksum is missing");
  }
  if (!(bytes instanceof Uint8Array) || bytes.length < 960 || bytes.length > 8 * 1024 * 1024) {
    throw new Error("Bundled firmware has an invalid size");
  }
  const actual = await digest(bytes);
  if (actual !== entry.sha256.toLowerCase()) {
    throw new Error("Bundled RUPTURE.fwsc is different from this release (SHA-256 mismatch)");
  }
  return actual;
}
export function assertAllowedSave(locked, expectedHash, record) {
  if (locked && (!expectedHash || !record || record.hash !== expectedHash)) {
    throw new Error("This edition only accepts saves made for its bundled RUPTURE firmware");
  }
}
export function assertFirmwareImportAllowed(locked) {
  if (locked) throw new Error("Firmware import is disabled in the RUPTURE edition");
}
