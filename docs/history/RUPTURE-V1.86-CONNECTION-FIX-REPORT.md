# RUPTURE V1.86 connection fix: test report

## Root cause (confirmed by source and simulated packets)

V1.85.2 added cached JEDEC bytes to the end of the 27-byte OTA identity payload. The website and WebInstaller parsed the name using `.filter(x => x!==0)` over all 27 bytes. This concatenated the three binary JEDEC bytes onto the correct name `FM-1_968` and caused the strict identity regex to reject the FM1, even when the reply arrived correctly. This bug affects both flash ID `0x856014` and `0xB36014` when nonzero.

## Changes

- Stop parsing at the **first null byte**. Read JEDEC telemetry separately as an optional numeric field.
- Automatic MIDI detection checks a bounded set of otherwise unrecognized input/output pairs via a read-only identity query.
- Displays browser-visible MIDI input/output names and a manual pairing option if detection fails.
- OTA CHECK, loader discovery, loader WRITE, flash metadata, speaker timing, and native firmware remain unchanged.

## Tests

- `node tests/test_v186_connection_fix.mjs`: both JEDEC IDs, old identity without appended bytes, loader identity, malformed packet rejection, generic Windows MIDI name pairing, no firmware-write commands during detection.
- `python3 tools/build_site.py`, `python3 tools/test_site.py`, `python3 tests/test_website_release.py`: installer/index embedded package consistency, firmware SHA and content.
- No physical FM1 or browser USB device is available here, so live hardware/driver/permission behavior is not verified.

## Usage

1. Replace **both** `index.html` and `WebInstaller.html` on GitHub Pages; also publish the existing matching `.fwsc`.
2. Load the new HTTPS website in Chrome/Edge with a hard refresh.
3. Connect a **powered-on** FM1 with a USB data cable, grant Web MIDI **SysEx**, click CONNECT FM1.
4. If ports appear but automatic identification does not, use the manual MIDI input/output pair picker and inspect the technical log.
5. Do not start firmware installation if the identity is not verified.

**Firmware SHA-256 (unchanged):** `239edee0056b32896f6f293cab22a05e366b94928ed76d2ebd4487047a45ccd8`
