# PULSÓMETRO · Instituto San Miguel
Firmware 2.0 · WiFiManager + asistente emocional · ESP32 WROOM-32 DevKit · Arduino / PlatformIO.

## Montaje y carga
1. Abrir esta carpeta (la que contiene platformio.ini) en VS Code con PlatformIO.
2. Revisar include/config.h: pines, dirección OLED y contraseñas. Las credenciales predeterminadas son públicas en este proyecto; cambiarlas antes del uso.
3. Conectar por USB, pulsar Build y después Upload. Monitor serie: 115200.
   Comandos equivalentes: `pio run`, `pio run -t upload`, `pio device monitor`.
4. No requiere subir filesystem: la web, las imágenes y la CA están compiladas en el firmware.
5. Conectar el celular/notebook a PULSOMETRO-ISM (clave inicial SanMiguel2026).
6. Abrir http://pulsometro.local o http://192.168.4.1. Si el teléfono avisa que la red no tiene internet, mantener la conexión. Puede ser necesario desactivar el cambio automático a datos móviles.
7. Abrir Configuración → Abrir WiFiManager. Usuario admin, clave inicial ClubISM2026. Pulsar el enlace Ir al portal Wi-Fi (http://192.168.4.1:8080 desde la red propia), seleccionar Wi-Fi 2,4 GHz y guardar. El portal se abre también los primeros 3 minutos del arranque. Se puede iniciar Play sin configurar internet: el portal se cierra automáticamente.
8. La red propia PULSOMETRO-ISM queda disponible aunque no exista router. WiFiManager administra el portal y guarda las credenciales en el almacenamiento Wi-Fi del ESP32. No se requiere editar SSID/clave para el uso normal.
9. Al conectar internet se sincronizan fecha/hora NTP y clima. La web mantiene su puerto 80 y el portal WiFiManager usa 8080 para evitar conflicto de servidores. No depende de una ventana cautiva automática: usar el enlace explícito.

Se usa huge_app.csv para una aplicación de hasta 3 MB en flash de 4 MB, sin OTA de doble partición. No usar placas con menos de 4 MB de flash.

La plataforma espressif32 6.9.0 fija Arduino-ESP32 2.0.17; se utiliza la API LEDC por canal de esa versión. No cambiar a Arduino 3 sin adaptar LEDC. La primera compilación requiere internet para descargar herramientas y bibliotecas; el juego ya cargado funciona sin internet.

## Conexiones
| Parte | ESP32 / conexión |
|---|---|
| OLED SSD1306 128x64 SDA | GPIO21 |
| OLED SCL | GPIO22 |
| OLED VCC, GND | 3,3V, GND |
| Aro metálico con cable flexible | GND |
| Alambre de cobre desnudo | Resistencia serie 1kΩ → GPIO27 |
| Pull-up contacto | Resistencia 10kΩ entre GPIO27 y 3,3V |
| Pulsador final B, normalmente abierto | Un terminal GPIO33, otro GND |
| Control buzzer | GPIO25 → 1kΩ → base NPN BC337/2N2222 |
| NPN | Emisor GND; colector al negativo del buzzer |
| Buzzer positivo | Alimentación de acuerdo con su tensión nominal (3,3V o 5V) |
| Base NPN | Resistencia 10kΩ a GND |
| Alimentación DevKit | USB 5V |

Todos los GND unidos. El pulsador usa INPUT_PULLUP y antirrebote de 35 ms. Para cables largos añadir pull-up externa 10kΩ en GPIO33. Verificar el pinout del transistor concreto. Usar piezo PASIVO para las distintas notas; si es un transductor magnético inductivo, añadir diodo de rueda libre en paralelo (cátodo al positivo). Nunca aplicar 5V a un GPIO. Metales de detección solo a GND / señal 3,3V. No conectar a red eléctrica.

## Reglas implementadas
- Sin sensor A: el organizador coloca físicamente el aro en el inicio. Pulsar Play inicia cuenta regresiva 3 s; al terminar arranca el tiempo. El firmware no puede verificar la posición A.
- No permite iniciar si B está presionado o hay contacto con el cobre. Un contacto o pulsación B durante la cuenta cancela el intento.
- Contactos capturados por interrupción, incluso breves entre iteraciones. Un contacto continuo descuenta una sola vida. Debe haber 150 ms sin contacto para rearmar.
- El pulsador B finaliza al presionarlo (flanco estable). No finaliza por estar mantenido desde antes del inicio.
- 3 errores: derrota. Si el tercer error coincide con la llegada, se pierde.
- Si la llegada coincide con el primer o segundo error, ese error se registra y se completa.
- Tiempo máximo 10 minutos: finaliza sin clasificar. Cancelar tampoco clasifica.
- El cronómetro sigue en la ESP32 aunque se cierre la web. Se transmite estado cada 200 ms; no depende del reloj del navegador.
- Estrellas: 0 errores ★★★, 1 ★★☆, 2 ★☆☆, derrota ☆☆☆.
- Ranking TOP 30 de recorridos completos: menos errores, luego menor tiempo. Tiempos iguales en milisegundos y errores iguales comparten posición. Muestra centésimas, almacena milisegundos.
- Ranking persistente en NVS. Se escribe al incorporar un resultado, no en cada contacto. Reiniciar aborta una partida en curso; conserva ranking y Wi-Fi. Para conservar tiempos por debajo del TOP 30 se necesitaría ampliar almacenamiento.
- Cancelación disponible para los dispositivos conectados. Abrir de nuevo el portal Wi-Fi/borrar ranking/cambiar sonido requiere autenticación del organizador. El portal WiFiManager en puerto 8080 es accesible para cualquier cliente de la red local mientras está abierto (3 min al encender o después de abrirlo); usar una clave AP propia y red de confianza. La red local es de confianza; HTTP Basic no cifra la sesión. No exponer puertos a internet.

## Pantalla y sonidos
Arranque: identificación institucional 2 s, créditos 2,5 s. Reposo: cara animada, fecha/hora y clima alternados cada 6 s; pitido breve cada 20 s, desactivable. En partida: nombre, estado, cronómetro y vidas. Buzzer de error 350 Hz, salida 1800 Hz, victoria 2400 Hz, derrota 180 Hz.
OLED SSD1306 0x3C predeterminada: si es 0x3D, editar config.h. SH1106 requiere otra biblioteca/constructor. Nombres UTF-8 se muestran correctamente en la web; la fuente básica de Adafruit en OLED no soporta todos los acentos. La OLED usa textos sin acentos. Si falta OLED, el juego y la web continúan funcionando.

## Clima
API Open-Meteo, coordenadas -26.8241,-65.2226, San Miguel de Tucumán. Campos temperatura, código WMO e indicador día/noche; consulta cada 15 min y reintento al minuto si falla. HTTPS con validación de CA ISRG Root X1 y fecha NTP. No utiliza setInsecure. Si el proveedor cambia su cadena de confianza, habrá que actualizar include/weather_ca.h. API y atribución: https://open-meteo.com/en/docs . Uso educativo no comercial conforme a condiciones del proveedor.
La descarga va en una tarea separada del procesamiento del juego. Sin Wi-Fi o NTP se omite la consulta, sin bloquear partidas. El último dato se conserva en RAM y aparece como sin actualizar al desconectarse/fallar o superar 30 min; no se conserva entre reinicios. No es una medición de un sensor local.

## Logo institucional: pendiente de archivo oficial
NO se incluyó ni se inventó el escudo oficial, porque no fue suministrado. Se incluye una placa tipográfica provisional ISM en OLED y web.
- Web: reemplazar WEB_LOGO en include/branding.h por el SVG oficial (sin scripts ni dependencias externas).
- OLED: convertir el logo oficial a bitmap monocromo 128x64, reemplazar OFFICIAL_LOGO (1024 bytes, formato Adafruit drawBitmap) y activar HAS_OFFICIAL_LOGO=true.

## Archivos
- src/main.cpp: conexiones, OLED, web, ranking NVS, mDNS, clima TLS y sonidos.
- include/game.h: máquina de estados independiente y testeable.
- include/web.h: interfaz responsive sin CDN.
- include/config.h: pines, red, contraseñas.
- include/branding.h: recursos institucionales reemplazables.
- include/weather_ca.h: autoridad de confianza HTTPS para clima.
- include/assistant.h: cola de consultas API, validación JSON y siete expresiones OLED.
- include/ai_ca.h: raíces de confianza TLS para la API IA.
- tests/game_test.cpp: pruebas nativas de reglas.

## Verificación en banco antes de exhibir
1. Encender sin Wi-Fi: revisar arranque, carita y acceso por IP.
2. Iniciar con cobre tocado o B mantenido: debe rechazar.
3. Tocar durante cuenta regresiva: debe cancelar.
4. Mantener contacto: solo un error; separar 150 ms y repetir tres veces: derrota.
5. Iniciar de nuevo, llegar y presionar B: debe guardar tiempo y estrellas.
6. Reiniciar: TOP 30 debe persistir. Cerrar web durante partida: el tiempo sigue.
7. Activar Wi-Fi, verificar clima/fecha y apagar router durante una partida: juego sigue.
8. Revisar que el mango accione B solo al llegar; el firmware confía en la pulsación física y no verifica que todo el recorrido se haya realizado.

Prueba nativa: `c++ -std=c++11 -I include tests/game_test.cpp -o /tmp/pulsometro_test && /tmp/pulsometro_test`.

## Fecha y hora
Se usa SNTP/NTP (pool.ntp.org y time.google.com), no una API de IA ni el reloj del navegador. Zona POSIX ART3, UTC-3 para Tucumán. La fecha y hora aparecen en OLED y web. Sin internet después de sincronizar, el reloj del sistema continúa; al cortar alimentación se pierde la hora hasta la próxima sincronización, porque no hay RTC con pila. El tiempo de la partida siempre se mide con millis y es independiente de NTP.

## Asistente virtual emocional (nuevo)
Integración real de OpenAI Chat Completions por HTTPS. Modelo configurable gpt-4.1-mini, respuestas JSON validadas: emotion y text. Referencia oficial: https://developers.openai.com/api/reference/resources/chat/subresources/completions/methods/create

Para activar: crear una clave API propia con acceso al modelo y facturación/cuota del proveedor; en include/config.h completar OPENAI_API_KEY, compilar y cargar de nuevo. No hay una clave de ejemplo funcional. La suscripción a una aplicación de chat no se utiliza como credencial API. El firmware no devuelve la clave al navegador ni por el monitor serie. La clave compilada puede extraerse si alguien tiene acceso físico al dispositivo: usar una clave restringida y un proyecto con límite de gasto; para uso público permanente conviene un intermediario que guarde el secreto.

No se puede completar una llamada real sin la clave del usuario; esta entrega no incluye llamadas API pagadas ni una prueba autenticada.

- Desde la web, escribir un saludo o pedir consejo y pulsar Interactuar. El texto se envía a OpenAI cuando hay conexión y clave. No incluir datos personales. Se muestran estado y respuesta.
- Tras victoria/derrota se solicita un mensaje de felicitación/ánimo si hay cuota disponible, sin enviar el nombre del participante ni ranking.
- Siete expresiones: feliz, triste, sorprendida, pensativa, somnolienta, entusiasmada y neutral. Ojos con parpadeo y movimiento; boca y cejas cambian según emoción.
- Contacto, inicio, victoria y derrota tienen reacciones inmediatas locales. La API solo selecciona emoción/frase; no controla vidas, tiempos, sonidos de error ni la lógica del juego.
- Sin clave, sin conexión, sin fecha TLS válida, ante respuesta inválida, error o cuota agotada, conserva reacciones locales. No pretende comprender conversaciones sin API. No hay micrófono ni síntesis de voz: interacción por texto y pitidos.
- Una consulta como máximo cada 60 s y 40 por encendido (incluidas consultas automáticas y fallidas). Este límite se reinicia al cortar alimentación y NO sustituye el límite de gasto del proveedor. No hay consultas periódicas automáticas en reposo.
- Solo una petición pendiente. Las consultas se ejecutan en la tarea de red, secuencialmente con el clima, fuera del bucle de juego. Una descarga lenta no congela cronómetro ni sensores. El timeout de lectura se limita a 10 s y el de conexión a 5 s.
- Respuesta breve hasta 80 caracteres ASCII en la OLED. El texto aparece desplazándose en reposo y también en la web. No se conserva historial de conversación.
- El portal WiFiManager se procesa solo entre partidas y se detiene antes de empezar. Escanear redes o guardar credenciales puede demorar la interfaz mientras se configura; nunca se procesa esa configuración con la partida activa.

## Pruebas adicionales de versión 2
- WiFiManager: elegir red válida, probar clave incorrecta, cerrar portal y jugar sin router; reiniciar y comprobar reconexión.
- NTP: verificar fecha/hora UTC-3 en OLED y web; desconectar router y confirmar continuidad del reloj sin reiniciar.
- IA sin clave: Interactuar conserva carita local y no hace llamadas.
- IA con clave propia: confirmar respuesta y emoción; repetir antes de 60 s y verificar límite. Probar error API y juego durante consulta en curso.
- Confirmar que ni /api/state ni los recursos web contienen la clave API.
