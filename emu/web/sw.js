const CACHE_NAMESPACE = 'fm1-joakim-' + new URL('./', self.location).pathname.replace(/[^a-z0-9]+/gi, '-');
const CACHE = CACHE_NAMESPACE + 'v2.4-' + 'rupture';
const CORE = ['./', './index.html', './style.css', './main.js', './firmware-policy.js', './render-scheduler.js', './accuracy.js', './calibration.js', './calibration-ui.js', './shortcuts.js', './protect.js', './pwa.js', './manifest.webmanifest', './worklet.js', './jit.js', './fwsc.js', './persistence.js', './tryout.js', './firmwares.json', './RUPTURE.fwsc', './favicon.png', './icons/icon-192.png', './icons/icon-512.png', './icons/apple-touch-icon.png'];
self.addEventListener('install', e => {
  e.waitUntil(caches.open(CACHE).then(async cache => {
    await cache.addAll(CORE);
    // The source-only upload intentionally lacks WASM, while the GitHub build
    // ships it. Cache it for truly offline PWA use wherever present.
    try { await cache.add('./fm1_web.wasm'); } catch { /* upload-only edition */ }
  }).then(() => self.skipWaiting()));
});
self.addEventListener('activate', e => e.waitUntil(caches.keys().then(keys => Promise.all(keys.filter(k => k.startsWith(CACHE_NAMESPACE) && k !== CACHE).map(k => caches.delete(k)))).then(() => self.clients.claim())));
self.addEventListener('fetch', e => {
  if (e.request.method !== 'GET') return;
  const url = new URL(e.request.url);
  const local = url.origin === self.location.origin && url.pathname.startsWith(new URL('./', self.location).pathname);
  const engineMirror = url.href === 'https://joakimeriksson.github.io/fm1-emu/fm1_web.wasm';
  if (!local && !engineMirror) return;
  e.respondWith((async () => {
    const cache = await caches.open(CACHE);
    if (engineMirror) {
      try {
        const response = await fetch(e.request);
        if (response.ok) { await cache.put(e.request, response.clone()); return response; }
      } catch { /* offline */ }
      return (await cache.match(e.request)) || Response.error();
    }
    if (url.pathname.endsWith('/fm1_web.wasm')) {
      const hit = await cache.match(e.request);
      if (hit) return hit;
      const response = await fetch(e.request);
      if (response.ok) await cache.put(e.request, response.clone());
      return response;
    }
    const cached = await cache.match(e.request);
    if (cached) return cached;
    try {
      const response = await fetch(e.request);
      if (response.ok) await cache.put(e.request, response.clone());
      return response;
    } catch { return cached || Response.error(); }
  })());
});
