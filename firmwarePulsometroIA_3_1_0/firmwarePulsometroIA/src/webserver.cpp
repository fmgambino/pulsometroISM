#include "app.h"
#include "web_generated.h"
// El servidor local entrega recursos estáticos y el estado calculado en la placa.
// Proteger acciones del organizador con autenticación local.
bool admin() {
    if (http().authenticate("admin", ADMIN_PASSWORD))
        return true;
    http().requestAuthentication();
    return false;
}
// Responder un error legible para la interfaz web.
void fail(int code, const char *s) {
    http().send(code, "text/plain; charset=utf-8", s);
}
// Traducir los códigos meteorológicos WMO a iconos para la web.
const char *weatherIcon(const Weather &w) {
    if (w.code == 0)
        return w.day ? "☀️" : "🌙";
    if (w.code <= 3)
        return "☁️";
    if (w.code <= 48)
        return "🌫️";
    if (w.code >= 95)
        return "⛈️";
    if (w.code >= 71 && w.code <= 77)
        return "❄️";
    if (w.code == 85 || w.code == 86)
        return "❄️";
    return "🌧️";
}
// Entregar una instantánea de estado; nunca aceptar tiempo o errores del navegador.
void state() {
    DynamicJsonDocument d(12288);
    uint32_t now = millis();
    d["version"] = FIRMWARE_VERSION;
    d["phase"] = phaseName();
    d["name"] = player;
    d["errors"] = game.errors;
    d["ms"] = game.elapsed;
    d["countdown"] = game.phase == Phase::Countdown
                         ? 3 - min(2UL, (unsigned long)((now - game.mark) / 1000))
                         : 0;
    bool touch = digitalRead(PIN_TOUCH) == LOW, finish = digitalRead(PIN_FINISH) == LOW;
    d["touch"] = touch;
    d["finish"] = finish;
    d["ready"] = now - bootAt >= BOOT_PRESENTATION_MS && !game.busy() && !touch && !finish;
    d["internet"] = WiFi.status() == WL_CONNECTED;
    d["ip"] = WiFi.localIP().toString();
    d["idleSound"] = idleSound;
    d["wifiPortal"] = wifiPortalOpen;
    time_t epoch = time(nullptr);
    bool validTime = epoch > 1700000000;
    d["timeValid"] = validTime;
    char dateText[40] = "Fecha y hora sin sincronizar";
    if (validTime) {
        struct tm t;
        localtime_r(&epoch, &t);
        strftime(dateText, sizeof(dateText), "%d/%m/%Y %H:%M:%S", &t);
    }
    d["datetime"] = dateText;
    AssistantState face = assistantSnapshot();
    d["emotion"] = emotionName(face.emotion);
    d["assistantText"] = face.text;
    d["assistantStatus"] = face.status;
    d["aiConfigured"] = strlen(OPENAI_API_KEY) > 0;
    d["aiCalls"] = assistantCalls();
    auto a = d.createNestedArray("ranking");
    int position = 1;
    for (int i = 0; i < rankCount; i++) {
        if (i && (ranking[i].errors != ranking[i - 1].errors || ranking[i].ms != ranking[i - 1].ms))
            position = i + 1;
        auto o = a.createNestedObject();
        o["position"] = position;
        o["name"] = ranking[i].name;
        o["ms"] = ranking[i].ms;
        o["errors"] = ranking[i].errors;
    }
    Weather w = getWeather();
    auto o = d.createNestedObject("weather");
    o["valid"] = w.valid;
    o["temp"] = w.temp;
    o["icon"] = weatherIcon(w);
    o["age"] = w.valid ? (uint32_t)(now - w.fetched) / 1000 : 0;
    o["stale"] =
        w.failed || WiFi.status() != WL_CONNECTED || (w.valid && now - w.fetched > 1800000);
    String s;
    serializeJson(d, s);
    http().sendHeader("Cache-Control", "no-store");
    http().send(200, "application/json", s);
}
// Registrar recursos locales y acciones. No hay dependencias de CDN.
void routes() {
    // En una red sin configurar, la raíz muestra WiFiManager. /game siempre abre el juego.
    http().on("/", HTTP_GET, [] {
        if (wifiPortalOpen && !game.busy())
            wifiManager.renderHome();
        else
            http().send_P(200, "text/html; charset=utf-8", PAGE);
    });
    http().on("/game", HTTP_GET, [] {
        wifiPortalOpen = false;
        http().send_P(200, "text/html; charset=utf-8", PAGE);
    });
    http().on("/wifi", HTTP_GET, [] {
        if (game.busy()) {
            fail(409, "Finalizá la partida para configurar Wi-Fi");
            return;
        }
        wifiPortalOpen = true;
        wifiManager.renderNetworks(true);
    });
    http().on("/0wifi", HTTP_GET, [] {
        if (game.busy()) {
            fail(409, "Partida activa");
            return;
        }
        wifiPortalOpen = true;
        wifiManager.renderNetworks(false);
    });
    http().on("/wifisave", HTTP_ANY, [] {
        if (game.busy()) {
            fail(409, "Partida activa");
            return;
        }
        wifiManager.saveNetwork();
    });
    http().on("/theme.js", HTTP_GET,
              [] { http().send_P(200, "application/javascript; charset=utf-8", THEME); });
    http().on("/portal.js", HTTP_GET,
              [] { http().send_P(200, "application/javascript; charset=utf-8", PORTAL_SCRIPT); });
    http().on("/portal.css", HTTP_GET,
              [] { http().send_P(200, "text/css; charset=utf-8", PORTAL_STYLE); });
    http().on("/logo.png", HTTP_GET, [] {
        http().send_P(200, "image/png", reinterpret_cast<const char *>(LOGO_PNG), sizeof(LOGO_PNG));
    });
    http().on("/api/health", HTTP_GET, [] {
        http().sendHeader("Cache-Control", "no-store");
        http().send(200, "text/plain; charset=utf-8",
                    String("PULSOMETRO ") + FIRMWARE_VERSION +
                        " OK\nAP: " + WiFi.softAPIP().toString() +
                        "\nHTTP unificado: 80\nOLED I2C: " + String(OLED_ADDR, HEX));
        Serial.println("[HTTP] /api/health recibida");
    });
    auto redirect = [] {
        http().sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
        http().send(302, "text/plain", "");
    };
    for (const char *path :
         {"/generate_204", "/gen_204", "/hotspot-detect.html", "/connecttest.txt", "/ncsi.txt",
          "/redirect", "/fwlink", "/canonical.html"})
        http().on(path, HTTP_GET, redirect);
    http().on("/exit", HTTP_ANY, [] {
        wifiPortalOpen = false;
        http().sendHeader("Location", "/game", true);
        http().send(302, "text/plain", "");
    });
    // Este proyecto no ofrece OTA ni borrado global desde el portal público.
    for (const char *path : {"/erase", "/reset", "/restart", "/close", "/update", "/updatedone"})
        http().on(path, HTTP_ANY, [] { fail(403, "Acción no disponible en el portal del juego"); });
    http().on("/style.css", HTTP_GET, [] { http().send_P(200, "text/css; charset=utf-8", STYLE); });
    http().on("/app.js", HTTP_GET,
              [] { http().send_P(200, "application/javascript; charset=utf-8", SCRIPT); });
    http().on("/logo.svg", HTTP_GET, [] { http().send_P(200, "image/svg+xml", WEB_LOGO); });
    http().on("/api/state", HTTP_GET, state);
    http().on("/api/play", HTTP_POST, [] {
        String n = http().arg("name");
        n.trim();
        if (n.length() == 0 || n.length() > 64) {
            fail(400, "Ingresá un nombre de hasta 32 caracteres (64 bytes).");
            return;
        }
        for (size_t i = 0; i < n.length(); i++)
            if ((uint8_t)n[i] < 32) {
                fail(400, "Nombre inválido");
                return;
            }
        if (millis() - bootAt < BOOT_PRESENTATION_MS) {
            fail(409, "Esperá la presentación inicial");
            return;
        }
        // Cerrar el portal antes de iniciar los 3 segundos; su cierre no consume tiempo de juego.
        if (!game.busy())
            wifiPortalOpen = false;
        clearContactLatch();
        if (!game.start(millis(), digitalRead(PIN_TOUCH) == LOW, digitalRead(PIN_FINISH) == LOW)) {
            fail(409, "Partida activa, aro tocando cobre o pulsador B presionado");
            return;
        }
        player = n;
        assistantSet(Emotion::Thinking, "Concentracion!", "Reaccion local");
        beep(1000, 100);
        http().send(200, "text/plain", "OK");
    });
    http().on("/api/cancel", HTTP_POST, [] {
        game.cancel(millis());
        resultAt = millis();
        assistantSet(Emotion::Neutral, "Partida cancelada", "Reaccion local");
        http().send(200, "text/plain", "OK");
    });
    http().on("/admin/wifi", HTTP_POST, [] {
        if (!admin())
            return;
        if (game.busy()) {
            fail(409, "Esperá a finalizar la partida");
            return;
        }
        if (!wifiPortalOpen) {
            wifiPortalOpen = true;
            wifiPortalAt = millis();
        }
        http().send(200, "text/plain", "Portal WiFiManager disponible en /wifi");
    });
    http().on("/admin/wifi-close", HTTP_POST, [] {
        if (!admin())
            return;
        wifiPortalOpen = false;
        http().send(200, "text/plain", "OK");
    });
    http().on("/api/interact", HTTP_POST, [] {
        if (game.busy()) {
            fail(409, "Interactuá con la carita entre partidas");
            return;
        }
        String msg = http().arg("text");
        msg.trim();
        if (msg.length() == 0 || msg.length() > 160) {
            fail(400, "Escribí hasta 160 bytes");
            return;
        }
        assistantSet(Emotion::Happy, "Hola! Tu puedes!", "Reaccion local");
        if (!strlen(OPENAI_API_KEY) || WiFi.status() != WL_CONNECTED) {
            http().send(200, "text/plain",
                        "Carita local activa: IA no disponible sin clave o conexión");
            return;
        }
        if (!queueAI(msg)) {
            fail(429, "Esperá 60 segundos entre consultas; límite 40 por encendido");
            return;
        }
        http().send(200, "text/plain", "Consulta enviada; la respuesta aparecerá en la carita");
    });
    http().on("/admin/sound", HTTP_POST, [] {
        if (!admin())
            return;
        idleSound = !idleSound;
        prefs.putBool("sound", idleSound);
        http().send(200, "text/plain", "OK");
    });
    http().on("/admin/clear", HTTP_POST, [] {
        if (!admin())
            return;
        if (game.busy()) {
            fail(409, "Partida activa");
            return;
        }
        rankCount = 0;
        saveRanks();
        http().send(200, "text/plain", "OK");
    });
    http().onNotFound([] {
        // Las sondas del sistema y dominios del DNS cautivo llegan al portal local.
        if (wifiPortalOpen && !game.busy()) {
            http().sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/",
                              true);
            http().send(302, "text/plain", "");
        } else
            fail(404, "No encontrado");
    });
}
