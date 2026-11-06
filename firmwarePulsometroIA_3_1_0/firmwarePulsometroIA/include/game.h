#pragma once
#include <stdint.h>

// Núcleo independiente de Arduino: permite verificar las reglas en una PC.
enum class Phase { Idle, Countdown, Running, Won, Lost, Cancelled, Timeout };
struct Game {
    static constexpr uint32_t COUNTDOWN_MS = 3000;
    static constexpr uint32_t RELEASE_MS = 150;
    static constexpr uint32_t MAX_GAME_MS = 600000;
    Phase phase = Phase::Idle;
    uint32_t mark = 0, elapsed = 0, clearSince = 0;
    unsigned errors = 0;
    bool armed = true;

    bool busy() const {
        return phase == Phase::Countdown || phase == Phase::Running;
    }

    // Play prepara el inicio. El organizador debe colocar el aro físicamente en A.
    bool start(uint32_t now, bool touch, bool finish) {
        if (busy() || touch || finish)
            return false;
        errors = 0;
        elapsed = 0;
        mark = now;
        clearSince = now;
        phase = Phase::Countdown;
        armed = true;
        return true;
    }
    // Procesar contactos antes de llegada: un tercer error simultáneo causa derrota.
    // contactEvent es la señal capturada por la interrupción para no perder roces.
    void tick(uint32_t now, bool touch, bool contactEvent, bool finishEdge) {
        if (phase == Phase::Countdown) {
            if (touch || contactEvent || finishEdge) {
                phase = Phase::Cancelled;
                return;
            }
            if (uint32_t(now - mark) < COUNTDOWN_MS)
                return;
            phase = Phase::Running;
            mark = now;
            clearSince = now;
        }
        if (phase != Phase::Running)
            return;
        elapsed = now - mark; // Resta sin signo: tolera el desbordamiento de millis().
        if (touch || contactEvent) {
            clearSince = now;
            if (armed) {
                ++errors;
                armed = false;
                if (errors == 3) {
                    phase = Phase::Lost;
                    return;
                }
            }
        } else if (uint32_t(now - clearSince) >= RELEASE_MS) {
            armed = true; // Solo otro contacto tras separarse consume otra vida.
        }
        if (elapsed >= MAX_GAME_MS) {
            phase = Phase::Timeout;
            return;
        }
        if (finishEdge)
            phase = Phase::Won;
    }
    void cancel(uint32_t now) {
        if (busy()) {
            if (phase == Phase::Running)
                elapsed = now - mark;
            phase = Phase::Cancelled;
        }
    }
};
