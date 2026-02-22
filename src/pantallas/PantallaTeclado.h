#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "UI.h"

namespace PantallaTeclado {

  // --- Estado ---
  inline String& codigo() { static String c = ""; return c; }
  inline bool& pintada() { static bool p = false; return p; }
  inline uint32_t& ultimoToqueMs() { static uint32_t t = 0; return t; }

  // --- Layout ---
  inline int topH(TFT_eSPI& tft) { return (int)(tft.height() * 0.30f); }
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

  inline void dibujarZonaSuperior(TFT_eSPI& tft) {
    int h = topH(tft);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

    tft.setTextSize(2);
    tft.drawString("Codigo Usuario", tft.width() / 2, (int)(h * 0.25f));

    int boxX = 20;
    int boxW = tft.width() - 40;
    int boxH = (int)(h * 0.40f);
    int boxY = (int)(h * 0.45f);

    tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);

    tft.setTextSize(3);
    tft.drawString(codigo().c_str(), tft.width() / 2, boxY + boxH / 2);
  }

  inline void refrescarCajaCodigo(TFT_eSPI& tft) {
    int h = topH(tft);

    int boxX = 20;
    int boxW = tft.width() - 40;
    int boxH = (int)(h * 0.40f);
    int boxY = (int)(h * 0.45f);

    tft.fillRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.drawString(codigo().c_str(), tft.width() / 2, boxY + boxH / 2);
  }

  inline void dibujarTecla(TFT_eSPI& tft, int fila, int col, bool pulsada) {
    int y0 = tecladoY(tft);
    int w  = teclaW(tft);
    int h  = teclaH(tft);

    int x = col * w;
    int y = y0 + fila * h;

    // Fondo y texto según estado
    uint16_t bg = pulsada ? TFT_WHITE : TFT_BLACK;
    uint16_t fg = pulsada ? TFT_BLACK : TFT_WHITE;

    // Relleno completo + borde + texto
    tft.fillRect(x, y, w, h, bg);
    tft.drawRect(x, y, w, h, TFT_WHITE);

    tft.setTextColor(fg, bg);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString(labelTecla(fila, col), x + w / 2, y + h / 2);
  }

  inline void efectoPulsacion(TFT_eSPI& tft, int fila, int col) {
    dibujarTecla(tft, fila, col, true);
    delay(90);                       // “flash” corto
    dibujarTecla(tft, fila, col, false);
  }

  inline void dibujarTeclado(TFT_eSPI& tft) {
    for (int fila = 0; fila < 4; fila++) {
      for (int col = 0; col < 3; col++) {
        dibujarTecla(tft, fila, col, false);
      }
    }
  }

  inline void mostrar(TFT_eSPI& tft) {
    UI::fondoConBorde(tft);
    dibujarZonaSuperior(tft);
    dibujarTeclado(tft);
    pintada() = true;
  }

  inline void procesarTecla(TFT_eSPI& tft, const char* k) {
    if (strcmp(k, "CLR") == 0) {
      codigo() = "";
      refrescarCajaCodigo(tft);
      return;
    }
    if (strcmp(k, "OK") == 0) {
      return; // luego validas
    }
    if (codigo().length() < 8) {
      codigo() += k;
      refrescarCajaCodigo(tft);
    }
  }

  // ---- TOUCH XPT2046 ----
  // Ajusta estos 4 valores con tu calibración real:
  // (si están mal, tocarás "desplazado" o invertido)
  static const int TS_MINX = 200;
  static const int TS_MAXX = 3800;
  static const int TS_MINY = 200;
  static const int TS_MAXY = 3800;

  inline bool leerToque(XPT2046_Touchscreen& ts, TFT_eSPI& tft, uint16_t &x, uint16_t &y) {
    if (!ts.touched()) return false;

    TS_Point p = ts.getPoint();

    // map a coordenadas pantalla
    int sx = map(p.x, TS_MINX, TS_MAXX, 0, tft.width() - 1);
    int sy = map(p.y, TS_MINY, TS_MAXY, 0, tft.height() - 1);

    // Si con setRotation(2) notas invertido, aquí se corrige fácil:
    // sx = (tft.width() - 1) - sx;  // invertir X
    // sy = (tft.height() - 1) - sy; // invertir Y

    x = (uint16_t)constrain(sx, 0, tft.width() - 1);
    y = (uint16_t)constrain(sy, 0, tft.height() - 1);
    return true;
  }

  // Llamar desde loop()
  inline void actualizar(TFT_eSPI& tft, XPT2046_Touchscreen& ts) {

    // Dibujar la pantalla una sola vez
    if (!pintada()) {
      mostrar(tft);
    }

    // Leer touch (RAW)
    TS_Point p = ts.getPoint();

    // Umbral de presión (bájalo si necesitas apretar mucho)
    if (p.z < 20) return;

    // Anti-rebote simple
    uint32_t ahora = millis();
    if (ahora - ultimoToqueMs() < 180) return;
    ultimoToqueMs() = ahora;

    // ====== Calibración (tus esquinas) ======
    // Arriba-izquierda:  x=446  y=3723
    // Arriba-derecha:    x=801  y=863
    // Abajo-izquierda:   x=3415 y=3390
    // Abajo-derecha:     x=3832 y=378
    const int TS_MINX = 446;
    const int TS_MAXX = 3832;
    const int TS_MINY = 378;
    const int TS_MAXY = 3723;

    // ====== Mapeo correcto para tu caso ======
    // - RAW y -> X pantalla (invertido)
    // - RAW x -> Y pantalla
    int x = map(p.y, TS_MAXY, TS_MINY, 0, tft.width() - 1);
    int y = map(p.x, TS_MINX, TS_MAXX, 0, tft.height() - 1);

    x = constrain(x, 0, tft.width() - 1);
    y = constrain(y, 0, tft.height() - 1);

    // ====== Detectar tecla ======
    int y0 = tecladoY(tft);
    if (y < y0) return; // toque en zona superior (no teclado)

    int col = x / teclaW(tft);
    int fila = (y - y0) / teclaH(tft);

    if (col < 0 || col > 2 || fila < 0 || fila > 3) return;

    // ====== Efecto visual + acción ======
    efectoPulsacion(tft, fila, col);
    procesarTecla(tft, labelTecla(fila, col));
  }
}