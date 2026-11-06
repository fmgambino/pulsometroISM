# PULSÓMETRO — Instituto San Miguel
Versión 3.2.0 · ESP32 DevKit WROOM-32 de 38 pines, flash 4 MB · PlatformIO + Arduino.
Diseño y Desarrollo del Club de Robótica e IA.

## Empezar
1. Descomprimir el ZIP en una carpeta nueva. Abrir en VS Code la carpeta que contiene `platformio.ini`.
2. Conectar la placa con un cable USB de datos. Compilar con **PlatformIO: Build** y cargar con **PlatformIO: Upload**. Monitor serie a 115200.
3. La primera compilación descarga plataforma, compilador y bibliotecas; necesita internet y puede tardar. No cancelar mientras instala dependencias. Arduino-ESP32 2.0.17 está fijado mediante espressif32 6.9.0.
4. Conectar el celular/notebook a **PULSOMETRO-ISM**, contraseña inicial **SanMiguel2026**. Mantener conexión aunque el teléfono indique que no hay internet.
5. Abrir **http://pulsometro.local** o **http://192.168.4.1**. La resolución .local depende del sistema del cliente; la IP es la alternativa.
6. Para configurar clima/fecha/hora: abrir **http://192.168.4.1/wifi**, seleccionar la red de 2,4 GHz y guardar. La raíz muestra WiFiManager mientras no hay red configurada. El juego siempre está en **http://192.168.4.1/game**. No utilizar el puerto 8080. El portal no vence a los tres minutos; durante partidas se bloquean los cambios de red.
7. Revisar `docs/VALIDACION.md` para conocer qué se verificó realmente en esta entrega.

Cambiar las contraseñas de ejemplo en `include/config.h` antes de usarlo con público. No publicar ni redirigir puertos del dispositivo a internet.

## Grabar un binario
Cada compilación exitosa exporta `release/PULSOMETRO_ESP32_38P.bin`, imagen unificada con bootloader, particiones y aplicación. Se graba en **0x0000** con una herramienta compatible con ESP32 clásico. No grabar el archivo `firmware.bin` en 0x0000: es solamente la aplicación y corresponde a 0x10000.

Si el paquete recibido contiene el binario unificado, `GRABAR_WINDOWS.bat COM5` permite grabarlo usando las herramientas de PlatformIO ya instaladas. Sustituir COM5 por el puerto correcto. Si se queda en Connecting, mantener BOOT mientras inicia la conexión y soltarlo cuando comienza a escribir. Alternativa: abrir el proyecto y usar Upload, que calcula los offsets automáticamente. Al cambiar credenciales/clave API o código, compilar y cargar de nuevo.

No usa LittleFS/SPIFFS para la web: no hay que ejecutar Upload Filesystem. La partición huge_app permite hasta 3 MB de aplicación en flash de 4 MB y no implementa actualización OTA de doble partición.

## Conexiones
Los números son **GPIO**, no el orden de las patas del conector.

| Componente | Conexión |
|---|---|
| OLED SSD1306 128×64 VCC / GND | 3V3 / GND |
| OLED SDA / SCL | GPIO21 / GPIO22 |
| Aro metálico con mango aislante | GND con cable flexible |
| Cobre desnudo del recorrido | A GPIO27 mediante resistencia serie 1 kΩ |
| Pull-up de detección | 10 kΩ entre GPIO27 y 3V3, antes de la resistencia serie |
| Pulsador B normalmente abierto | GPIO33 y GND; pull-up interno activado |
| Control de buzzer pasivo | GPIO25 → 1 kΩ → base de BC337/2N2222 |
| Transistor NPN | Emisor GND, colector negativo del buzzer |
| Resistencia de base | 10 kΩ de base a GND |
| Positivo del buzzer | 3,3 V o 5 V según tensión nominal |
| Alimentación de la placa | USB 5 V |

Todos los GND unidos. Verificar B/C/E del transistor elegido. No aplicar 5 V a ningún GPIO. Este circuito usa piezo pasivo; un buzzer magnético/inductivo requiere diodo de protección. Si el pulsador tiene cuatro patas, usar los dos contactos que se conectan al presionarlo. Se incluye el diagrama en docs/.
OLED predeterminada: dirección 0x3C; modificar config.h si es 0x3D. Una SH1106 necesita otro controlador, no basta cambiar dirección.

## Reglas
- El participante escribe su nombre y pulsa Play con el aro ubicado físicamente en A, separado del cobre, y B suelto. No se instaló sensor A: el software no puede verificar esa posición.
- Cuenta regresiva 3 segundos; al terminar arranca el cronómetro. Un contacto o pulsación B durante la cuenta cancela el intento.
- Cada contacto válido descuenta una vida y produce un pitido. Mantener contacto consume una sola vida; se rearma después de 150 ms de separación.
- Tercer error: derrota. El pulsador B completa el recorrido al registrar una nueva pulsación estable (35 ms de antirrebote).
- Si el tercer error y B coinciden, se pierde. Si coincide el primer/segundo error, ese error se contabiliza y se completa.
- Tiempo máximo 10 minutos. Al alcanzarlo, la partida termina sin clasificar. Una llegada al límite o posterior no clasifica.
- Ranking TOP 30 persistente: primero menos errores y luego menor tiempo. Empates en ambos valores comparten posición. Se guardan milisegundos y se muestran centésimas.
- Estrellas al completar: 0 errores = 3; 1 error = 2; 2 errores = 1. Derrota = 0. No se guardan cancelaciones ni recorridos sin completar en el TOP 30.
- La llegada incluye el antirrebote del pulsador (35 ms) y la latencia del bucle. Resolución de almacenamiento: 1 ms; no equivale a exactitud de laboratorio.
- El cronómetro continúa aunque se cierre el navegador. Reiniciar/cortar alimentación aborta la partida activa, pero conserva ranking y credenciales guardadas.

## Pantalla, fecha, clima y asistente
Arranque con escudo oficial durante 2 s, luego créditos. En reposo alterna carita, fecha/hora y clima. En partida muestra tiempo y vidas; los errores/victoria/derrota generan expresiones locales. Pitidos de reposo cada 20 s, desactivables desde la web.

Fecha/hora: SNTP/NTP, zona `ART3` (UTC-3). Sin conexión después de sincronizar, el reloj continúa; sin alimentación pierde la hora hasta la próxima sincronización porque no se usa RTC con batería. La partida usa millis y no depende de NTP.

Clima: Open-Meteo, coordenadas de San Miguel de Tucumán, actualización cada 15 min y reintento al minuto ante fallo. Último dato en RAM; se indica sin actualizar ante desconexión/error o más de 30 min. No se inventa temperatura si no hay dato. API: https://open-meteo.com/en/docs . Atribución visible en OLED/web.

El juego y las emociones locales funcionan sin internet ni clave API. La consulta de clima y la IA se ejecutan en otra tarea, con HTTPS y validación de certificados. WiFiManager solo se procesa entre partidas para que guardar/escaneo de Wi-Fi no interrumpa un recorrido.

### IA opcional
1. Copiar `include/secrets.example.h` como `include/secrets.h`.
2. Colocar allí una clave API propia de OpenAI con crédito/cuota y permiso de acceso al modelo. No se incluye ninguna clave real.
3. Compilar y cargar. Modelo configurable en `config.h`: `gpt-5.6-terra`; petición Chat Completions, JSON, razonamiento `none`, máximo 256 tokens de salida.
4. En la web escribir un mensaje breve y pulsar Interactuar. La respuesta aparece en web y OLED. También se solicita una frase tras victoria/derrota cuando hay cuota disponible.

La API se factura según uso y se configura por separado de una suscripción de chat. Máximo 1 consulta cada 60 s y 40 por encendido; incluye peticiones fallidas. El límite se reinicia al apagar y no sustituye límites de gasto del proveedor. No se realizan consultas periódicas de IA en reposo. No se envía el nombre del participante ni el ranking automáticamente; el texto que escriba el usuario sí se envía al proveedor.

Siete emociones: happy/sad/surprised/thinking/sleepy/excited/neutral. La respuesta se valida antes de mostrarla y se convierte a ASCII para la fuente OLED. Ante error, sin internet, sin fecha TLS o sin clave, conserva las expresiones locales. La API no modifica reglas, ranking, vidas ni cronómetro. Una respuesta antigua no sobrescribe una reacción de juego más reciente. No hay entrada de voz ni reproducción de voz; el buzzer solo emite tonos.

La clave no se devuelve por la web ni se imprime en el monitor. El archivo secrets.h está excluido de git. Un binario con clave compilada puede extraerse de un equipo con acceso físico: usar credenciales limitadas y no publicar esos binarios.
Referencia: https://developers.openai.com/api/docs/models/gpt-5.6-terra

## Estructura y edición
| Archivo/carpeta | Propósito |
|---|---|
| src/main.cpp | Inicialización, entradas e integración de módulos |
| include/game.h | Máquina de estados y reglas sin dependencia de Arduino |
| src/storage.cpp | Ranking en NVS |
| src/network.cpp | WiFiManager, mDNS, NTP y clima |
| src/assistant.cpp | API IA, estados de emoción y dibujo de ojos/boca |
| src/display.cpp | Pantallas OLED |
| src/sound.cpp | Pitidos sin esperas bloqueantes |
| src/webserver.cpp | Servidor HTTP, rutas y validación |
| src/branding.cpp / include/branding.h | Bitmap oficial OLED de 1024 bytes |
| web/index.html | Estructura de la interfaz |
| web/style.css | Estilos responsive |
| web/app.js | Interacciones y actualización en vivo |
| web/logo.svg | Logo vectorial oficial para web |
| tools/embed_web.py | Convierte los cuatro archivos web a recursos C++ en cada build |
| include/web_generated.h | Resultado generado; no editarlo manualmente |
| tools/export_binary.py | Exporta binario unificado después de compilar |

Para cambiar la web, editar web/ y recompilar; el firmware incorpora los cambios automáticamente. Logo OLED ya activado mediante HAS_OFFICIAL_LOGO=true; no hay que pegar hexadecimal otra vez. Vista ampliada del bitmap en assets/.

## Si VS Code marca #pragma once en rojo
Todos los encabezados de hardware incluyen Arduino.h y las variables tienen una única definición. `#pragma once` es correcto. Ejecutar PlatformIO: Rebuild C/C++ Project Index después de instalar dependencias. La prueba real es Build: revisar su resultado final.
Si aparece `null character(s) ignored` en un archivo bajo `.platformio/packages/framework-arduinoespressif32`, el aviso se refiere al framework local, no al bitmap. Detener la compilación, renombrar únicamente esa carpeta del framework y recompilar con internet para reinstalarla. No editar manualmente core.h ni FreeRTOS.

## Comprobaciones antes de exhibir
1. Encender sin router: logo, créditos, cara y acceso por IP.
2. Intentar Play con contacto o B mantenido: rechaza. Contacto durante cuenta: cancela.
3. Mantener cobre tocado: un error; separar 150 ms y repetir hasta perder.
4. Completar y pulsar B: registrar resultado, tiempo y estrellas; reiniciar y comprobar persistencia.
5. Cerrar la web mientras juega: cronómetro sigue en OLED.
6. Probar WiFiManager con contraseña correcta/incorrecta y retorno al juego offline.
7. Verificar hora UTC-3, clima y marca de desactualizado tras desconectar router.
8. Con clave propia, probar saludo IA y límite de consultas. Sin clave, comprobar carita local.
9. Verificar que el mecanismo accione B solo al llegar; el software confía en ese pulsador.

Pruebas nativas: `c++ -std=c++11 -I include tests/game_test.cpp -o /tmp/pulsometro_test && /tmp/pulsometro_test`.

## Cambios de la versión 3.2.0
- Un único servidor HTTP en el puerto 80, compartido con WiFiManager. Rutas registradas antes de las rutas originales para garantizar su prioridad.
- DNS cautivo local en UDP 53 y redirecciones para sondas Android/Apple/Windows. La apertura automática depende del teléfono; la IP manual sigue disponible.
- AP permanente 192.168.4.1 y juego sin internet. Configurar redes no requiere acceso a internet.
- Logo PNG original suministrado, sin aproximaciones ni enlaces externos, tanto en el juego como en WiFiManager. Pie institucional en el portal mediante extensión, sin editar dependencias descargadas.
- Tema claro/oscuro con SVG sol/luna y preferencia guardada en el navegador.
- Rutas relativas válidas al abrir web/index.html con Live Server. En Live Server se ve el diseño; la API de juego requiere la ESP32.
- OLED SSD1306 configurada en 0x3C, SDA21/SCL22; se verifica ACK antes de inicializar.

## Si el portal no aparece automáticamente
1. Permanecer conectado a PULSOMETRO-ISM aunque Android indique «sin internet»: es normal para el AP local.
2. Abrir exactamente http://192.168.4.1/wifi (HTTP, sin :8080).
3. Consultar http://192.168.4.1/api/health: debe responder versión 3.2.0. Una versión anterior indica que se grabó otro binario.
4. Si Android cambia de red, desactivar temporalmente los datos móviles y una VPN, si estuvieran activos. Reiniciar con EN y revisar el monitor serie a 115200.
5. Después de guardar la red, el AP sigue disponible. También se puede acceder desde el router usando la IP que imprime el monitor o http://pulsometro.local.

## Versión 3.2.0: conexión, memoria y asistente web
### Conexión al router
El portal funciona en http://192.168.4.1/wifi. La versión anterior ya recibía el SSID pero el intento de asociación fallaba. Ahora se desconecta limpiamente la estación anterior, se evita la reconexión automática simultánea y la espera de asociación/DHCP sube de 8 a 25 segundos. Mientras se conecta, la página puede tardar en responder: esperar y, si el AP cambia de canal, volver a conectarse a PULSOMETRO-ISM.
El portal y la web muestran el código de desconexión. El monitor serie imprime [WiFi] Desconexión: motivo N. No se puede asegurar la conexión al router real sin verificar contraseña, señal y modo de seguridad.
- 201: red no encontrada; verificar SSID, cobertura y banda 2,4 GHz.
- 202: fallo de autenticación; revisar contraseña y seguridad del router.
- 204 o 15: no se completó el intercambio de claves; comprobar contraseña, cobertura y seguridad. El código por sí solo no demuestra contraseña incorrecta.
Usar una red personal compatible con el ESP32 clásico. Una red institucional con inicio de sesión empresarial requiere otra configuración. El AP y el juego local continúan disponibles aunque el router rechace la conexión.

### Memoria del ranking
Se guardan nombre, tiempo, errores e ID en NVS (memoria flash interna). Se conservan los 30 mejores recorridos terminados, no todas las partidas ni cada actualización del cronómetro. Las credenciales Wi-Fi y la preferencia de sonidos de reposo también persisten. No se guarda un historial de chat.
Los resultados anteriores se migran agregando un ID estable sin borrarlos. Los borrados se escriben en NVS y permanecen al reiniciar. En el ranking hay botón Eliminar por fila, casilla por registro y casilla de encabezado para seleccionar todos; luego pulsar Eliminar seleccionados. Requiere autenticación admin y confirmación. Durante partidas no se permite borrar. Si falla la escritura del borrado, se restaura el ranking en RAM y se informa el error.
La carga normal con PlatformIO no realiza un borrado total de flash y esta versión mantiene la tabla de particiones, pero Erase Flash sí elimina los resultados y las credenciales.

### Asistente animado y voz
La carita SVG muestra emociones y anima ojos/boca. Pulsar Activar voz y sonidos después de abrir la web. La voz y los pitidos salen del celular/PC, no del buzzer de la ESP32. SpeechSynthesis usa las voces en español del sistema; algunas voces pueden requerir internet y ciertos navegadores no incluyen síntesis. Para uso offline, instalar una voz española local y probarla en el dispositivo elegido. Los pitidos usan Web Audio.
La función de voz del navegador no utiliza una API de pago ni requiere clave. La conversación de IA remota sigue siendo opcional y requiere configurar la clave del proveedor; el juego y las reacciones locales no dependen de ella.

### Pruebas locales
`c++ -std=c++11 -I include tests/game_test.cpp -o juego_test` y ejecutar `juego_test`.
`c++ -std=c++11 -I include tests/ranking_selection_test.cpp -o seleccion_test` y ejecutar `seleccion_test`.
