#!/usr/bin/env bash
# Build the bundled, locked RUPTURE FM-1 emulator PWA, including its self-hosted WebAssembly core.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$ROOT/web/dist}"
cd "$ROOT"
export CARGO_PROFILE_RELEASE_CODEGEN_UNITS=1
export RUSTFLAGS="${RUSTFLAGS:-} -Cllvm-args=-inline-threshold=2000 -Clink-arg=--export-table -Clink-arg=--growable-table"
cargo build --release -p fm1-web --target wasm32-unknown-unknown --target-dir target/wasm
rm -rf "$OUT"
mkdir -p "$OUT/icons"
cp target/wasm/wasm32-unknown-unknown/release/fm1_web.wasm "$OUT/"
cp web/index.html web/style.css web/main.js web/persistence.js web/firmware-policy.js web/tryout.js web/worklet.js web/jit.js web/fwsc.js web/firmwares.json web/shortcuts.js web/render-scheduler.js web/accuracy.js web/calibration.js web/calibration-ui.js web/protect.js web/pwa.js web/sw.js web/manifest.webmanifest web/favicon.png "$OUT/"
cp web/RUPTURE.fwsc "$OUT/RUPTURE.fwsc"
cp web/icons/*.png "$OUT/icons/"
cp LICENSE "$OUT/LICENSE"
cp WEB-README.md "$OUT/README.md"
touch "$OUT/.nojekyll"
echo "Ready to deploy: $OUT (including fm1_web.wasm)"
