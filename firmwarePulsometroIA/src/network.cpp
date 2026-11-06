#include "app.h"
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "weather_ca.h"
// Único propietario de la caché meteorológica; copiar bajo bloqueo entre núcleos.
static Weather weather;
static portMUX_TYPE weatherMux = portMUX_INITIALIZER_UNLOCKED;
Weather getWeather() {
    Weather w;
    portENTER_CRITICAL(&weatherMux);
    w = weather;
    portEXIT_CRITICAL(&weatherMux);
    return w;
}
// TLS con CA y fecha NTP. Tarea separada: HTTP nunca detiene el juego.
void weatherTask(void *) {
    uint32_t attempted = 0;
    bool tried = false;
    for (;;) {
        uint32_t now = millis();
        if (WiFi.status() == WL_CONNECTED && (!tried || now - attempted >= 900000)) {
            tried = true;
            attempted = now;
            Weather w = getWeather();
            bool ok = false;
            if (time(nullptr) > 1700000000) {
                WiFiClientSecure client;
                client.setCACert(WEATHER_CA);
                client.setHandshakeTimeout(5);
                HTTPClient http;
                http.setConnectTimeout(4000);
                http.setTimeout(4000);
                if (http.begin(client,
                               "https://api.open-meteo.com/v1/"
                               "forecast?latitude=-26.8241&longitude=-65.2226&current=temperature_"
                               "2m,weather_code,is_day&timezone=America%2FArgentina%2FTucuman")) {
                    int code = http.GET();
                    if (code == 200) {
                        DynamicJsonDocument d(4096);
                        if (!deserializeJson(d, http.getString()) &&
                            d["current"]["temperature_2m"].is<float>() &&
                            d["current"]["weather_code"].is<int>()) {
                            w.temp = d["current"]["temperature_2m"];
                            w.code = d["current"]["weather_code"];
                            w.day = d["current"]["is_day"].as<int>() != 0;
                            w.valid = true;
                            w.fetched = millis();
                            ok = true;
                        }
                    }
                    http.end();
                }
            }
            w.failed = !ok;
            portENTER_CRITICAL(&weatherMux);
            weather = w;
            portEXIT_CRITICAL(&weatherMux);
            if (!ok)
                attempted = millis() - 840000;
        }
        assistantNetworkStep();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Punto de acceso permanente + estación: se puede jugar sin router.
// WiFiManager usa 8080; la web del juego conserva el puerto HTTP 80.
void beginNetwork() {
    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_AP_STA);
    // IP explícita y ahorro de energía desactivado para la conexión local.
    IPAddress apIP(192, 168, 4, 1), mask(255, 255, 255, 0);
    if (!WiFi.softAPConfig(apIP, apIP, mask))
        Serial.println("[RED] Error al fijar IP del AP");
    bool apOK = WiFi.softAP(AP_SSID, AP_PASSWORD);
    WiFi.setSleep(false);
    Serial.printf("[RED] AP=%s IP=%s inicio=%s\n", AP_SSID, WiFi.softAPIP().toString().c_str(),
                  apOK ? "OK" : "ERROR");
    WiFi.setAutoReconnect(true);
    // No intentar una conexión de estación si aún no hay SSID guardado.
    if (WiFi.SSID().length() > 0)
        WiFi.begin();
    wifiManager.setDebugOutput(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setHttpPort(8080);
    // Se accede por IP directa: no se necesitan redirecciones cautivas.
    wifiManager.setCaptivePortalEnable(false);
    std::vector<const char *> menu = {"wifi", "info", "exit"};
    wifiManager.setMenu(menu);
    wifiManager.setConnectTimeout(8);
    wifiManager.setSaveConnectTimeout(8);
    wifiManager.startWebPortal();
    wifiPortalOpen = wifiManager.getWebPortalActive();
    wifiPortalAt = millis();
    Serial.printf("[RED] WiFiManager=%s http://%s:8080/wifi\n", wifiPortalOpen ? "ACTIVO" : "ERROR",
                  WiFi.softAPIP().toString().c_str());
    if (MDNS.begin(HOSTNAME))
        MDNS.addService("http", "tcp", 80);
    configTzTime(LOCAL_TZ, "pool.ntp.org", "time.google.com");
    if (strlen(OPENAI_API_KEY))
        assistantSet(Emotion::Happy, "Hola! Jugamos?", "IA configurada: esperando interaccion");
    networkTaskAvailable =
        xTaskCreatePinnedToCore(weatherTask, "red", 12288, nullptr, 1, nullptr, 0) == pdPASS;
    if (!networkTaskAvailable)
        Serial.println("Sin tarea de red; juego local disponible");
}
void processNetwork(bool busy) {
    uint32_t now = millis();
    // Escanear/guardar Wi-Fi puede demorar: se permite solo entre partidas.
    if (wifiPortalOpen && !busy) {
        wifiManager.process();
        wifiPortalOpen = wifiManager.getWebPortalActive();
        if (WIFI_PORTAL_TIMEOUT_MS > 0 && wifiPortalOpen &&
            now - wifiPortalAt >= WIFI_PORTAL_TIMEOUT_MS) {
            wifiManager.stopWebPortal();
            wifiPortalOpen = false;
        }
    }
    if (!busy && WiFi.SSID().length() > 0 && WiFi.status() != WL_CONNECTED &&
        now - lastWifiTry > 30000) {
        lastWifiTry = now;
        WiFi.reconnect();
    }
}
