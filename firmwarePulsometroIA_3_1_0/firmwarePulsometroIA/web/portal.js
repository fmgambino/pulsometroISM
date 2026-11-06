// Identidad del portal usando los puntos de extensión de WiFiManager.
document.addEventListener('DOMContentLoaded', () => {
  document.documentElement.lang = 'es';
  document.title = 'Conectar Wi-Fi · Instituto San Miguel';
  const header = document.createElement('header');
  header.className = 'ism-header';
  header.innerHTML = '<img src="/logo.png" alt="Instituto San Miguel"><h1>PULSÓMETRO</h1><p>Configuración de redes Wi-Fi</p><button type="button" class="theme-toggle" aria-label="Cambiar tema"></button><p><a href="/game">Ir al juego sin internet</a></p>';
  document.body.prepend(header);
  const footer = document.createElement('footer');
  footer.className = 'ism-footer';
  footer.innerHTML = 'Diseño y Desarrollo del Club de Robótica e IA<br>Instituto San Miguel · Tucumán · Argentina<br>© 2026 · Ing. Fernando Gambino';
  document.body.append(footer);
  if (window.ISMTheme) window.ISMTheme.update();
});
