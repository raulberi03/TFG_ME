#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>
#include "pantallas/PantallaBase.h"
#include "pantallas/MenuAdministrador.h"

namespace WiFiController {
    // Placeholder de pantallas WiFi.
    inline void pedirSSID(TFT_eSPI& tft) {
        PantallaBase::fondoConBorde(tft);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(2);
        tft.drawString("Cambiar SSID", tft.width()/2, tft.height()/2 - 20);
        tft.drawString("Proximamente...", tft.width()/2, tft.height()/2 + 20);
        delay(2000);
        MenuAdministrador::mostrar(tft);
    }

    inline void pedirPasswordWifi(TFT_eSPI& tft) {
        PantallaBase::fondoConBorde(tft);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(2);
        tft.drawString("Cambiar Password WiFi", tft.width()/2, tft.height()/2 - 20);
        tft.drawString("Proximamente...", tft.width()/2, tft.height()/2 + 20);
        delay(2000);
        MenuAdministrador::mostrar(tft);
    }
}
