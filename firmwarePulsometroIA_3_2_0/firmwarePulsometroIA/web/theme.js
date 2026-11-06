// Tema compartido entre el juego y el portal. Funciona sin internet.
(function () {
  const root = document.documentElement;
  let saved;
  try { saved = localStorage.getItem('ism-theme'); } catch (_) {}
  root.dataset.theme = saved === 'light' || saved === 'dark' ? saved :
    (window.matchMedia && matchMedia('(prefers-color-scheme: light)').matches ? 'light' : 'dark');
  const sun = '<circle cx="12" cy="12" r="4"/><path d="M12 2v2m0 16v2M2 12h2m16 0h2M5 5l1.5 1.5m11 11L19 19M5 19l1.5-1.5m11-11L19 5"/>';
  const moon = '<path d="M20.9 13A9 9 0 0 1 11 3.1 9 9 0 1 0 20.9 13Z"/>';
  function update() {
    document.querySelectorAll('.theme-toggle').forEach(button => {
      const light = root.dataset.theme === 'light';
      button.innerHTML = '<svg viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="currentColor" stroke-width="2" aria-hidden="true">' + (light ? moon : sun) + '</svg>';
      button.setAttribute('aria-label', light ? 'Activar modo oscuro' : 'Activar modo claro');
      button.title = button.getAttribute('aria-label');
      button.onclick = () => {
        root.dataset.theme = light ? 'dark' : 'light';
        try { localStorage.setItem('ism-theme', root.dataset.theme); } catch (_) {}
        update();
      };
    });
  }
  window.ISMTheme = {update};
  document.addEventListener('DOMContentLoaded', update);
})();
