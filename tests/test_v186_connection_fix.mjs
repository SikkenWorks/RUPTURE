/* Regression coverage for RUPTURE V1.86 Web MIDI identity and port discovery. */
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..');
const html=readFileSync(path.join(root,'WebInstaller.html'),'utf8');
const scripts=[...html.matchAll(/<script\b[^>]*>([\s\S]*?)<\/script>/g)];
const engineSource=scripts[1]?.[1];
assert.ok(engineSource,'Cannot locate OTA engine script');
const context={window:{},console,setTimeout,clearTimeout,Date,Uint8Array,Array,Math,Promise,Set};
vm.runInNewContext(engineSource,context);
const {wrap,parseIdentity,Engine,C}=context.window.ETCHOta;
function reply(name,jedec=0){
  const payload=new Uint8Array(27);
  for(let i=0;i<name.length;i++)payload[i]=name.charCodeAt(i);
  payload[24]=(jedec>>>16)&255;payload[25]=(jedec>>>8)&255;payload[26]=jedec&255;
  const b=new Uint8Array(34);b.set([0,0x59,0x11,27,0,0],0);b.set(payload,6);
  let sum=0;for(const ch of payload)sum=(sum+ch)&255;
  b[33]=(~sum)&255;
  return wrap(b);
}
for(const [chip,id] of [
  ['old',0],['Puya P25Q80',0x856014],['UC25HQ80',0xB36014]
]) {
  const dev=parseIdentity(reply('FM-1_968',id));
  assert.ok(dev,`FM1 must be identified for ${chip}`);
  assert.equal(dev.text,'FM-1_968');
  assert.equal(dev.loader,false);
  assert.equal(dev.jedec,id);
}
assert.equal(parseIdentity(reply('ota-FM-1_015')).text,'ota-FM-1_015');
assert.equal(parseIdentity(reply('garbage')),null);
const altered=reply('FM-1_968',0x856014);
altered[5]=0xF7;
assert.equal(parseIdentity(altered),null,'malformed packet must not pass');
const sent=[];
const inPort={id:'fake-in',name:'MIDIIN2 (generic Windows port)',state:'connected',listeners:new Set(),
  async open(){return this;},async close(){return this;},
  addEventListener(type,cb){assert.equal(type,'midimessage');this.listeners.add(cb);},
  removeEventListener(type,cb){this.listeners.delete(cb);}};
const outPort={id:'fake-out',name:'MIDIOUT2 (generic Windows port)',state:'connected',
  async open(){return this;},async close(){return this;},
  send(cmd){sent.push(Array.from(cmd));if(Array.from(cmd).join(',')===Array.from(C.QUERY).join(',')){
    setTimeout(()=>{for(const cb of inPort.listeners)cb({data:reply('FM-1_968',0xB36014)});},2);
  }}};
assert.equal(Engine.known(inPort.name),false,'Fixture must exercise unknown-name fallback');
const access={inputs:new Map([['1',inPort]]),outputs:new Map([['2',outPort]])};
const logs=[];const engine=new Engine(access,msg=>logs.push(msg),()=>{});
const found=await engine.find();
assert.ok(found,'Auto-discovery must validate a generic Windows MIDI pair');
assert.equal(found.id.text,'FM-1_968');
assert.equal(found.id.jedec,0xB36014);
assert.ok(logs.some(x=>x.includes('read-only identity queries')));
assert.ok(sent.length>0);
assert.ok(sent.every(cmd=>cmd.join(',')===Array.from(C.QUERY).join(',')), 'Auto-discovery must never send UPGRADE/WRITE');
await engine.cleanup();
console.log('PASS: NUL-terminated identity with both JEDEC IDs, OTA identity, invalid packet rejection, generic Windows MIDI port fallback (read-only)');
