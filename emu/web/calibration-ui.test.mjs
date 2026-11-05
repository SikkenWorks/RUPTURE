import {test} from 'node:test';
import assert from 'node:assert/strict';
import vm from 'node:vm';
import {readFileSync} from 'node:fs';
import {readPcm16Wav,compareWavs,calibrationText} from './calibration.js';
function ui(){
  const els=new Map();
  class E{
    constructor(id){this.id=id;this.files=[];this.disabled=true;this.textContent='';this.value='';this.events={};this.calls=0;}
    addEventListener(event,cb){this.events[event]=cb;}
    click(){this.calls++;} showModal(){this.open=true;}
  }
  const get=id=>{if(!els.has(id))els.set(id,new E(id));return els.get(id);};
  get('calibration-skip').value='0.5';get('calibration-lag').value='250';
  const downloads=[];
  const document={getElementById:get,createElement(){return {click(){downloads.push('clicked');},set href(v){downloads.push(v);},set download(v){downloads.push(v);}};}};
  const src=readFileSync(new URL('./calibration-ui.js',import.meta.url),'utf8').replace(/^import .*;\s*/m,'');
  vm.runInNewContext(src,{readPcm16Wav,compareWavs,calibrationText,document,Blob,
    URL:{createObjectURL:()=> 'blob:local',revokeObjectURL:()=>{}},
    requestAnimationFrame:fn=>fn(),setTimeout:()=>{} });
  return {get,downloads};
}
function pcm16(samples=8000){
  const b=new ArrayBuffer(44+samples*2),v=new DataView(b);
  const put=(offset,s)=>{for(let i=0;i<s.length;i++)v.setUint8(offset+i,s.charCodeAt(i));};
  put(0,'RIFF');v.setUint32(4,36+samples*2,true);put(8,'WAVE');put(12,'fmt ');
  v.setUint32(16,16,true);v.setUint16(20,1,true);v.setUint16(22,1,true);
  v.setUint32(24,8000,true);v.setUint32(28,16000,true);v.setUint16(32,2,true);
  v.setUint16(34,16,true);put(36,'data');v.setUint32(40,samples*2,true);
  for(let i=0;i<samples;i++)v.setInt16(44+i*2,Math.round(8000*Math.sin(2*Math.PI*315*i/8000)),true);
  return b;
}
test('calibration input pairs are required and selecting new recordings clears prior output',async()=>{
  const x=ui();
  x.get('diagnostics-open').events.click();
  assert.equal(x.get('diagnostics-dialog').open,true);
  assert.equal(x.get('calibration-run').disabled,true);
  const bytes=pcm16(24000),file={size:bytes.byteLength,arrayBuffer:async()=>bytes};
  x.get('physical-wav').files=[file];x.get('physical-wav').events.change();
  assert.equal(x.get('calibration-run').disabled,true);
  x.get('emulator-wav').files=[file];x.get('emulator-wav').events.change();
  assert.equal(x.get('calibration-run').disabled,false);
  await x.get('calibration-run').events.click();
  assert.match(x.get('calibration-status').textContent,/Comparison complete/);
  assert.match(x.get('calibration-result').textContent,/Waveform correlation/);
  assert.equal(x.get('calibration-export').disabled,false);
  x.get('emulator-wav').events.change();
  assert.equal(x.get('calibration-export').disabled,true);
});
test('invalid WAV shows error without exporting audio',async()=>{
  const x=ui(),bad={size:400,arrayBuffer:async()=>new Uint8Array(400).buffer};
  x.get('physical-wav').files=[bad];x.get('emulator-wav').files=[bad];
  await x.get('calibration-run').events.click();
  assert.match(x.get('calibration-status').textContent,/Unable to compare/);
  assert.equal(x.get('calibration-export').disabled,true);
  assert.deepEqual(x.downloads,[]);
});
