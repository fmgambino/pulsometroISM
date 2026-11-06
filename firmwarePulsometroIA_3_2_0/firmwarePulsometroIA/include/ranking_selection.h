#pragma once
#include <stddef.h>
#include <stdint.h>
// Primero validar todos los IDs; una selección vieja nunca borra otras filas.
// El orden relativo de los resultados conservados permanece intacto.
template <class Record>
int removeSelected(Record *records, int count, const uint32_t *ids, size_t selected) {
    if (selected == 0 || selected > 30)
        return -1;
    for (size_t j = 0; j < selected; j++) {
        bool found = false;
        for (int i = 0; i < count; i++)
            if (records[i].id == ids[j] && ids[j] != 0)
                found = true;
        if (!found)
            return -1;
    }
    int write = 0;
    for (int i = 0; i < count; i++) {
        bool remove = false;
        for (size_t j = 0; j < selected; j++)
            if (records[i].id == ids[j])
                remove = true;
        if (!remove)
            records[write++] = records[i];
    }
    return write;
}
