#include "app.h"
// Formatos comunes de estado y cronómetro.
const char *phaseName() {
    switch (game.phase) {
    case Phase::Idle:
        return "idle";
    case Phase::Countdown:
        return "countdown";
    case Phase::Running:
        return "running";
    case Phase::Won:
        return "won";
    case Phase::Lost:
        return "lost";
    case Phase::Cancelled:
        return "cancelled";
    default:
        return "timeout";
    }
}
String clockText(uint32_t ms) {
    char s[24];
    snprintf(s, sizeof(s), "%02lu:%02lu,%02lu", (unsigned long)(ms / 60000),
             (unsigned long)(ms / 1000 % 60), (unsigned long)(ms / 10 % 100));
    return s;
}

// Etiquetas visibles en OLED, independientes de los identificadores JSON.
const char *phaseLabel() {
    switch (game.phase) {
    case Phase::Idle:
        return "Listo";
    case Phase::Countdown:
        return "Preparados";
    case Phase::Running:
        return "En marcha";
    case Phase::Won:
        return "Completado";
    case Phase::Lost:
        return "Sin vidas";
    case Phase::Cancelled:
        return "Cancelado";
    default:
        return "Tiempo agotado";
    }
}
