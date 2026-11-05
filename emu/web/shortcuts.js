// Keyboard mapping for the import-only FM-1 Emulator PWA.
// Original emulator & device mappings: Joakim Eriksson, https://github.com/joakimeriksson/fm1-emu
// This web-only rebinding UI does not modify or emulate new FM1 firmware features.
const KEY = 'fm1.pwa.shortcuts.v1';
const NOTE_LABELS = ['C4', 'C#4', 'D4', 'D#4', 'E4', 'F4', 'F#4', 'G4', 'G#4', 'A4', 'A#4', 'B4', 'C5', 'C#5', 'D5', 'D#5', 'E5', 'F5'];
const NOTE_CODES = ['KeyA', 'KeyW', 'KeyS', 'KeyE', 'KeyD', 'KeyF', 'KeyT', 'KeyG', 'KeyY', 'KeyH', 'KeyU', 'KeyJ', 'KeyK', 'KeyO', 'KeyL', 'KeyP', 'Semicolon', 'Quote'];
const BUTTONS = [
  [0, 'OCT −', 'KeyZ'], [1, 'OCT +', 'KeyX'], [2, 'FX', 'Digit1'],
  [3, 'SCL', 'Digit2'], [4, 'ENV', 'Digit3'], [5, 'LFO', 'Digit4'],
  [6, 'EDIT', 'Digit5'], [7, 'GLO', 'Digit6'], [8, 'HOME', 'Digit7'],
  [9, 'SAVE', 'Digit8'], [10, 'ARP', 'Digit9'], [11, 'SEQ', 'Digit0'],
  [12, 'PLAY/STOP', 'Space'], [13, 'REC', 'KeyR'],
];
const ENCODERS = [
  ['SELECT', 'ArrowLeft', 'ArrowRight'],
  ['PRESETS', 'ArrowDown', 'ArrowUp'],
  ['ALGORITHM', 'Comma', 'Period'],
  ['KNOB 1', null, null], ['KNOB 2', null, null],
  ['KNOB 3', null, null], ['KNOB 4', null, null],
];
export const CONTROLS = [
  ...NOTE_LABELS.map((label, i) => ({ id: `note-${i}`, name: label, group: 'Piano keys', defaultCode: NOTE_CODES[i] })),
  ...BUTTONS.map(([id, name, defaultCode]) => ({ id: `button-${id}`, name, group: 'FM-1 buttons', defaultCode })),
  ...ENCODERS.flatMap(([name, minus, plus], i) => [
    { id: `enc-${i}-minus`, name: `${name} −`, group: 'Rotary encoders', defaultCode: minus },
    { id: `enc-${i}-plus`, name: `${name} +`, group: 'Rotary encoders', defaultCode: plus },
  ]),
  { id: 'master-minus', name: 'MASTER −', group: 'Master volume', defaultCode: null },
  { id: 'master-plus', name: 'MASTER +', group: 'Master volume', defaultCode: null },
];
export function defaults() { return Object.fromEntries(CONTROLS.map(c => [c.id, c.defaultCode])); }
function safeRead() {
  try {
    const raw = JSON.parse(localStorage.getItem(KEY) || 'null');
    if (!raw || typeof raw !== 'object' || Array.isArray(raw)) return defaults();
    const mapped = defaults(), seen = new Set();
    for (const c of CONTROLS) {
      const code = Object.prototype.hasOwnProperty.call(raw, c.id) ? raw[c.id] : mapped[c.id];
      if (code === null || (typeof code === 'string' && /^[A-Za-z][A-Za-z0-9]{1,32}$/.test(code) && !seen.has(code))) {
        mapped[c.id] = code;
        if (code) seen.add(code);
      } else mapped[c.id] = null;
    }
    return mapped;
  } catch { return defaults(); }
}
export function displayCode(code) {
  if (!code) return 'Not assigned';
  const known = { Space: 'Space', Semicolon: ';', Quote: "'", Comma: ',', Period: '.',
    BracketLeft: '[', BracketRight: ']', Backslash: '\\', Minus: '-', Equal: '=', Backquote: '`',
    ArrowLeft: '←', ArrowRight: '→', ArrowUp: '↑', ArrowDown: '↓', Escape: 'Esc', Enter: 'Enter' };
  return known[code] || code.replace(/^Key/, '').replace(/^Digit/, '').replace(/^Numpad/, 'Num ');
}
function codeFromEvent(event) {
  if (event.code && event.code !== 'Unidentified' && event.code !== 'Process') return event.code;
  const k = event.key;
  if (/^[a-z]$/i.test(k)) return `Key${k.toUpperCase()}`;
  if (/^[0-9]$/.test(k)) return `Digit${k}`;
  const chars = { ' ': 'Space', ';': 'Semicolon', ':': 'Semicolon', "'": 'Quote', '"': 'Quote',
    ',': 'Comma', '<': 'Comma', '.': 'Period', '>': 'Period', '[': 'BracketLeft', ']': 'BracketRight',
    '-': 'Minus', '=': 'Equal' };
  return chars[k] || (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'Enter', 'Escape', 'Backspace', 'Delete'].includes(k) ? k : null);
}

export function createShortcuts({ dialog, onDown, onUp, shouldIgnore }) {
  const table = dialog.querySelector('#key-grid');
  const captureInput = dialog.querySelector('#key-capture');
  const prompt = dialog.querySelector('#key-prompt');
  let bindings = safeRead();
  let waiting = null;
  let inverse = new Map();
  const refresh = () => { inverse = new Map(Object.entries(bindings).filter(([id, key]) => key).map(([id, key]) => [key, id])); };
  const write = () => { try { localStorage.setItem(KEY, JSON.stringify(bindings)); } catch { /* blocked storage */ } refresh(); };
  const buttonFor = (control) => {
    const button = document.createElement('button');
    button.type = 'button';
    button.className = 'keybind';
    button.textContent = displayCode(bindings[control.id]);
    button.setAttribute('aria-label', `Change shortcut for ${control.name}: ${button.textContent}`);
    button.addEventListener('click', () => {
      waiting = control.id;
      prompt.textContent = `Press a key for ${control.name}. Backspace clears; Esc cancels.`;
      captureInput.hidden = false;
      captureInput.value = '';
      captureInput.focus();
      button.textContent = 'Press key…';
    });
    return button;
  };
  const render = () => {
    table.replaceChildren();
    let last = '';
    for (const control of CONTROLS) {
      if (control.group !== last) {
        last = control.group;
        const heading = document.createElement('h3'); heading.textContent = last;
        table.append(heading);
      }
      const row = document.createElement('div'); row.className = 'binding-row';
      const label = document.createElement('span'); label.textContent = control.name;
      row.append(label, buttonFor(control));
      table.append(row);
    }
  };
  function stopCapture() { waiting = null; captureInput.value = ''; captureInput.hidden = true; prompt.textContent = 'Tap a shortcut to reassign it.'; render(); }
  function onCapture(e) {
    if (!waiting) return;
    e.stopImmediatePropagation(); e.preventDefault();
    if (e.key === 'Escape') return stopCapture();
    if (e.key === 'Tab' || e.key === 'Shift' || e.key === 'Control' || e.key === 'Alt' || e.key === 'Meta') return;
    if (e.key === 'Backspace' || e.key === 'Delete') { bindings[waiting] = null; write(); return stopCapture(); }
    if (e.altKey || e.ctrlKey || e.metaKey) return;
    const code = codeFromEvent(e);
    if (!code) return;
    // Reassign duplicate keys, rather than triggering two controls at once.
    for (const id of Object.keys(bindings)) if (id !== waiting && bindings[id] === code) bindings[id] = null;
    bindings[waiting] = code;
    write(); stopCapture();
  }
  // Capture on the document so both hardware and mobile soft keyboards can remap shortcuts.
  document.addEventListener('keydown', onCapture, true);
  captureInput.addEventListener('input', () => {
    if (!waiting || !captureInput.value) return;
    const k = [...captureInput.value].at(-1);
    const code = codeFromEvent({ code: '', key: k });
    if (code) {
      for (const id of Object.keys(bindings)) if (id !== waiting && bindings[id] === code) bindings[id] = null;
      bindings[waiting] = code;
      write(); stopCapture();
    }
  });
  dialog.addEventListener('close', stopCapture);
  refresh(); render();
  return {
    actionFor: code => inverse.get(code),
    isCapturing: () => waiting !== null,
    shouldIgnore,
    open() { stopCapture(); dialog.showModal(); },
    close() { dialog.close(); },
    reset() { bindings = defaults(); write(); stopCapture(); },
    get() { return { ...bindings }; },
    onDown, onUp,
  };
}
