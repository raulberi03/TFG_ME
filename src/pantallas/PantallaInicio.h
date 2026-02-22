#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "UI.h"
#include "PantallaTeclado.h"

namespace PantallaInicio {

  inline void mostrar(TFT_eSPI& tft) {
    UI::fondoConBorde(tft);

    // Texto centrado "Proyecto TFG"
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.drawString("Proyecto TFG", tft.width() / 2, tft.height() / 2);

    delay(2000);

    // Pasamos al teclado
    PantallaTeclado::mostrar(tft);
  }

}