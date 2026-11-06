#pragma once
// Interfaz común de módulos. Las definiciones viven en archivos .cpp.
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "portal_manager.h"
#include <Preferences.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"
#include "game.h"
#include "assistant.h"
struct Score {
    String name;
    uint32_t ms;
    uint8_t errors;
    uint32_t id;
};
struct Weather {
    bool valid = false, failed = false;
    float temp = 0;
    int code = 0;
    bool day = true;
    uint32_t fetched = 0;
};
extern WebServer *httpServer;
WebServer &http();
extern PortalManager wifiManager;
extern Preferences prefs;
extern Adafruit_SSD1306 oled;
extern Game game;
extern String player;
extern Score ranking[30];
extern int rankCount;
extern bool oledOK, idleSound, wifiPortalOpen;
extern uint32_t bootAt, lastDraw, lastChirp, toneEnd, lastWifiTry, wifiPortalAt, faceUntil,
    resultAt;
Weather getWeather();
const char *phaseName();
String clockText(uint32_t ms);
void beep(int hz, uint32_t duration);
void updateSound(uint32_t now);
void recordWin();
bool saveRanks();
uint16_t wifiFailureReason();
void loadRanks();
const char *weatherIcon(const Weather &w);
void routes();
void draw(uint32_t now);
void weatherTask(void *);
void beginNetwork();
void processNetwork(bool busy);
void clearContactLatch();

const char *phaseLabel();

String oledSafe(String text);
