#pragma once
#include <TFT_eSPI.h>
#include <U8g2_for_TFT_eSPI.h>

namespace FontHelper {
    // Fuentes predefinidas para diferentes usos
    const uint8_t* FONT_TITULO = u8g2_font_helvB24_tf;      // 24pt Bold para títulos
    const uint8_t* FONT_BOTON = u8g2_font_helvR18_tf;       // 18pt para botones
    const uint8_t* FONT_TEXTO = u8g2_font_helvR14_tf;       // 14pt para texto normal
    const uint8_t* FONT_PEQUENA = u8g2_font_helvR10_tf;     // 10pt para texto pequeño
    
    // Instancia global de U8g2
    inline U8g2_for_TFT_eSPI& getU8g2(TFT_eSPI& tft) {
        static U8g2_for_TFT_eSPI u8g2;
        static bool initialized = false;
        if (!initialized) {
            u8g2.begin(tft);
            initialized = true;
        }
        return u8g2;
    }
    
    // Dibuja texto con soporte completo UTF-8 usando U8g2
    // Centrado horizontal y vertical automático
    inline void drawStringWithSpanish(TFT_eSPI& tft, const String& text, int x, int y, const uint8_t *font = FONT_BOTON) {
        U8g2_for_TFT_eSPI& u8g2 = getU8g2(tft);
        u8g2.setFont(font);
        u8g2.setForegroundColor(tft.textcolor);
        u8g2.setBackgroundColor(tft.textbgcolor);
        
        // Centrar horizontalmente
        int textWidth = u8g2.getUTF8Width(text.c_str());
        int xPos = x - textWidth / 2;
        
        // Centrar verticalmente usando ascent
        int yPos = y + u8g2.getFontAscent() / 2 - u8g2.getFontDescent() / 2;
        
        u8g2.setCursor(xPos, yPos);
        u8g2.print(text);
    }
    
    // Versión centrada (alias para compatibilidad)
    inline void drawCentreStringWithSpanish(TFT_eSPI& tft, const String& text, int x, int y, const uint8_t *font = FONT_BOTON) {
        drawStringWithSpanish(tft, text, x, y, font);
    }
    
    // Obtiene el ancho de un texto (útil para centrado manual)
    inline int getTextWidth(TFT_eSPI& tft, const String& text, const uint8_t *font = FONT_BOTON) {
        U8g2_for_TFT_eSPI& u8g2 = getU8g2(tft);
        u8g2.setFont(font);
        return u8g2.getUTF8Width(text.c_str());
    }
    
    // Obtiene la altura de la fuente
    inline int getFontHeight(TFT_eSPI& tft, const uint8_t *font = FONT_BOTON) {
        U8g2_for_TFT_eSPI& u8g2 = getU8g2(tft);
        u8g2.setFont(font);
        return u8g2.getFontAscent() - u8g2.getFontDescent();
    }
}