#!/usr/bin/env python3
"""Guard: the production CPU0 app remains identical to V2.0.32 except version."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
app=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin'
if not app.is_file():
 print('SKIP binary parity: native target not compiled in this source checkout')
else:
 data=app.read_bytes()
 assert data.count(b'V2.0.36')==1
 normalized=data.replace(b'V2.0.36',b'V2.0.32')
 previous='262af0b23d4a148942538293a1747cba4c79548314067669254d2bcb98e1d751'
 assert hashlib.sha256(normalized).hexdigest()==previous
 assert len(data)==548808
 print('PASS V2.0.36 production app byte-identical to V2.0.32 except version label')
