#pragma once
#include <Arduino.h>
constexpr char FIRMWARE_VERSION[] = "3.0.1";
// Configuración única de hardware. Los números son GPIO, no posiciones de patas.
constexpr int PIN_TOUCH = 27;  // Aro a GND, cobre a GPIO27 mediante 1 kΩ.
constexpr int PIN_FINISH = 33; // Pulsador B normalmente abierto hacia GND.
constexpr int PIN_BUZZER = 25; // Buzzer piezo pasivo mediante transistor NPN.
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr char AP_SSID[] = "PULSOMETRO-ISM";
constexpr char AP_PASSWORD[] = "SanMiguel2026"; // Cambiar para la exposición.
constexpr char HOSTNAME[] = "pulsometro";       // mDNS no utiliza tilde.
constexpr char ADMIN_PASSWORD[] = "ClubISM2026";
constexpr char LOCAL_TZ[] = "ART3"; // Tucumán: UTC-3.
constexpr uint32_t BOOT_PRESENTATION_MS = 4500;
constexpr uint32_t WIFI_PORTAL_TIMEOUT_MS = 0;
// Secreto opcional. Crear include/secrets.h a partir de secrets.example.h.
#if __has_include("secrets.h")
#include "secrets.h"
#else
constexpr char OPENAI_API_KEY[] = "";
#endif
constexpr char OPENAI_MODEL[] = "gpt-5.6-terra";
constexpr uint32_t AI_COOLDOWN_MS = 60000;
constexpr unsigned AI_MAX_REQUESTS_PER_BOOT = 40;
