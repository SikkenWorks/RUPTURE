/* Node-only, no real MIDI: exercise both firmware selection paths safely. */
'use strict';
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const {webcrypto}=require('node:crypto');
const data=JSON.parse(fs.readFileSync(0,'utf8'));
const elements=new Map();
const classes=()=>({add(){},remove(){},toggle(){}});
function el(id){
 if(!elements.has(id))elements.set(id,{
  value:'',textContent:'',hidden:false,disabled:false,checked:false,
  scrollHeight:0,scrollTop:0,classList:classes(),
  parentElement:{classList:classes()},setAttribute(){},
 });
 return elements.get(id);
}
el('firmware').textContent=data.stable;
el('firmware-experimental').textContent=data.core2;
el('firmware-choice').value='stable';
let confirmations=0,downloads=0;
const ctx=vm.createContext({
 window:{ETCHOta:{
  fromBase64:s=>Uint8Array.from(Buffer.from(s,'base64')),
  productOf:()=> 'FM-1_968',logicalImage:raw=>raw,
 },addEventListener(){}},
 document:{getElementById:el},
 crypto:webcrypto,Uint8Array,Blob,URL:{
  createObjectURL:()=> 'blob:mock'+(++downloads),revokeObjectURL(){},
 },
 Date,console,confirm:()=>{confirmations++;return true;},
 setTimeout,clearTimeout,
});

(async()=>{
 vm.runInContext(data.controller,ctx,{timeout:1000});
 await vm.runInContext('load()',ctx);
 assert.equal(vm.runInContext('pkg.edition',ctx),'stable');
 assert.equal(el('firmware-warning').hidden,true);
 assert.equal(el('experimental-ack-line').hidden,true);
 assert.equal(el('firmware-download').download,'RUPTURE-V2.0.38-STABLE.fwsc');

 el('firmware-choice').value='core2';
 await vm.runInContext('load()',ctx);
 assert.equal(vm.runInContext('pkg.edition',ctx),'core2');
 assert.equal(el('firmware-warning').hidden,false);
 assert.equal(el('experimental-ack-line').hidden,false);
 assert.equal(el('firmware-download').download,'RUPTURE-V2.0.38-CORE2-EXPERIMENTAL.fwsc');

 const installed=[];
 ctx.fakeEngine={
  install:async(_connected,pkg)=>{
   installed.push(pkg.edition);
   return {transferred:true,verified:true,identity:'FM-1_968'};
  },
  cleanup:async()=>{},transferAcknowledged:false,
 };
 vm.runInContext("engine=fakeEngine;connected={id:{text:'FM-1_968'}};",ctx);
 await el('install').onclick();
 assert.deepEqual(installed,[],'backup checkbox must block installation');
 el('confirm').checked=true;
 await el('install').onclick();
 assert.deepEqual(installed,[],'core2 recovery checkbox must block installation');
 assert.equal(confirmations,0);
 el('experimental-ack').checked=true;
 await el('install').onclick();
 assert.deepEqual(installed,['core2']);
 assert.equal(confirmations,2,'core2 must show an extra risk prompt');

 el('firmware-choice').value='stable';
 await vm.runInContext('load()',ctx);
 assert.equal(vm.runInContext('pkg.edition',ctx),'stable');
 assert.equal(el('experimental-ack').checked,false);
 assert.equal(el('confirm').checked,false,'switching firmware needs fresh backup consent');
 assert.equal(el('firmware-warning').hidden,true);
 vm.runInContext("connected={id:{text:'FM-1_968'}};",ctx);
 el('confirm').checked=true;
 await el('install').onclick();
 assert.deepEqual(installed,['core2','stable']);
 assert.equal(confirmations,3,'stable needs only the normal confirmation');
 process.stdout.write('PASS WebInstaller JS UI mock: stable/core2 SHA, extra consent, both routes\n');
})().catch(e=>{console.error(e);process.exitCode=1;});
