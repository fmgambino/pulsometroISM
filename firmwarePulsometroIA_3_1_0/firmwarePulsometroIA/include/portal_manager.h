#pragma once
#include <WiFiManager.h>
// Adaptador de la biblioteca oficial: un único servidor HTTP sirve portal y juego.
// No se modifica la copia descargada de WiFiManager ni se duplican servidores.
class PortalManager : public WiFiManager {
  public:
    PortalManager() {
        _disableConfigPortal = false; // Mantener el servidor al guardar una red.
        _allowExit = false;           // La navegación no destruye el servidor del juego.
    }
    void renderHome() {
        handleRoot();
    }
    void renderNetworks(bool scan = true) {
        handleWifi(scan);
    }
    void saveNetwork() {
        handleWifiSave();
    }
};
