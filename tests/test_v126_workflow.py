"""Regression guard for physical SAVE/SEL semantics and release wording."""
from pathlib import Path
root=Path(__file__).resolve().parent.parent
n=root/'vendor/etch-v1.88-source/firmware/fm1-native'
b=(n/'bringup.c').read_text();u=(n/'rupture_ui.c').read_text()
assert 'const uint8_t entry_page=ru_page;' in b
assert 'if(entry_page==RU_PAGE_NAME && (pressed&(1u<<9))){if(ru_manager_name)ru_manager_commit_name();else ru_commit_name();}' in b
assert 'if(!ru_seq_slot_used(slot))ru_begin_name(1u,1u);' in b
assert 'if(!(native_project_present&(1u<<(ru_project_slot-1u))))ru_begin_name(0u,1u);' in b
assert 'ru_seq_slot_action(0u)' in b and 'ru_seq_slot_action(1u)' in b
assert 'ru_project_action(0u)' in b and 'ru_project_action(1u)' in b
assert 'ru_project_confirm!=2u' in b and 'ru_seq_confirm!=2u' in b
assert 'ru_project_confirm!=1u' in b and 'ru_seq_confirm!=1u' in b
assert 'ru_active_seq_slot=ru_seq_slot' in b
assert 'ru_active_seq_slot=0u' in b
assert 'ru_battery_filtered>=578?100' in b
assert 'SAVE TO STORE' in u and 'SEL NO ACTION' not in u
assert 'SEL AGAIN TO OPEN' in u and 'SAVE AGAIN TO REPLACE' in u
for filename in ('WebInstaller.html','index.html','tools/site_template.html'):
 text=(root/filename).read_text().lower()
 assert 'the firmware is not physically validated on the fm1' not in text
 assert 'physical rupture hardware operation is not yet validated' not in text
web=(root/'WebInstaller.html').read_text()
assert 'const transmitted=new Uint8Array(pkg.image.length);' in web
assert 'report(count,r.addr+r.n,r.addr,r.n);' in web
assert 'transmitted[i]=1;uniqueBytes++' in web
assert 'ota_progress_update(i+n,stage_len);' in (n/'ota.c').read_text()
assert 'state.k[0]=ota_matrix_percent' in (n/'ota_target.c').read_text()
print('PASS V1.26: slot selection, separate confirm actions, name-entry safety, battery and accurate browser progress')
