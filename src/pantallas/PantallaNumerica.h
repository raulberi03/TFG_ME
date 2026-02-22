#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "PantallaBase.h"

namespace PantallaNumerica {

  typedef void (*OnOkCallback)(TFT_eSPI& tft, const String& valor);
  typedef void (*OnClrCallback)(TFT_eSPI& tft, String& valor);

  struct Config {
    String textoSuperior;
    uint8_t maxLen = 8;
    OnOkCallback onOk = nullptr;
    OnClrCallback onClr = nullptr;
  };

  inline String& valor() { static String v = ""; return v; }
  inline bool& pintada() { static bool p = false; return p; }
  inline Config& config() { static Config c; return c; }

  inline void dibujarZonaSuperior(TFT_eSPI& tft) {
    PantallaBase::dibujarLogoWiFiConEstado(tft);
    PantallaBase::dibujarTextoYCaja(tft, config().textoSuperior, valor());
  }

  inline void refrescarCajaValor(TFT_eSPI& tft) {
    PantallaBase::refrescarCajaValor(tft, valor());
  }

  inline int topH(TFT_eSPI& tft) { return (int)(tft.height() * 0.38f); }
  inline int tecladoY(TFT_eSPI& tft) { return topH(tft); }
  inline int tecladoH(TFT_eSPI& tft) { return tft.height() - tecladoY(tft); }
  inline int teclaW(TFT_eSPI& tft) { return tft.width() / 3; }
  inline int teclaH(TFT_eSPI& tft) { return tecladoH(tft) / 4; }

  inline const char* labelTecla(int fila, int col) {
    static const char* keys[4][3] = {
      {"1","2","3"},
      {"4","5","6"},
      {"7","8","9"},
      {"CLR","0","OK"}
    };
    return keys[fila][col];
  }

  inline void dibujarTecla(TFT_eSPI& tft, int fila, int col, bool pulsada) {
    int y0 = tecladoY(tft);
    int w  = teclaW(tft);
    int h  = teclaH(tft);
    int x = col * w;
    int y = y0 + fila * h;
    uint16_t bg = pulsada ? TFT_WHITE : TFT_BLACK;
    uint16_t fg = pulsada ? TFT_BLACK : TFT_WHITE;
    tft.fillRect(x, y, w, h, bg);
    tft.drawRect(x, y, w, h, TFT_WHITE);
    tft.setTextColor(fg, bg);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString(labelTecla(fila, col), x + w / 2, y + h / 2);
  }

  inline void efectoPulsacion(TFT_eSPI& tft, int fila, int col) {
    dibujarTecla(tft, fila, col, true);
    delay(90);
    dibujarTecla(tft, fila, col, false);
  }

  inline void dibujarTeclado(TFT_eSPI& tft) {
    for (int fila = 0; fila < 4; fila++) {
      for (int col = 0; col < 3; col++) {
        dibujarTecla(tft, fila, col, false);
      }
    }
  }

  inline void mostrar(TFT_eSPI& tft, const Config& cfg) {
    config() = cfg;
    valor() = "";
    PantallaBase::fondoConBorde(tft);
    dibujarZonaSuperior(tft);
    dibujarTeclado(tft);
    pintada() = true;
  }

  inline void procesarTecla(TFT_eSPI& tft, const char* k) {
    if (strcmp(k, "CLR") == 0) {
      if (config().onClr) config().onClr(tft, valor());
      refrescarCajaValor(tft);
      return;
    }
    if (strcmp(k, "OK") == 0) {
      if (config().onOk) config().onOk(tft, valor());
      return;
    }
    if (valor().length() < config().maxLen) {
      valor() += k;
      refrescarCajaValor(tft);
    }
  }
}