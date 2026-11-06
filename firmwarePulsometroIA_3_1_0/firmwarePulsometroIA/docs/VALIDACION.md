# Validación de PULSÓMETRO 3.1.0

Compilación PlatformIO completa: SUCCESS. espressif32 6.9.0, Arduino-ESP32 2.0.17, placa esp32dev. RAM estática 52.708 / 327.680 bytes (16,1 %). Aplicación 1.230.909 / 3.145.728 bytes (39,1 %). Registro en compilacion.txt.

## Verificaciones realizadas
- Pruebas C++ nativas de juego y ranking aprobadas: cuenta regresiva, contactos, vidas, llegada, prioridad del tercer error, cancelación, límite de tiempo, desbordamiento de millis y orden del ranking.
- JavaScript de juego, tema y portal: sintaxis comprobada con Node.
- Tema ejecutado con DOM simulado: alternancia claro/oscuro, iconos SVG, etiquetas accesibles y persistencia mediante localStorage.
- IDs HTML únicos, referencias DOM existentes y archivos estáticos locales presentes. Se eliminaron etiquetas script anidadas y enlaces CSS duplicados del adjunto.
- PNG utilizado idéntico al logo original adjunto. SVG XML válido y bitmap OLED de 1024 bytes. Fuentes sin caracteres NUL.
- Revisión de WiFiManager 2.0.17 y WebServer Arduino 2.0.17: registro por callback antes de rutas originales, servidor único HTTP 80, guardas de partida y DNS UDP 53. No se modifica la dependencia instalada.
- Binario unificado verificado byte a byte: bootloader 0x1000, particiones 0x8000, boot_app0 0xe000, aplicación 0x10000. Imagen completa en 0x0000. Hashes SHA-256 en release/manifest.json.

## Pendiente de verificación física
No se dispone de tu placa, teléfono ni router. La apertura automática del portal, asociación a redes, mDNS, OLED, buzzer y contactos deben comprobarse en banco. 0x3C es la dirección detectada en tu captura; no identifica por sí sola el controlador. Esta entrega usa SSD1306 128×64.
No se ejecutaron consultas autenticadas de IA ni pruebas reales de clima/NTP.
Se intentó una comprobación visual automatizada, pero el navegador del entorno no pudo iniciarse. No se afirma validación visual de navegador; las pruebas de web descritas arriba son estáticas y con DOM simulado.

## Prueba rápida en tu equipo
1. Grabar el binario nuevo y pulsar EN. Verificar versión 3.1.0 en /api/health.
2. Conectarse a PULSOMETRO-ISM. Abrir http://192.168.4.1/wifi sin puerto adicional.
3. Confirmar logo/footer, seleccionar Wi-Fi 2,4 GHz y guardar. Revisar IP asignada en el monitor serie.
4. Abrir /game, alternar tema, recargar y comprobar que se conserva.
5. Probar contacto continuo (una sola vida), tres contactos separados (derrota) y llegada con pulsador B.
6. Apagar el router y comprobar que el juego sigue accesible por el AP.
