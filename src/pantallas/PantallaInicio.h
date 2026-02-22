#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "PantallaBase.h"
#include "PantallaNumerica.h"
#include "FontHelper.h"

namespace PantallaInicio {

  inline void mostrar(TFT_eSPI& tft) {
    PantallaBase::fondoConBorde(tft);

    // Texto centrado "Proyecto TFG"
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    FontHelper::drawStringWithSpanish(tft, "Proyecto TFG", tft.width() / 2, tft.height() / 2, FontHelper::FONT_TITULO);

    delay(2000);
  }
}