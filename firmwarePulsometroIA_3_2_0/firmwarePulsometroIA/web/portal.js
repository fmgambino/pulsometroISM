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
  const status=document.createElement('p');status.setAttribute('role','status');
  header.append(status);
  async function checkWiFi(){
    try{const r=await fetch('/api/state',{cache:'no-store'});const s=await r.json();
    const reasons={201:'No se encontró la red. Acercá el router y verificá 2,4 GHz.',202:'Falló la autenticación. Revisá la contraseña y seguridad del router.',204:'Falló el intercambio de claves. Revisá la contraseña.',15:'El router no completó el intercambio de claves. Revisá contraseña y WPA2.'};
    status.textContent=s.internet?'Conectado al router. IP: '+s.ip:(s.wifiReason?'Sin conexión: '+(reasons[s.wifiReason]||'motivo '+s.wifiReason):'Seleccioná una red de 2,4 GHz y guardá. La conexión puede tardar hasta 25 segundos.');
    }catch(_){status.textContent='Conectando con la ESP32… Si cambió de canal, reconectate a PULSOMETRO-ISM.';}
    setTimeout(checkWiFi,3000);
  }
  checkWiFi();
  if (window.ISMTheme) window.ISMTheme.update();
});
