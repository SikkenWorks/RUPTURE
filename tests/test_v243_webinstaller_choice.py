#!/usr/bin/env python3
"""Verify both embedded FWSCs, strict opt-in and unchanged OTA transport."""
from pathlib import Path
import base64
import hashlib
import json
import re
import shutil
import subprocess

root=Path(__file__).resolve().parents[1]
stable=(root/'RUPTURE-V2.0.43-STABLE.fwsc').read_bytes()
core2=(root/'RUPTURE-V2.0.43-CORE2-EXPERIMENTAL.fwsc').read_bytes()
ota_reference='5cb6c4f5bd7d7337bbd35a63911f506b4b1d589549dc174b78b7a9c8c353fe08'

for name in ('WebInstaller.html','index.html'):
 html=(root/name).read_text()
 transport=re.search(r'<script>/\* SPDX-License-Identifier: GPL-3.0-only.*?</script>',html,re.S)
 assert transport and hashlib.sha256(transport.group().replace('V2.0.43','V2.0.35').encode()).hexdigest()==ota_reference
 assert html.count('id="firmware-choice"')==1
 assert html.count('<option value="stable" selected>')==1
 assert html.count('<option value="core2">')==1
 assert 'id="firmware-warning"' in html
 assert 'id="experimental-ack-line"' in html
 assert 'id="experimental-ack"' in html
 assert 'if(pkg.edition===\'core2\')' in html
 assert 'if(!$(' in html and 'experimental-ack' in html
 assert "$('firmware-choice').onchange=" in html
 assert "$('firmware-choice').disabled=true;" in html
 assert 'if(ticket!==loadTicket)return;' in html
 assert 'Firmware identity mismatch.' in html
 assert 'Selected '+"'"+'+choice.label+' in html
 for tag,data,hash_name,len_name in (
  ('firmware',stable,'PKG_HASH','PKG_LEN'),
  ('firmware-experimental',core2,'EXP_PKG_HASH','EXP_PKG_LEN'),
 ):
  m=re.search(r'<script id="'+tag+r'"[^>]*>(.*?)</script>',html,re.S)
  assert m and base64.b64decode(m.group(1))==data
  assert f"const {hash_name}='{hashlib.sha256(data).hexdigest()}'" in html
  assert f'const {len_name}={len(data)}' in html
 scripts=re.findall(r'<script>(.*?)</script>',html,re.S)
 controller=next((s for s in scripts if 'const FW_EDITIONS=' in s),None)
 assert controller
 if shutil.which('node'):
  jscheck=subprocess.run(['node','--check','-'],input=controller,text=True,capture_output=True)
  assert jscheck.returncode==0,(name,jscheck.stderr)
  harness=root/'tests/test_v243_webinstaller_choice_runtime.js'
  data={'controller':controller,'stable':base64.b64encode(stable).decode(),
        'core2':base64.b64encode(core2).decode()}
  runtime=subprocess.run(['node',str(harness)],input=json.dumps(data),text=True,capture_output=True)
  assert runtime.returncode==0,(name,runtime.stdout,runtime.stderr)
 else:
  print('SKIP live JS UI mock (node unavailable); embedded FWSC/hash/source checks passed')

print('PASS V2.0.43: two-option Web installers, default STABLE, core2 consent and original OTA transport')
