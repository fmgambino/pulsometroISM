// Interfaz local: los datos de tiempo y errores siempre llegan desde la ESP32.

const $ = id => document.getElementById(id);
let busy = false;
const fmt = ms => {
    let c = Math.floor(ms / 10);
    return String(Math.floor(c / 6000)).padStart(2, '0') + ':' +
           String(Math.floor(c / 100) % 60).padStart(2, '0') + ',' +
           String(c % 100).padStart(2, '0')
};
const stars = n => '★'.repeat(Math.max(0, n)) + '☆'.repeat(Math.min(3, 3 - n));
// Enviar acciones mediante POST; mostrar el texto de error entregado por la placa.
async function post(path, values = {}) {
    const r = await fetch(path, {method : 'POST', body : new URLSearchParams(values)});
    if (!r.ok)
        throw Error(await r.text());
    return r.text()
}
// Usar textContent: los nombres de participantes no se interpretan como HTML.
function row(values) {
    let tr = document.createElement('tr');
    values.forEach(v => {
        let td = document.createElement('td');
        td.textContent = v;
        tr.append(td)
    });
    return tr
}
// Actualizar el tablero cada 200 ms y recuperar conexión sin recargar la página.
async function poll() {
    // Recuperar rápido si el dispositivo se desconecta durante una petición.
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 3000);
    try {
        const r = await fetch('/api/state', {cache : 'no-store', signal : controller.signal});
        if (!r.ok)
            throw Error();
        const s = await r.json();
        $('link').textContent =
            'ESP32 conectada · ' + (s.internet ? 'Wi-Fi del instituto conectado' : 'Red local');
        $('phase').textContent = ({
            idle : 'Listo para jugar',
            countdown : 'Preparados… ' + s.countdown,
            running : '¡En marcha!',
            won : '¡Recorrido completado!',
            lost : 'Sin vidas · ¡Seguí practicando!',
            cancelled : 'Partida cancelada',
            timeout : 'Tiempo máximo alcanzado'
        })[s.phase];
        $('time').textContent = fmt(s.ms);
        $('player').textContent = s.name || '¿Te animás?';
        $('stars').textContent = stars(s.phase === 'lost' ? 0 : 3 - s.errors);
        $('lives').textContent = (3 - s.errors) + ' vidas restantes · ' + s.errors + '/3 usadas';
        busy = [ 'running', 'countdown' ].includes(s.phase);
        $('start').disabled = busy || !s.ready;
        $('cancel').disabled = !busy;
        $('notice').textContent = s.touch                   ? 'Separá el aro del cobre'
                                  : s.finish                ? 'Soltá el pulsador B'
                                  : s.phase === 'countdown' ? 'Esperá la señal de inicio'
                                                            : '';
        renderRanking(s.ranking);
        updateAssistant(s);
        $('result').textContent = s.phase === 'won' ? 'Resultado: ' + fmt(s.ms) + ' · ' + s.errors +
                                                          ' vidas usadas · ' + stars(3 - s.errors)
                                                    : '';
        $('weather').textContent =
            s.weather.valid
                ? s.weather.icon + ' Tucumán: ' + s.weather.temp.toFixed(1) + ' °C · ' +
                      (s.weather.stale
                           ? 'sin actualizar'
                           : 'actualizado hace ' + Math.floor(s.weather.age / 60) + ' min')
                : 'Clima no disponible';
        $('datetime').textContent = s.datetime;
        $('assistantText').textContent = s.assistantText;
        $('assistantStatus').textContent = s.assistantStatus + ' · Consultas: ' + s.aiCalls + '/40';
        $('faceEmoji').textContent = ({
                                         happy : '😊',
                                         sad : '😢',
                                         surprised : '😮',
                                         thinking : '🤔',
                                         sleepy : '😴',
                                         excited : '🤩',
                                         neutral : '😐'
                                     })[s.emotion] ||
                                     '😊';
        $('askButton').disabled = busy;
        $('portal').href = location.origin + '/wifi';
        $('portal').hidden = !s.wifiPortal;
        $('network').textContent = 'IP del instituto: ' + s.ip +
                                   ' · Motivo última desconexión: ' + s.wifiReason + ' · Reposo: ' + (s.idleSound ? 'con sonido' : 'silencioso');
    } catch (e) {
        $('link').textContent = 'Sin conexión con ESP32 · reconectando';
        $('start').disabled = true;
        $('cancel').disabled = true
    } finally {
        clearTimeout(timeout);
        setTimeout(poll, 200)
    }
}
// Controles del participante: el inicio real se valida en el firmware.
$('play').onsubmit = async e => {
    e.preventDefault();
    $('start').disabled = true;
    try {
        await post('/api/play', {name : $('name').value})
    } catch (e) {
        alert(e.message)
    }
};
$('cancel').onclick = async () => {
    if (confirm('¿Cancelar la partida actual?'))
        try {
            await post('/api/cancel')
        } catch (e) {
            alert(e.message)
        }
}; // Configuración del organizador y acceso al portal WiFiManager.
$('wifi').onclick = async () => {
    try {
        await post('/admin/wifi');
        alert('Portal abierto. Pulsá Ir al portal Wi-Fi.')
    } catch (e) {
        alert(e.message)
    }
};
$('wifiClose').onclick = async () => {
    try {
        await post('/admin/wifi-close')
    } catch (e) {
        alert(e.message)
    }
};
$('interact').onsubmit = async e => {
    e.preventDefault();
    try {
        let text = await post('/api/interact', {text : $('ask').value});
        $('ask').value = '';
        alert(text)
    } catch (e) {
        alert(e.message)
    }
};
$('sound').onclick = async () => {
    try {
        await post('/admin/sound')
    } catch (e) {
        alert(e.message)
    }
};
// IDs estables preservan la selección aunque se actualice el tablero.
const selected = new Set();
let rankingCache = '', visibleIds = [];
function selectionState() {
    $('deleteSelected').disabled = busy || selected.size === 0;
    $('selectAll').checked = visibleIds.length > 0 && visibleIds.every(id => selected.has(id));
    $('selectAll').indeterminate = selected.size > 0 && !$('selectAll').checked;
    $('selectAll').disabled = busy || !visibleIds.length;
    document.querySelectorAll('#rank input[type=checkbox]').forEach(cb=>cb.disabled=busy);
    document.querySelectorAll('.delete-row').forEach(b => b.disabled = busy);
}
async function deleteRecords(ids) {
    if (!ids.length || !confirm('¿Eliminar ' + ids.length + ' registro(s) de la memoria de la ESP32?')) return;
    try { await post('/admin/delete', {ids: JSON.stringify(ids)}); ids.forEach(id=>selected.delete(id)); rankingCache=''; }
    catch(e) { alert(e.message); }
}
function renderRanking(records) {
    visibleIds=records.map(x=>x.id);
    for(const id of selected) if(!visibleIds.includes(id)) selected.delete(id);
    const key=JSON.stringify(records);
    if(key!==rankingCache) {
        rankingCache=key;
        $('rank').replaceChildren(...records.map(x=>{
            const tr=row([x.position,x.name,fmt(x.ms),x.errors+'/3',stars(3-x.errors)]);
            const td=document.createElement('td'), checkbox=document.createElement('input');
            checkbox.type='checkbox';checkbox.checked=selected.has(x.id);checkbox.setAttribute('aria-label','Seleccionar resultado de '+x.name);
            checkbox.onchange=()=>{checkbox.checked?selected.add(x.id):selected.delete(x.id);selectionState();};
            checkbox.dataset.id=x.id;td.append(checkbox);tr.prepend(td);
            const action=document.createElement('td'),button=document.createElement('button');
            button.type='button';button.className='secondary delete-row';button.textContent='Eliminar';button.onclick=()=>deleteRecords([x.id]);
            action.append(button);tr.append(action);return tr;
        }));
    }
    selectionState();
}
$('selectAll').onchange=()=>{
    visibleIds.forEach(id=>$('selectAll').checked?selected.add(id):selected.delete(id));
    document.querySelectorAll('#rank input[type=checkbox]').forEach(cb=>cb.checked=selected.has(Number(cb.dataset.id)));
    selectionState();
};
$('deleteSelected').onclick=()=>deleteRecords([...selected]);
// Audio exclusivamente en el navegador, tras un gesto explícito del usuario.
let audioEnabled=false, audioContext, lastSpoken='', lastPhase='', lastErrors=0;
function chirp(hz=700) {
    if(!audioEnabled || !audioContext) return;
    const oscillator=audioContext.createOscillator(), gain=audioContext.createGain();
    oscillator.frequency.value=hz;gain.gain.setValueAtTime(0.06,audioContext.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.001,audioContext.currentTime+0.12);
    oscillator.connect(gain);gain.connect(audioContext.destination);oscillator.start();oscillator.stop(audioContext.currentTime+0.13);
}
function speak(text) {
    if(!audioEnabled || !('speechSynthesis' in window)) return;
    speechSynthesis.cancel();
    const utterance=new SpeechSynthesisUtterance(text);
    const voices=speechSynthesis.getVoices();
    utterance.voice=voices.find(v=>v.lang.toLowerCase()==='es-ar') || voices.find(v=>v.lang.startsWith('es')) || null;
    utterance.lang=utterance.voice?utterance.voice.lang:'es-AR';utterance.rate=1;
    utterance.onstart=()=>$('assistantAvatar').classList.add('speaking');
    const end=()=>$('assistantAvatar').classList.remove('speaking');
    utterance.onend=end;utterance.onerror=()=>{end();$('voiceStatus').textContent='Voz no disponible. Revisá las voces en español del dispositivo.';};
    speechSynthesis.speak(utterance);
}
$('voiceToggle').onclick=()=>{
    audioEnabled=!audioEnabled;
    $('voiceToggle').textContent=audioEnabled?'Silenciar voz y sonidos':'Activar voz y sonidos';
    $('voiceStatus').textContent=audioEnabled?'Audio activado. La voz depende del navegador y las voces instaladas.':'Audio desactivado';
    if(audioEnabled) {
        const Audio=window.AudioContext||window.webkitAudioContext;
        if(Audio){try{audioContext=audioContext||new Audio();audioContext.resume().catch(()=>{});}catch(_){}}
        speak('¡Hola! Soy tu asistente Pulsómetro. ¿Jugamos?');chirp(900);
    } else {if('speechSynthesis' in window)speechSynthesis.cancel();$('assistantAvatar').classList.remove('speaking');}
};
function updateAssistant(s) {
    $('assistantAvatar').dataset.emotion=s.emotion;
    $('assistantAvatar').setAttribute('aria-label','Asistente: '+s.emotion);
    if(s.assistantText!==lastSpoken){lastSpoken=s.assistantText;speak(s.assistantText);}
    if(s.phase!==lastPhase){if(s.phase==='running')chirp(1400);if(s.phase==='won')chirp(2000);lastPhase=s.phase;}
    if(s.errors>lastErrors)chirp(250);lastErrors=s.errors;
}
poll();
