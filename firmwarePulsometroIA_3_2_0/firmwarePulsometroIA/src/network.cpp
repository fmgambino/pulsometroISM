#include "app.h"
#include <ESPmDNS.h>
#include <DNSServer.h>
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

// Un solo servidor HTTP (propiedad de WiFiManager) evita puertos en conflicto.
// El DNS local dirige las solicitudes cautivas al AP, incluso sin internet.
static DNSServer captiveDNS;
// El evento corre en otra tarea: compartir solo el código numérico, sin credenciales.
static volatile uint16_t lastDisconnectReason = 0;
uint16_t wifiFailureReason() {
    return lastDisconnectReason;
}
WebServer &http() {
    return *httpServer;
}
static const char portalHead[] = "<link rel=\"stylesheet\" href=\"/portal.css\">"
                                 "<script src=\"/theme.js\"></script>"
                                 "<script defer src=\"/portal.js\"></script>";
void beginNetwork() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.setSleep(false);
    IPAddress ip(192, 168, 4, 1), mask(255, 255, 255, 0);
    if (!WiFi.softAPConfig(ip, ip, mask) || !WiFi.softAP(AP_SSID, AP_PASSWORD))
        Serial.println("ERROR al iniciar AP");
    // Evitar que la reconexión automática compita con el cambio de red del portal.
    WiFi.setAutoReconnect(false);
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            if (info.wifi_sta_disconnected.reason != 8)
                lastDisconnectReason = info.wifi_sta_disconnected.reason;
            Serial.printf("[WiFi] Desconexión: motivo %u (202/204: autenticación; 201: red no "
                          "encontrada; 15: handshake)\n",
                          lastDisconnectReason);
        } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP)
            lastDisconnectReason = 0;
    });
    if (WiFi.SSID().length())
        WiFi.begin();
    wifiManager.setDebugOutput(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setHttpPort(80);
    wifiManager.setConfigPortalTimeout(0);
    wifiManager.setCaptivePortalEnable(false); // DNS y sondas atendidos por nuestra aplicación.
    wifiManager.setCustomHeadElement(portalHead);
    std::vector<const char *> menu = {"wifi", "info"};
    wifiManager.setMenu(menu);
    wifiManager.setCleanConnect(true); // Desconectar estación anterior antes de asociar.
    wifiManager.setConnectTimeout(25);
    wifiManager.setSaveConnectTimeout(25);
    // La biblioteca llama este punto antes de registrar sus rutas: las nuestras
    // tienen prioridad, sin sustituir ni reiniciar el servidor.
    wifiManager.setWebServerCallback([] {
        httpServer = wifiManager.server.get();
        routes();
    });
    wifiManager.setSaveConfigCallback([] { lastWifiTry = millis(); });
    wifiManager.startWebPortal();
    httpServer = wifiManager.server.get();
    wifiPortalOpen = WiFi.status() != WL_CONNECTED;
    captiveDNS.setErrorReplyCode(DNSReplyCode::NoError);
    if (!captiveDNS.start(53, "*", ip))
        Serial.println("ERROR al iniciar DNS cautivo");
    if (MDNS.begin(HOSTNAME))
        MDNS.addService("http", "tcp", 80);
    configTzTime(LOCAL_TZ, "pool.ntp.org", "time.google.com");
    networkTaskAvailable =
        xTaskCreatePinnedToCore(weatherTask, "red", 12288, nullptr, 1, nullptr, 0) == pdPASS;
    Serial.println("Portal WiFi: http://192.168.4.1/ | Juego: http://192.168.4.1/game");
}
void processNetwork(bool busy) {
    captiveDNS.processNextRequest();
    // Durante una partida solo atender HTTP: no escanear ni conectar redes.
    if (busy)
        http().handleClient();
    else
        wifiManager.process();
    static bool connectedBefore = false;
    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected && !connectedBefore) {
        wifiPortalOpen = false;
        Serial.printf("WiFi conectado. IP router: %s\n", WiFi.localIP().toString().c_str());
    }
    connectedBefore = connected;
    if (!busy && !connected && WiFi.SSID().length() && !wifiPortalOpen &&
        millis() - lastWifiTry > 30000) {
        lastWifiTry = millis();
        WiFi.reconnect();
    }
}
