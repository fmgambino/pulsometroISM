#include "app.h"
#include "ranking_order.h"
// Ranking persistente TOP 30: menos errores, luego menor tiempo.
// Comparar precisión antes de velocidad.
bool better(const Score &a, const Score &b) {
    return scoreBetter(a.errors, a.ms, b.errors, b.ms);
}
// Guardar el ranking completo solamente cuando cambia; evita desgaste por cada tick.
bool saveRanks() {
    DynamicJsonDocument d(8192);
    JsonArray a = d.to<JsonArray>();
    for (int i = 0; i < rankCount; i++) {
        auto o = a.createNestedObject();
        o["id"] = ranking[i].id;
        o["name"] = ranking[i].name;
        o["ms"] = ranking[i].ms;
        o["errors"] = ranking[i].errors;
    }
    String s;
    serializeJson(d, s);
    if (prefs.putString("ranking", s) == 0) {
        Serial.println("No se pudo guardar ranking en NVS");
        return false;
    }
    return true;
}
// Recuperar NVS y descartar registros inválidos sin bloquear el arranque.
void loadRanks() {
    DynamicJsonDocument d(8192);
    if (!prefs.isKey("ranking"))
        return;
    bool migrated = false;
    if (deserializeJson(d, prefs.getString("ranking", "[]")))
        return;
    for (JsonObject o : d.as<JsonArray>()) {
        if (rankCount == 30)
            break;
        if (!o["name"].is<const char *>() || !o["ms"].is<uint32_t>() || !o["errors"].is<int>())
            continue;
        int e = o["errors"];
        if (e < 0 || e > 2)
            continue;
        String name = o["name"].as<String>();
        uint32_t ms = o["ms"].as<uint32_t>();
        if (name.length() == 0 || name.length() > 64 || ms >= Game::MAX_GAME_MS)
            continue;
        Score loaded = {name, ms, (uint8_t)e};
        loaded.id = o["id"] | 0UL;
        bool duplicate = loaded.id == 0;
        for (int j = 0; j < rankCount; j++)
            if (ranking[j].id == loaded.id)
                duplicate = true;
        if (duplicate) {
            bool collision;
            do {
                loaded.id = esp_random();
                collision = loaded.id == 0;
                for (int j = 0; j < rankCount; j++)
                    if (ranking[j].id == loaded.id)
                        collision = true;
            } while (collision);
            migrated = true;
        }
        ranking[rankCount++] = loaded;
    }
    if (migrated)
        saveRanks(); // Migrar sin perder resultados de versiones anteriores.
    // Restaurar el orden incluso si se importa una lista guardada por otra versión.
    for (int i = 1; i < rankCount; i++) {
        Score current = ranking[i];
        int j = i;
        while (j > 0 && better(current, ranking[j - 1])) {
            ranking[j] = ranking[j - 1];
            --j;
        }
        ranking[j] = current;
    }
}
// Insertar ordenadamente y conservar solamente los 30 mejores recorridos.
void recordWin() {
    Score s = {player, game.elapsed, (uint8_t)game.errors};
    bool collision;
    do {
        s.id = esp_random();
        collision = s.id == 0;
        for (int j = 0; j < rankCount; j++)
            if (ranking[j].id == s.id)
                collision = true;
    } while (collision);
    int pos = 0;
    while (pos < rankCount && !better(s, ranking[pos]))
        pos++;
    if (pos >= 30)
        return;
    int end = min(rankCount, 29);
    for (int i = end; i > pos; i--)
        ranking[i] = ranking[i - 1];
    ranking[pos] = s;
    rankCount = min(rankCount + 1, 30);
    saveRanks();
}
