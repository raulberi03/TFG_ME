#pragma once
#include <TFT_eSPI.h>

class PantallaBase {
public:
    // Dibuja el borde blanco y fondo negro
    static void fondoConBorde(TFT_eSPI& tft) {
        tft.fillScreen(TFT_BLACK);
        tft.drawRect(0, 0, tft.width(), tft.height(), TFT_WHITE);
    }

    // Dibuja el logo WiFi en la esquina superior derecha
    static void dibujarLogoWiFi(TFT_eSPI& tft) {
        int x = tft.width() - 35;
        int y = 10;
        tft.fillCircle(x, y + 15, 3, TFT_WHITE);
        tft.drawArc(x, y + 15, 8, 8, 200, 340, TFT_WHITE, TFT_BLACK);
        tft.drawArc(x, y + 15, 13, 13, 210, 330, TFT_WHITE, TFT_BLACK);
        tft.drawArc(x, y + 15, 18, 18, 220, 320, TFT_WHITE, TFT_BLACK);
    }

    // Dibuja el texto superior y la caja de contenido
    static void dibujarTextoYCaja(TFT_eSPI& tft, const String& texto, const String& valor) {
        int h = (int)(tft.height() * 0.20f);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(2);
        tft.drawString(texto.c_str(), tft.width() / 2, h / 2);
        int boxX = 20;
        int boxW = tft.width() - 40;
        int boxH = 40;
        int boxY = h + 10;
        tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
        tft.setTextSize(3);
        tft.drawString(valor.c_str(), tft.width() / 2, boxY + boxH / 2);
    }

    // Refresca solo el valor en la caja
    static void refrescarCajaValor(TFT_eSPI& tft, const String& valor) {
        int h = (int)(tft.height() * 0.20f);
        int boxX = 20;
        int boxW = tft.width() - 40;
        int boxH = 40;
        int boxY = h + 10;
        tft.fillRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(3);
        tft.drawString(valor.c_str(), tft.width() / 2, boxY + boxH / 2);
    }
};
