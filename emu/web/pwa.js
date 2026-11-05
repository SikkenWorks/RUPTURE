// Relative scope works in GitHub Pages subpaths and custom domains.
if ('serviceWorker' in navigator && (location.protocol === 'https:' || location.hostname === 'localhost')) {
  addEventListener('load', () => navigator.serviceWorker.register('./sw.js', {scope:'./'}).catch(error => {
    console.warn('Offline mode unavailable:', error);
  }));
}

const midiMenu = document.getElementById('midi-settings');
const midiPopup = midiMenu.querySelector('.popover');
midiMenu.addEventListener('toggle', () => {
  if (!midiMenu.open) return;
  const r = midiMenu.querySelector('summary').getBoundingClientRect();
  const width = Math.min(310, innerWidth - 20);
  midiPopup.style.position = 'fixed';
  midiPopup.style.top = `${Math.min(r.bottom + 7, innerHeight - 165)}px`;
  midiPopup.style.left = `${Math.max(10, Math.min(r.right - width, innerWidth - width - 10))}px`;
  midiPopup.style.right = 'auto';
});
document.addEventListener('pointerdown', event => {
  if (midiMenu.open && !midiMenu.contains(event.target)) midiMenu.open = false;
});
