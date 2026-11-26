#!/usr/bin/env python3
"""Package V2.0.61 from audited native builds, preserving trusted boot/OTA."""
from pathlib import Path
import base64, hashlib, json, re, shutil, sys
ROOT=Path(__file__).resolve().parents[1]
V='2.0.61'
N=ROOT/'vendor/etch-v1.88-source'
sys.path.insert(0,str(N/'tools'))
import fm1_fwsc as fw
sha=lambda data:hashlib.sha256(data).hexdigest()
trusted=ROOT/'reference/firmware/RUPTURE-V1.86.1.fwsc'
assert fw.verify_fwsc(trusted)[1]==[]
assets=fw.extract_stock_assets(trusted)
trusted_entries={e['name']:e['data'] for e in fw.parse_ufw(trusted.read_bytes())['entries']}
manifest=json.loads((ROOT/'RELEASE-MANIFEST.json').read_text())
packages=[]
for experimental,subdir,prefix in (
 (False,'fm1-native-storage-updater-uac48-p05',''),
 (True,'fm1-native-storage-updater-uac48-p05-c1fx','cpu1-fx-'),
):
 directory=N/'build'/subdir
 app=(directory/'app.bin').read_bytes()
 opts=json.loads((directory/'native-build-options.json').read_text())
 assert opts['version']==f'RUPTURE V{V}' and opts['app_sha256']==sha(app)
 assert opts['cpu1_full_fx_live_lab_enabled']==experimental
 assert opts['cpu1_audio_offload_enabled']==experimental
 assert opts['storage_writes'] and opts['ota_writes'] and not opts['hardware_verified']
 assert len(app)<=581564 and not fw.validate_app_binary(app)
 assert f'V{V}'.encode() in app
 flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
 assert len(flash)==0x93000
 assert flash[:0x4000]==trusted_entries['flash.bin'][:0x4000]
 pkg=fw.build_fwsc(flash,assets['ota'],'FM-1_968')
 filename=f'RUPTURE-V{V}-'+('CORE2-EXPERIMENTAL' if experimental else 'STABLE')+'.fwsc'
 (ROOT/filename).write_bytes(pkg)
 assert fw.verify_fwsc(ROOT/filename)[1]==[]
 entries={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
 assert entries['ota.bin']==trusted_entries['ota.bin']
 assert entries['flash.bin']==flash
 for source,dest in (
  ('app.bin',prefix+'app.bin'),('app.bin.audit.json',prefix+'app.bin.audit.json'),
  ('native-build-options.json',prefix+'build-options.json' if experimental else 'native-build-options.json'),
 ):
  shutil.copy2(directory/source,ROOT/'native-build'/dest)
 key='experimental_' if experimental else ''
 manifest.update({key+'firmware_file':filename,key+'firmware_sha256':sha(pkg),key+'firmware_bytes':len(pkg),key+'app_sha256':sha(app),key+'app_bytes':len(app)})
 packages.append(pkg)
assert packages[0]!=packages[1]
manifest.update(version=V,release_feature='Silent-DMA storage handoff; power-off analogue pop remains unqualified',
 release_note='Fade final PCM down, require both distinct DMA halves zero, then keep I2S and analogue enable unchanged through storage. Guard unwrapped catalog reads and nested pauses. Failed handoff blocks flash and requires explicit audio rearm. No physical power-off suppression is claimed.',
 physical_fm1_verified=False,hardware_verified=False,release_ready=False,
 release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',
 headroom_qualification='Native link/app allocation audits pass. Real DSP overhead and analogue jack behavior are unmeasured.',
)
manifest['storage_audio_v260']={
 'fade_out_frames':256,'drain_requires_distinct_dma_halves':2,
 'deadline_checked_before_and_after_publish':True,
 'normal_storage_keeps_i2s_clock':True,'normal_storage_toggles_pc6':False,
 'warm_resume_reinitializes_codec':False,'warm_resume_fade_in_frames':8192,
 'raw_store_callbacks_guarded':5,'small_read_batch_resumes_in_foreground':True,
 'timeout_ms':50,'timeout_latches_audio_fault':True,
 'timeout_blocks_flash_callbacks':True,'explicit_home_play_rearm':True,
 'host_warm_transactions':10000,'host_fade_amplitudes':65536,
 'additional_regression_programs_passed':18,
 'physical_jack_noise_measured':False,'poweroff_early_warning_proven':False,
 'poweroff_pop_fixed':False,'physical_fm1_verified':False,
 'synthesis_fx_store_hal_source_hashes_file':'tests/v260_preserved_sources.json',
 'audit':'docs/AUDIO-OUTPUT-AUDIT-V2.0.61.md',
}
manifest['release_feature']='Panel LEDs settle before codec startup'
manifest['release_note']='V2.0.61 finishes the startup LED wave before codec initialization and reduces the function button animation peak. Physical verification pending.'
manifest['led_startup_v261']={'panel_quiet_ms_before_codec':400,'boot_button_peak':'mid','physical_verified':False}
(ROOT/'RELEASE-MANIFEST.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
for filename in ('WebInstaller.html','index.html'):
 path=ROOT/filename;old=path.read_text();new=old.replace('V2.0.60',f'V{V}')
 # Require already embedded trusted boot/OTA before replacing either payload.
 for tag,pkg,prefix in (('firmware',packages[0],''),('firmware-experimental',packages[1],'EXP_')):
  pattern=r'(<script id="'+tag+r'"[^>]*>)(.*?)(</script>)'
  match=re.search(pattern,old,re.S);assert match
  previous=base64.b64decode(match.group(2))
  previous_entries={e['name']:e['data'] for e in fw.parse_ufw(previous)['entries']}
  assert previous_entries['ota.bin']==assets['ota']
  assert previous_entries['flash.bin'][:0x4000]==trusted_entries['flash.bin'][:0x4000]
  new,count=re.subn(pattern,lambda m:m.group(1)+base64.b64encode(pkg).decode()+m.group(3),new,count=1,flags=re.S);assert count==1
  new,count=re.subn(r"const "+prefix+r"PKG_HASH='[0-9a-f]{64}'",f"const {prefix}PKG_HASH='{sha(pkg)}'",new);assert count==1
  new,count=re.subn(r'const '+prefix+r'PKG_LEN=\d+',f'const {prefix}PKG_LEN={len(pkg)}',new);assert count==1
 if filename=='index.html':
  for tag,source in (('fallback-readme','README.md'),('fallback-changelog','CHANGELOG.md')):
   pattern=r'(<script id="'+tag+r'"[^>]*>)(.*?)(</script>)'
   new,count=re.subn(pattern,lambda m:m.group(1)+base64.b64encode((ROOT/source).read_bytes()).decode()+m.group(3),new,count=1,flags=re.S);assert count==1
  pattern=r'(<script id="site-release-meta"[^>]*>)(.*?)(</script>)'
  def meta(m):
   data=json.loads(m.group(2));data.update(version=V,sha256=sha(packages[0]),bytes=len(packages[0]),experimental_sha256=sha(packages[1]),experimental_bytes=len(packages[1]))
   return m.group(1)+json.dumps(data)+m.group(3)
  new,count=re.subn(pattern,meta,new,count=1,flags=re.S);assert count==1
 # Prove all executable installer JavaScript remains byte-identical except
 # for the package length/hash constants. Inline base64/doc/metadata scripts
 # are compared independently by test_v260_release.py.
 def executable(html):
  scripts=[]
  for attrs,body in re.findall(r'<script\b([^>]*)>(.*?)</script>',html,re.S):
   if re.search(r'type=["\'](?:application/octet-stream|application/json|text/plain)',attrs):continue
   body=re.sub(r"const (EXP_)?PKG_HASH='[0-9a-f]{64}'",r"const \1PKG_HASH='HASH'",body)
   body=re.sub(r'const (EXP_)?PKG_LEN=\d+',r'const \1PKG_LEN=0',body)
   scripts.append((attrs,body.replace('V2.0.60','VERSION').replace('V'+V,'VERSION')))
  return scripts
 assert executable(old)==executable(new),filename+' executable installer logic changed'
 path.write_text(new)
print('PASS V'+V+': both native apps/FWSCs, protected boot/OTA and unchanged executable installer logic')
