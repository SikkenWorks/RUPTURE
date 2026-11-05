// Paint only when the panel state changes. Keep a short rAF run for encoder animations;
// never make the 1280×820 FM-1 shell a permanent 60 FPS workload.
export function createPanelScheduler({ requestFrame, render }) {
  let pending = false;
  let dirty = false;
  let painting = false;
  const request = () => {
    if (pending) return;
    pending = true;
    requestFrame(tick);
  };
  function tick() {
    pending = false;
    if (!dirty && !painting) return;
    dirty = false;
    // render returns true only while a short hardware-control animation is active.
    painting = !!render();
    if (dirty || painting) request();
  }
  return {
    invalidate() { dirty = true; request(); },
    isPending() { return pending; },
  };
}
