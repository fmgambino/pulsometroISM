#include <cassert>
#include "game.h"
#include "ranking_order.h"
// Casos de reglas y límites sin necesidad de conectar una placa.
int main() {
    Game g;
    assert(!g.start(0, true, false));
    assert(!g.start(0, false, true));
    assert(g.start(0, false, false));
    assert(!g.start(1, false, false));
    g.tick(2999, false, false, false);
    assert(g.phase == Phase::Countdown);
    g.tick(3000, false, false, false);
    assert(g.phase == Phase::Running);
    g.tick(3100, true, true, false);
    assert(g.errors == 1);
    g.tick(3500, true, false, false);
    assert(g.errors == 1);
    g.tick(3600, false, false, false);
    g.tick(3651, false, false, false);
    g.tick(3700, true, true, false);
    assert(g.errors == 2);
    g.tick(3900, false, false, false);
    g.tick(4000, true, true, true);
    assert(g.phase == Phase::Lost && g.errors == 3);
    Game w;
    w.start(100, false, false);
    w.tick(3100, false, false, false);
    w.tick(4220, false, false, true);
    assert(w.phase == Phase::Won && w.elapsed == 1120);
    Game c;
    c.start(0, false, false);
    c.tick(100, false, true, false);
    assert(c.phase == Phase::Cancelled);
    Game t;
    t.start(0, false, false);
    t.tick(3000, false, false, false);
    t.tick(603000, false, false, false);
    assert(t.phase == Phase::Timeout);
    Game rollover;
    rollover.start(UINT32_MAX - 2000, false, false);
    rollover.tick(999, false, false, false);
    assert(rollover.phase == Phase::Running);
    Game pulse;
    pulse.start(0, false, false);
    pulse.tick(3000, false, false, false);
    pulse.tick(3010, false, true, false);
    assert(pulse.errors == 1);

    // La llegada exactamente al límite no debe clasificar.
    Game boundary;
    boundary.start(0, false, false);
    boundary.tick(3000, false, false, false);
    boundary.tick(603000, false, false, true);
    assert(boundary.phase == Phase::Timeout);
    // Cancelar y volver a empezar no arrastra tiempo, vidas ni contactos de otra partida.
    Game reset;
    reset.start(0, false, false);
    reset.tick(3000, false, false, false);
    reset.tick(3100, true, true, false);
    reset.cancel(3200);
    assert(reset.phase == Phase::Cancelled && reset.elapsed == 200);
    assert(reset.start(4000, false, false) && reset.errors == 0 && reset.elapsed == 0);
    // Un contacto separado por menos de 150 ms no vuelve a consumir una vida.
    Game bounce;
    bounce.start(0, false, false);
    bounce.tick(3000, false, false, false);
    bounce.tick(3010, true, true, false);
    bounce.tick(3050, false, false, false);
    bounce.tick(3100, false, true, false);
    assert(bounce.errors == 1);
    // Un pulsador B durante la cuenta también cancela.
    Game early;
    early.start(0, false, false);
    early.tick(500, false, false, true);
    assert(early.phase == Phase::Cancelled);
    // Precisión antes de velocidad, tiempo como desempate y empate exacto.
    assert(scoreBetter(0, 90000, 1, 10000));
    assert(scoreBetter(1, 10000, 1, 20000));
    assert(!scoreBetter(2, 1000, 0, 90000));
    assert(!scoreBetter(1, 10000, 1, 10000));
}
