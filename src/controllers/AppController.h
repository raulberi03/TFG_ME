#pragma once
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "services/RFIDService.h"
#include "services/RFIDUsuariosService.h"
#include <Arduino.h>
#include "pantallas/MenuAdministrador.h"
#include "controllers/LoginController.h"
#include "controllers/RFIDController.h"
#include "controllers/TouchController.h"
#include "controllers/WiFiController.h"
#include "pantallas/PantallaBase.h"
#include "pantallas/PantallaNumerica.h"
#include "pantallas/PantallaAlfanumerica.h"

namespace AppController {
    // Inicializa los controladores y el flujo de pantallas.
    inline void begin(TFT_eSPI& tft, XPT2046_Touchscreen& ts, RFIDService& rfid, RFIDUsuariosService& rfidUsuarios) {
        (void)ts;
        (void)rfid;
        (void)rfidUsuarios;
        WiFiController::cargarCredenciales();
        WiFiController::conectarSilencioso();
        RFIDController::begin(rfidUsuarios);
        LoginController::begin(tft);
    }

    // Orquesta UI, RFID y tactil.
    inline void loop(TFT_eSPI& tft, XPT2046_Touchscreen& ts, RFIDService& rfid, RFIDUsuariosService& rfidUsuarios) {
        (void)rfidUsuarios;
        static uint32_t ultimoWifiCheckMs = 0;
        if (rfid.detectarTarjeta()) {
            Serial.print("RFID UID: ");
            Serial.println(rfid.ultimoUidHex());
            if (!RFIDController::handleCard(tft, rfid.ultimoUidHex()) && !MenuAdministrador::pintada()) {
                LoginController::handleRfidLogin(tft, rfid.ultimoUidValido());
            }
        }

        uint32_t ahoraMs = millis();
        if (ahoraMs - ultimoWifiCheckMs >= 2000) {
            ultimoWifiCheckMs = ahoraMs;
            if (PantallaNumerica::pintada() || PantallaAlfanumerica::pintada()) {
                PantallaBase::dibujarLogoWiFiConEstado(tft);
            }
        }

        if (MenuAdministrador::debeVolverAlLogin()) {
            MenuAdministrador::debeVolverAlLogin() = false;
            LoginController::begin(tft);
            return;
        }
        TouchController::process(tft, ts);
    }

    // Placeholder de configuracion WiFi.
    inline void pedirSSID(TFT_eSPI& tft) {
        WiFiController::pedirSSID(tft);
    }

    inline void pedirPasswordWifi(TFT_eSPI& tft) {
        WiFiController::pedirPasswordWifi(tft);
    }

    inline void gestionarRFIDAgregar(TFT_eSPI& tft) {
        RFIDController::startAdd(tft);
    }

    inline void gestionarRFIDSobrescribir(TFT_eSPI& tft) {
        RFIDController::startOverwrite(tft);
    }

    inline void gestionarRFIDDesvincular(TFT_eSPI& tft) {
        RFIDController::startUnlink(tft);
    }

    inline String obtenerSSID() {
        return WiFiController::obtenerSSID();
    }

    inline String obtenerPassword() {
        return WiFiController::obtenerPassword();
    }

    inline void borrarCredencialesWifi() {
        WiFiController::borrarCredenciales();
    }

    inline bool conectarWifi(TFT_eSPI& tft) {
        return WiFiController::conectarConPantalla(tft);
    }
}
