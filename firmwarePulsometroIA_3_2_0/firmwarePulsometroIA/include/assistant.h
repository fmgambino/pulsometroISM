#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
// Copias POD: se pueden compartir entre tareas sin compartir objetos String.
enum class Emotion : uint8_t { Happy, Sad, Surprised, Thinking, Sleepy, Excited, Neutral };
struct AssistantState {
    Emotion emotion = Emotion::Happy;
    char text[81] = "Hola! Jugamos?";
    char status[64] = "Local: sin clave API";
    bool online = false;
    uint32_t changed = 0;
};
extern bool networkTaskAvailable;
void beginAssistant();
AssistantState assistantSnapshot();
void assistantSet(Emotion emotion, const char *text, const char *status, bool online = false);
const char *emotionName(Emotion emotion);
bool queueAI(const String &message, bool showThinking = true);
void assistantNetworkStep();
void drawFace(Adafruit_SSD1306 &display, uint32_t now, Emotion emotion);
unsigned assistantCalls();
