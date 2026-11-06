#include "app.h"
// El buzzer usa LEDC por canal, API de Arduino-ESP32 2.0.17.
// No se usa delay para mantener ni detener los sonidos.
static bool toneActive = false;
void beep(int hz, uint32_t duration) {
    ledcWriteTone(0, hz);
    toneEnd = millis() + duration;
    toneActive = true;
}
void updateSound(uint32_t now) {
    if (toneActive && int32_t(now - toneEnd) >= 0) {
        ledcWriteTone(0, 0);
        toneActive = false;
    }
    if (!game.busy() && now - bootAt > BOOT_PRESENTATION_MS && idleSound && !toneActive &&
        now - lastChirp > 20000) {
        lastChirp = now;
        beep(1600 + ((now / 100) % 6) * 100, 65);
    }
}
