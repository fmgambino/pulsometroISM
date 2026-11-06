# Validación de la versión 3.0.1

## Resultado
**Compilación completa PlatformIO: SUCCESS.** Se generaron aplicación y binario unificado para ESP32 clásico con flash de 4 MB.

- Plataforma: espressif32 6.9.0.
- Framework: Arduino-ESP32 2.0.17 (paquete 3.20017.241212+sha.dcc1105b).
- Placa: esp32dev, ESP32 240 MHz, flash 4 MB, sin PSRAM requerida.
- RAM estática: 52.892 / 327.680 bytes (16,1 %). El uso dinámico de Wi-Fi, TLS, JSON y tareas se suma en ejecución.
- Flash de aplicación: 1.196.721 / 3.145.728 bytes (38,0 %).
- Registro de compilación final: compilacion.txt.

## Verificaciones completadas
- Pruebas C++ nativas: inicio bloqueado por contacto/B/otra partida, cuenta regresiva, victoria, tercer error simultáneo a llegada, contacto continuo, rearme de 150 ms, rebotes, cancelación, reinicio de partida, límite de 10 minutos, llegada al límite y desbordamiento de millis.
- Comparación del ranking: prioridad de errores, desempate por tiempo y empate exacto.
- JavaScript: sintaxis válida con node --check. IDs DOM únicos y referencias a elementos existentes.
- Scripts Python de empaquetado: sintaxis válida.
- Recursos web generados a partir de HTML, CSS, JS y SVG; bitmap OLED de exactamente 1024 bytes.
- Fuentes del proyecto sin caracteres NUL.
- SVG parseable como XML y logo OLED integrado/activado.
- Binario unificado comprobado byte a byte: bootloader en 0x1000, particiones en 0x8000, boot_app0 en 0xe000 y aplicación en 0x10000. El archivo unificado completo se graba en 0x0000.
- Partición app0 verificada: offset 0x10000, tamaño 0x300000. Se incluye manifest.json con hashes SHA-256 de los binarios.

## Límites de esta validación
No se conectó una placa física. Quedan por verificar en banco el cableado, dirección/controlador de la OLED, timbre del buzzer, pulsador, mDNS y WiFiManager con el router real, persistencia en la flash del equipo y acceso meteorológico/NTP real.

La integración de IA compila, pero no se hizo una llamada autenticada porque no se proporcionó una clave API. El binario incluido no contiene una clave real; funciona con carita local. Para activar la API, configurar secrets.h y compilar de nuevo.

El archivo adjunto de entrada contenía la plantilla de PlatformIO y platformio.ini, sin fuentes. Esta entrega reconstruye y reorganiza la implementación acordada en la conversación e integra los logos suministrados.

## Actualización 3.0.1 — acceso al portal
Compilación SUCCESS y reglas nativas aprobadas. Se deshabilitaron las redirecciones cautivas y el cierre automático por tiempo, se fijó IP del AP y se desactivó el ahorro Wi-Fi. Se evita WiFi.begin/reconnect sin SSID guardado. Se agregaron /api/health, registros [RED] y un indicador [VIVO] periódico.
La causa exacta de ERR_TOO_MANY_RETRIES en el dispositivo del usuario sigue sin confirmarse. No se probó acceso por radio en hardware físico; no se garantiza que estas medidas por sí solas resuelvan ese fallo. Usar las nuevas señales de diagnóstico para aislarlo.
