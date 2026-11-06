#include "app.h"
#include <Wire.h>

// Objetos compartidos por los módulos. Se definen una sola vez aquí.
WebServer server(80);
WiFiManager wifiManager;
Preferences prefs;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
Game game;
String player;
Score ranking[30];
int rankCount = 0;
bool oledOK = false, idleSound = true, wifiPortalOpen = false;
uint32_t bootAt = 0, lastDraw = 0, lastChirp = 0, toneEnd = 0;
uint32_t lastWifiTry = 0, wifiPortalAt = 0, faceUntil = 0, resultAt = 0;

// La interrupción conserva contactos breves entre dos vueltas de loop().
// Nunca se dibuja, escribe flash ni emiten sonidos desde la interrupción.
static volatile bool contactIRQ = false;
static portMUX_TYPE irqMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR contactISR() {
    portENTER_CRITICAL_ISR(&irqMux);
    contactIRQ = true;
    portEXIT_CRITICAL_ISR(&irqMux);
}
void clearContactLatch() {
    portENTER_CRITICAL(&irqMux);
    contactIRQ = false;
    portEXIT_CRITICAL(&irqMux);
}

// Antirrebote del pulsador de llegada: se detecta una nueva pulsación estable.
struct FinishButton {
    bool raw = false, stable = false;
    uint32_t changed = 0;
    bool update(uint32_t now) {
        bool value = digitalRead(PIN_FINISH) == LOW;
        if (value != raw) {
            raw = value;
            changed = now;
        }
        if (raw != stable && now - changed >= 35) {
            stable = raw;
            return stable;
        }
        return false;
    }
} finishButton;

void setup() {
    Serial.begin(115200);
    bootAt = millis();
    pinMode(PIN_TOUCH, INPUT_PULLUP);
    pinMode(PIN_FINISH, INPUT_PULLUP);
    finishButton.raw = finishButton.stable = digitalRead(PIN_FINISH) == LOW;
    attachInterrupt(digitalPinToInterrupt(PIN_TOUCH), contactISR, FALLING);
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_BUZZER, 0);
    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setTimeOut(25);
    oledOK = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    // Si no hay pantalla, el juego y la web siguen disponibles.
    if (!oledOK)
        Serial.println("OLED no detectada; revisar dirección y conexiones");
    lastDraw = bootAt - 100;
    draw(bootAt); // Mostrar el escudo antes de inicializar servicios de red.
    if (!prefs.begin("pulsometro", false))
        Serial.println("No se pudo abrir NVS; revisar almacenamiento");
    idleSound = prefs.getBool("sound", true);
    loadRanks();
    beginAssistant();
    beginNetwork();
    routes();
    bootAt = millis(); // Dar tiempo completo al logo y los créditos tras inicializar la red.
    lastDraw = bootAt - 100;
    Serial.println("PULSOMETRO: http://pulsometro.local / http://192.168.4.1");
}

void loop() {
    uint32_t now = millis();
    bool finishEdge = finishButton.update(now), contact;
    portENTER_CRITICAL(&irqMux);
    contact = contactIRQ;
    contactIRQ = false;
    portEXIT_CRITICAL(&irqMux);

    Phase previous = game.phase;
    unsigned errors = game.errors;
    // El juego solo depende de las entradas físicas y millis(), nunca de internet.
    game.tick(now, digitalRead(PIN_TOUCH) == LOW, contact, finishEdge);
    if (game.errors != errors) {
        beep(350, 350);
        assistantSet(Emotion::Surprised, "Ups! Despacio, tu puedes.", "Reaccion local");
        faceUntil = now + 1300;
    }
    if (previous != game.phase) {
        lastChirp = now;
        if (game.phase == Phase::Running)
            beep(1800, 180);
        else if (!game.busy()) {
            resultAt = now;
            faceUntil = now + 3500;
            if (game.phase == Phase::Won) {
                beep(2400, 500);
                recordWin();
                assistantSet(Emotion::Excited, "Excelente! Llegaste!", "Reaccion local");
                queueAI("El jugador completo el recorrido. Felicitalo.", false);
            } else if (game.phase == Phase::Lost) {
                beep(180, 900);
                assistantSet(Emotion::Sad, "Otra oportunidad! Tu puedes.", "Reaccion local");
                queueAI("El jugador perdio sus tres vidas. Animalo a practicar.", false);
            } else {
                assistantSet(Emotion::Neutral, "Preparados para otro intento", "Reaccion local");
            }
        }
    }
    static uint32_t lastHeartbeat = 0;
    if (now - lastHeartbeat >= 10000) {
        lastHeartbeat = now;
        Serial.printf("[VIVO] version=%s tiempo=%lu clientes_AP=%u portal=%s memoria=%u\n",
                      FIRMWARE_VERSION, (unsigned long)(now / 1000), WiFi.softAPgetStationNum(),
                      wifiPortalOpen ? "8080" : "cerrado", ESP.getFreeHeap());
    }
    updateSound(now);
    if (!game.busy() && now - assistantSnapshot().changed > 90000)
        assistantSet(Emotion::Sleepy, "Me invitas a jugar?", "Reposo local");

    server.handleClient();
    processNetwork(game.busy());
    draw(millis());
    delay(1); // Ceder CPU al sistema sin bloquear cronómetro ni botones.
}
