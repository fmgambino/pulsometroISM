#pragma once
#include <stdint.h>
// Contrato del ranking: menos errores siempre tiene prioridad sobre menor tiempo.
inline bool scoreBetter(uint8_t errorsA, uint32_t timeA, uint8_t errorsB, uint32_t timeB) {
    return errorsA < errorsB || (errorsA == errorsB && timeA < timeB);
}
