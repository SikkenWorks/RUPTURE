// Disable text selection and context menus, while keeping inputs, shortcuts and touch controls usable.
(() => {
  const editable = target => target instanceof Element && !!target.closest('input, textarea, select, [contenteditable="true"]');
  document.addEventListener('contextmenu', event => event.preventDefault());
  document.addEventListener('selectstart', event => { if (!editable(event.target)) event.preventDefault(); });
  document.addEventListener('dragstart', event => { if (!editable(event.target)) event.preventDefault(); });
  document.documentElement.style.userSelect = 'none';
  document.documentElement.style.webkitUserSelect = 'none';
  document.addEventListener('DOMContentLoaded', () => {
    document.querySelectorAll('input, textarea, select').forEach(node => {
      node.style.userSelect = 'text';
      node.style.webkitUserSelect = 'text';
    });
  });
})();
