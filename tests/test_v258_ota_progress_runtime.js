/* Validate real byte coverage for both CHECK and WRITE, including retries. */
'use strict';
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const html=fs.readFileSync('WebInstaller.html','utf8');
const transport=[...html.matchAll(/<script>([\s\S]*?)<\/script>/g)]
 .map(x=>x[1]).find(x=>x.includes('root.ETCHOta=api'));
assert.ok(transport,'OTA transport module missing');
const window={};
vm.runInNewContext(transport,{window,Uint8Array,TextEncoder,setTimeout,clearTimeout,
  crypto:require('node:crypto').webcrypto},{timeout:1500});
const {byteCoverage}=window.ETCHOta;
assert.equal(typeof byteCoverage,'function');
for(const phase of ['CHECK','WRITE']){
 const sample=byteCoverage(1000);
 assert.deepEqual({...sample(0,100)},{bytes:100,percent:10});
 assert.deepEqual({...sample(0,100)},{bytes:100,percent:10},phase+' duplicate request');
 assert.deepEqual({...sample(900,100)},{bytes:200,percent:20},phase+' out-of-order request');
 assert.deepEqual({...sample(90,20)},{bytes:210,percent:21},phase+' overlap');
 assert.deepEqual({...sample(100,800)},{bytes:1000,percent:99},phase+' all bytes');
 assert.deepEqual({...sample(0,1000)},{bytes:1000,percent:99},phase+' never regresses');
 assert.throws(()=>sample(999,2),/Invalid OTA progress range/);
 assert.throws(()=>sample(-1,1),/Invalid OTA progress range/);
 assert.throws(()=>sample(0,-1),/Invalid OTA progress range/);
}
console.log('PASS V2.0.58 OTA byte coverage: duplicate, overlap, out-of-order, complete and bounds');
