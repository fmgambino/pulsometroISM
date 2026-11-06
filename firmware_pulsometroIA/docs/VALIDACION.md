# Validación de entrega 2.0

- Pruebas nativas C++ de máquina de estados: aprobadas (inicio, cuenta regresiva, contacto continuo, rearme, tercer error simultáneo a llegada, victoria, cancelación, contacto breve, límite de tiempo y rollover de millis).
- JavaScript: node --check aprobado. IDs DOM sin duplicados y referencias de elementos válidas.
- Revisión de integración: WiFiManager en 8080, web del juego en 80; portal se detiene al iniciar partida y no se procesa mientras se juega. IA y clima en una tarea de red independiente.
- Compilación PlatformIO: no completada. Se instaló la plataforma y finalmente el compilador Xtensa tras reintentos por checksum. La instalación del framework Arduino quedó pendiente y se detuvo la descarga; no se llegó a compilar/enlazar esta versión. No se entrega un .bin verificado.
- No se hicieron pruebas sobre una placa ESP32, OLED, WiFiManager con un router real, mDNS ni conexión meteorológica.
- API IA: integración implementada, sin prueba autenticada porque no se suministró una clave API. La clave entregada está vacía; se activa al configurarla en include/config.h.
- Logo oficial pendiente. Los recursos actuales son una identificación tipográfica ISM provisional.

Antes de exhibir el juego, compilar/cargar desde PlatformIO y completar las pruebas de banco del README, incluidas WiFiManager, NTP y fallback IA.
