// Optional V2.2 offline hardware A/B measurement UI; deliberately separate from
// emulation and flash-state lifecycle to avoid changes to Joakim's core.
import {readPcm16Wav,compareWavs,calibrationText} from './calibration.js';
const physical=document.getElementById('physical-wav');
const emulated=document.getElementById('emulator-wav');
const compareBtn=document.getElementById('calibration-run');
const exportBtn=document.getElementById('calibration-export');
const message=document.getElementById('calibration-status');
const results=document.getElementById('calibration-result');
let latest=null;
// Keep the read-only comparison usable when the optional WASM emulator engine
// is missing; the main emulator installs the same guarded dialog opener.
document.getElementById('diagnostics-open').addEventListener('click',()=>{
  const dialog=document.getElementById('diagnostics-dialog');
  if(!dialog.open)dialog.showModal();
});
const FILE_LIMIT=48*1024*1024;
function update(){compareBtn.disabled=!(physical.files?.[0] && emulated.files?.[0]);}
physical.addEventListener('change',()=>{latest=null;exportBtn.disabled=true;update();});
emulated.addEventListener('change',()=>{latest=null;exportBtn.disabled=true;update();});
compareBtn.addEventListener('click',async()=>{
  const files=[physical.files?.[0],emulated.files?.[0]];
  if(files.some(f=>!f))return;
  compareBtn.disabled=true;exportBtn.disabled=true;latest=null;
  message.textContent='Reading WAV files locally…';
  results.textContent='Comparing waveforms…';
  try{
    for(const file of files)if(file.size>FILE_LIMIT)throw new Error('Each WAV must be smaller than 48 MB');
    const [ref,sim]=await Promise.all(files.map(file=>file.arrayBuffer().then(readPcm16Wav)));
    // Give the UI a paint opportunity before CPU-bound correlation.
    await new Promise(resolve=>requestAnimationFrame(resolve));
    const skipSeconds=Number(document.getElementById('calibration-skip').value);
    const maxLagMs=Number(document.getElementById('calibration-lag').value);
    latest=compareWavs(ref,sim,{skipSeconds,maxLagMs});
    results.textContent=calibrationText(latest);
    message.textContent='Comparison complete. Only measurements, not recorded audio, are exported.';
    exportBtn.disabled=false;
  }catch(error){message.textContent=`Unable to compare: ${error.message}`;results.textContent='Check that both clips are 16-bit PCM WAV files at the same sample rate.';}
  finally{update();}
});
exportBtn.addEventListener('click',()=>{
  if(!latest)return;
  const blob=new Blob([JSON.stringify(latest,null,2)+'\n'],{type:'application/json'});
  const url=URL.createObjectURL(blob);
  const a=document.createElement('a');a.href=url;a.download='fm1-physical-comparison-v2.3.json';a.click();
  setTimeout(()=>URL.revokeObjectURL(url),1000);
});
