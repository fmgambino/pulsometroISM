#include "app.h"
#include "branding.h"
// Renderizado de OLED. Nunca se consulta internet desde este módulo.
// Dibujar iconos meteorológicos directamente con primitivas monocromas.
void weatherDrawing(Weather w) {
    int x = 100, y = 30;
    if (w.code == 0) {
        if (w.day) {
            oled.drawCircle(x, y, 8, WHITE);
            for (int i = 0; i < 8; i++) {
                float a = i * PI / 4;
                oled.drawLine(x + 11 * cos(a), y + 11 * sin(a), x + 15 * cos(a), y + 15 * sin(a),
                              WHITE);
            }
        } else {
            oled.fillCircle(x, y, 11, WHITE);
            oled.fillCircle(x + 5, y - 4, 9, BLACK);
        }
    } else {
        oled.fillCircle(x - 8, y, 7, WHITE);
        oled.fillCircle(x, y - 4, 10, WHITE);
        oled.fillCircle(x + 10, y, 7, WHITE);
        oled.fillRect(x - 12, y, 26, 8, WHITE);
        if (w.code > 48) {
            for (int i = 0; i < 3; i++)
                oled.drawLine(x - 10 + i * 10, y + 12, x - 12 + i * 10, y + 17, WHITE);
        }
    }
}
// Actualizar a 10 cuadros por segundo como máximo, sin esperas largas.
void draw(uint32_t now) {
    if (!oledOK || now - lastDraw < 100)
        return;
    lastDraw = now;
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    uint32_t age = now - bootAt;
    if (age < 2000) {
        if (HAS_OFFICIAL_LOGO)
            oled.drawBitmap(0, 0, OFFICIAL_LOGO, 128, 64, WHITE);
        else {
            oled.setTextSize(3);
            oled.setCursor(36, 8);
            oled.print("ISM");
            oled.setTextSize(1);
            oled.setCursor(13, 43);
            oled.print("Instituto San Miguel");
        }
    } else if (age < 4500) {
        oled.setCursor(0, 10);
        oled.println("Diseno y Desarrollo");
        oled.println();
        oled.println("Club de Robotica e IA");
        oled.println("Instituto San Miguel");
    } else if (faceUntil && int32_t(faceUntil - now) > 0) {
        auto face = assistantSnapshot();
        oled.println(String(face.text).substring(0, 21));
        drawFace(oled, now, face.emotion);
        oled.setCursor(0, 56);
        oled.print(clockText(game.elapsed));
        oled.printf(" V:%u", 3 - game.errors);
    } else if (game.busy() || (game.phase != Phase::Idle && now - resultAt < 10000)) {
        oled.println(oledSafe(player).substring(0, 20));
        oled.println(phaseLabel());
        oled.setTextSize(2);
        oled.setCursor(0, 23);
        if (game.phase == Phase::Countdown)
            oled.print(3 - (now - game.mark) / 1000);
        else
            oled.print(clockText(game.elapsed));
        oled.setTextSize(1);
        oled.setCursor(0, 53);
        oled.printf("Vidas:%u  Errores:%u", 3 - game.errors, game.errors);
    } else if ((now / 6000) % 3 == 0 || (faceUntil && int32_t(faceUntil - now) > 0)) {
        AssistantState face = assistantSnapshot();
        oled.print("PULSOMETRO ");
        oled.println(face.online ? "IA" : "local");
        drawFace(oled, now, face.emotion);
        String text = face.text;
        size_t offset = text.length() > 21 ? (now / 350) % (text.length() + 5) : 0;
        String scroll = text + "     " + text;
        oled.setCursor(0, 56);
        oled.print(scroll.substring(offset, offset + 21));
    } else if ((now / 6000) % 3 == 1) {
        time_t epoch = time(nullptr);
        oled.println("FECHA Y HORA");
        if (epoch > 1700000000) {
            struct tm t;
            localtime_r(&epoch, &t);
            char text[24];
            strftime(text, sizeof(text), "%d/%m/%Y", &t);
            oled.setCursor(0, 17);
            oled.print(text);
            strftime(text, sizeof(text), "%H:%M:%S", &t);
            oled.setTextSize(2);
            oled.setCursor(10, 32);
            oled.print(text);
            oled.setTextSize(1);
            oled.setCursor(0, 56);
            oled.print("Tucuman UTC-3");
        } else {
            oled.setCursor(0, 24);
            oled.println("Sin sincronizar");
            oled.println("Conecta WiFi / NTP");
        }
    }

    else {
        Weather w = getWeather();
        oled.println("S. M. de Tucuman");
        if (w.valid) {
            oled.setTextSize(2);
            oled.setCursor(0, 22);
            oled.printf("%.1f C", w.temp);
            weatherDrawing(w);
            oled.setTextSize(1);
            oled.setCursor(0, 53);
            oled.print(w.failed || WiFi.status() != WL_CONNECTED || now - w.fetched > 1800000
                           ? "Sin actualizar"
                           : "Clima: Open-Meteo");
        } else {
            oled.setCursor(0, 25);
            oled.println("Clima no disponible");
            oled.println("Juego listo offline");
        }
    }
    oled.display();
}
