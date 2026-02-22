#pragma once
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Arduino.h>
#include "pantallas/PantallaNumerica.h"
#include "pantallas/PantallaAlfanumerica.h"
#include "pantallas/MenuAdministrador.h"

namespace TouchController {
    namespace {
        bool readTouch(TFT_eSPI& tft, XPT2046_Touchscreen& ts, int& outX, int& outY) {
            TS_Point p = ts.getPoint();
            if (p.z < 20) {
                return false;
            }
            int x = map(p.y, 3723, 378, 0, tft.width() - 1);
            int y = map(p.x, 446, 3832, 0, tft.height() - 1);
            outX = constrain(x, 0, tft.width() - 1);
            outY = constrain(y, 0, tft.height() - 1);
            return true;
        }
    }

    // Procesa entradas tactiles para teclado numerico o menu.
    inline void process(TFT_eSPI& tft, XPT2046_Touchscreen& ts) {
        static uint32_t ultimoToqueMs = 0;
        uint32_t ahora = millis();
        if (ahora - ultimoToqueMs <= 180) {
            return;
        }

        int x = 0;
        int y = 0;
        if (!readTouch(tft, ts, x, y)) {
            return;
        }
        ultimoToqueMs = ahora;

        if (PantallaNumerica::pintada()) {
            int y0 = PantallaNumerica::tecladoY(tft);
            if (y >= y0) {
                int col = x / PantallaNumerica::teclaW(tft);
                int fila = (y - y0) / PantallaNumerica::teclaH(tft);
                if (col >= 0 && col <= 2 && fila >= 0 && fila <= 3) {
                    PantallaNumerica::efectoPulsacion(tft, fila, col);
                    PantallaNumerica::procesarTecla(tft, PantallaNumerica::labelTecla(fila, col));
                }
            }
            return;
        }

        if (PantallaAlfanumerica::pintada()) {
            int y0 = PantallaAlfanumerica::tecladoY(tft);
            if (y >= y0) {
                int fila = (y - y0) / PantallaAlfanumerica::teclaH(tft);
                if (fila >= 0 && fila < 4) {
                    int keyX = 0;
                    int keyW = 0;
                    const char* label = PantallaAlfanumerica::keyAt(tft, fila, x, keyX, keyW);
                    if (strlen(label) > 0) {
                        PantallaAlfanumerica::efectoPulsacion(tft, fila, keyX, keyW, label);
                        PantallaAlfanumerica::procesarTecla(tft, label);
                    }
                }
            }
            return;
        }

        if (MenuAdministrador::pintada()) {
            MenuAdministrador::procesarToque(tft, x, y);
        }
    }
}
