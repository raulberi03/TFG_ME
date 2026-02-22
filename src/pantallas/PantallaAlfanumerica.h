#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "PantallaBase.h"
#include "FontHelper.h"

namespace PantallaAlfanumerica {

  typedef void (*OnOkCallback)(TFT_eSPI& tft, const String& valor);
  typedef void (*OnClrCallback)(TFT_eSPI& tft, String& valor);
  typedef void (*OnBackCallback)(TFT_eSPI& tft);

  struct Key {
    const char* label;
    uint8_t span;
  };

  struct Config {
    String textoSuperior;
    uint8_t maxLen = 32;
    OnOkCallback onOk = nullptr;
    OnClrCallback onClr = nullptr;
    OnBackCallback onBack = nullptr;
  };

  inline String& valor() { static String v = ""; return v; }
  inline bool& pintada() { static bool p = false; return p; }
  inline Config& config() { static Config c; return c; }
  inline bool& mayusculas() { static bool m = false; return m; }
  inline bool& tecladoNumerico() { static bool n = false; return n; }

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
  inline int teclaW(TFT_eSPI& tft) { return tft.width() / 10; }
  inline int teclaH(TFT_eSPI& tft) { return tecladoH(tft) / 4; }

  // Layout fijo 4 filas con spans para teclas especiales.
  inline void rowKeysLetras(int fila, bool mayus, const Key*& keys, int& count) {
    static const Key min0[] = {{"q",1},{"w",1},{"e",1},{"r",1},{"t",1},{"y",1},{"u",1},{"i",1},{"o",1},{"p",1}};
    static const Key min1[] = {{"a",1},{"s",1},{"d",1},{"f",1},{"g",1},{"h",1},{"j",1},{"k",1},{"l",1},{"ñ",1}};
    static const Key min2[] = {{"z",1},{"x",1},{"c",1},{"v",1},{"b",1},{"n",1},{"m",1},{"SHIFT",3}};
    static const Key min3[] = {{"ATRAS",3},{"123",2},{"DEL",2},{"OK",3}};

    static const Key may0[] = {{"Q",1},{"W",1},{"E",1},{"R",1},{"T",1},{"Y",1},{"U",1},{"I",1},{"O",1},{"P",1}};
    static const Key may1[] = {{"A",1},{"S",1},{"D",1},{"F",1},{"G",1},{"H",1},{"J",1},{"K",1},{"L",1},{"Ñ",1}};
    static const Key may2[] = {{"Z",1},{"X",1},{"C",1},{"V",1},{"B",1},{"N",1},{"M",1},{"SHIFT",3}};
    static const Key may3[] = {{"ATRAS",3},{"123",2},{"DEL",2},{"OK",3}};

    if (mayus) {
      if (fila == 0) { keys = may0; count = (int)(sizeof(may0) / sizeof(Key)); return; }
      if (fila == 1) { keys = may1; count = (int)(sizeof(may1) / sizeof(Key)); return; }
      if (fila == 2) { keys = may2; count = (int)(sizeof(may2) / sizeof(Key)); return; }
      keys = may3; count = (int)(sizeof(may3) / sizeof(Key)); return;
    }

    if (fila == 0) { keys = min0; count = (int)(sizeof(min0) / sizeof(Key)); return; }
    if (fila == 1) { keys = min1; count = (int)(sizeof(min1) / sizeof(Key)); return; }
    if (fila == 2) { keys = min2; count = (int)(sizeof(min2) / sizeof(Key)); return; }
    keys = min3; count = (int)(sizeof(min3) / sizeof(Key));
  }

  inline void rowKeysNumeros(int fila, const Key*& keys, int& count) {
    static const Key num0[] = {{"1",1},{"2",1},{"3",1},{"4",1},{"5",1},{"6",1},{"7",1},{"8",1},{"9",1},{"0",1}};
    static const Key num1[] = {{"!",1},{"@",1},{"#",1},{"$",1},{"%",1},{"^",1},{"&",1},{"*",1},{"(",1},{")",1}};
    static const Key num2[] = {{"-",1},{"+",1},{"=",1},{"/",1},{":",1},{";",1},{".",1},{",",1},{"'",1},{"_",1}};
    static const Key num3[] = {{"ATRAS",2},{"abc",2},{"DEL",3},{"OK",3}};

    if (fila == 0) { keys = num0; count = (int)(sizeof(num0) / sizeof(Key)); return; }
    if (fila == 1) { keys = num1; count = (int)(sizeof(num1) / sizeof(Key)); return; }
    if (fila == 2) { keys = num2; count = (int)(sizeof(num2) / sizeof(Key)); return; }
    keys = num3; count = (int)(sizeof(num3) / sizeof(Key));
  }

  inline const char* keyAt(TFT_eSPI& tft, int fila, int x, int& keyX, int& keyW) {
    const Key* keys = nullptr;
    int count = 0;
    if (tecladoNumerico()) {
      rowKeysNumeros(fila, keys, count);
    } else {
      rowKeysLetras(fila, mayusculas(), keys, count);
    }

    int curX = 0;
    int unitW = teclaW(tft);
    for (int i = 0; i < count; i++) {
      int w = keys[i].span * unitW;
      if (x >= curX && x < curX + w) {
        keyX = curX;
        keyW = w;
        return keys[i].label;
      }
      curX += w;
    }
    keyX = 0;
    keyW = 0;
    return "";
  }

  inline void dibujarTeclaRect(TFT_eSPI& tft, int fila, int x, int w, const char* label, bool pulsada) {
    if (strlen(label) == 0) {
      return;
    }
    int y = tecladoY(tft) + fila * teclaH(tft);
    int h = teclaH(tft);
    uint16_t bg = pulsada ? TFT_WHITE : TFT_BLACK;
    uint16_t fg = pulsada ? TFT_BLACK : TFT_WHITE;

    tft.fillRect(x, y, w, h, bg);
    tft.drawRect(x, y, w, h, TFT_WHITE);
    tft.setTextColor(fg, bg);
    // Usar fuente más pequeña para teclas con más de 1 carácter
    const uint8_t* font = (strlen(label) > 1) ? FontHelper::FONT_TEXTO : FontHelper::FONT_BOTON;
    FontHelper::drawStringWithSpanish(tft, label, x + w / 2, y + h / 2, font);
  }

  inline void efectoPulsacion(TFT_eSPI& tft, int fila, int x, int w, const char* label) {
    dibujarTeclaRect(tft, fila, x, w, label, true);
    delay(90);
    dibujarTeclaRect(tft, fila, x, w, label, false);
  }

  inline void dibujarTeclado(TFT_eSPI& tft) {
    int unitW = teclaW(tft);
    for (int fila = 0; fila < 4; fila++) {
      int y = tecladoY(tft) + fila * teclaH(tft);
      tft.fillRect(0, y, tft.width(), teclaH(tft), TFT_BLACK);
      const Key* keys = nullptr;
      int count = 0;
      if (tecladoNumerico()) {
        rowKeysNumeros(fila, keys, count);
      } else {
        rowKeysLetras(fila, mayusculas(), keys, count);
      }

      int curX = 0;
      for (int i = 0; i < count; i++) {
        int w = keys[i].span * unitW;
        dibujarTeclaRect(tft, fila, curX, w, keys[i].label, false);
        curX += w;
      }
    }
  }

  inline void mostrar(TFT_eSPI& tft, const Config& cfg) {
    config() = cfg;
    valor() = "";
    mayusculas() = false;
    tecladoNumerico() = false;
    PantallaBase::fondoConBorde(tft);
    dibujarZonaSuperior(tft);
    dibujarTeclado(tft);
    pintada() = true;
  }

  inline void procesarTecla(TFT_eSPI& tft, const char* k) {
    if (strcmp(k, "ATRAS") == 0) {
      if (config().onBack) config().onBack(tft);
      return;
    }

    if (strcmp(k, "SHIFT") == 0) {
      mayusculas() = !mayusculas();
      dibujarTeclado(tft);
      return;
    }

    if (strcmp(k, "123") == 0) {
      tecladoNumerico() = true;
      dibujarTeclado(tft);
      return;
    }

    if (strcmp(k, "abc") == 0) {
      tecladoNumerico() = false;
      mayusculas() = false;
      dibujarTeclado(tft);
      return;
    }

    if (strcmp(k, "BS") == 0 || strcmp(k, "DEL") == 0) {
      if (valor().length() > 0) {
        valor().remove(valor().length() - 1);
      }
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
      if (!tecladoNumerico() && mayusculas() && strlen(k) == 1 && isalpha(k[0])) {
        mayusculas() = false;
        dibujarTeclado(tft);
      }
    }
  }
}
