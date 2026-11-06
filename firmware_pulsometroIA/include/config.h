#pragma once
// ESP32 clasico WROOM-32, OLED SSD1306 I2C 128x64.
constexpr int PIN_TOUCH=27, PIN_FINISH=33, PIN_BUZZER=25;
constexpr int OLED_SDA=21, OLED_SCL=22, OLED_ADDR=0x3C;
constexpr char AP_SSID[]="PULSOMETRO-ISM";
constexpr char AP_PASSWORD[]="SanMiguel2026"; // Cambiar antes de una exposicion.
constexpr char HOSTNAME[]="pulsometro";
// Opcionales: tambien se pueden cargar desde la web protegida.
constexpr char WIFI_SSID[]="";
constexpr char WIFI_PASSWORD[]="";
constexpr char ADMIN_PASSWORD[]="ClubISM2026"; // Cambiar antes de usar.
// IA opcional: clave solo en firmware, nunca enviada al navegador.
constexpr char OPENAI_API_KEY[]="";
constexpr char OPENAI_MODEL[]="gpt-4.1-mini";
constexpr uint32_t AI_COOLDOWN_MS=60000;
constexpr unsigned AI_MAX_REQUESTS_PER_BOOT=40;
constexpr char LOCAL_TZ[]="ART3"; // Argentina UTC-3
