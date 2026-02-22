#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "PantallaBase.h"
#include "PantallaNumerica.h"

namespace PantallaInicio {

  inline void mostrar(TFT_eSPI& tft) {
    PantallaBase::fondoConBorde(tft);

    // Texto centrado "Proyecto TFG"
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.drawString("Proyecto TFG", tft.width() / 2, tft.height() / 2);

    delay(2000);
  }
}