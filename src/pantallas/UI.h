#pragma once
#include <TFT_eSPI.h>

namespace UI {
  inline void fondoConBorde(TFT_eSPI& tft) {
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(0, 0, tft.width(), tft.height(), TFT_WHITE);
  }
}