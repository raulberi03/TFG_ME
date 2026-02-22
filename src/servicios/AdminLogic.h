#pragma once
#include <TFT_eSPI.h>


namespace AdminLogic {

// Estado del menú administrador
inline bool& menuActivo() { static bool a = false; return a; }

// Código especial de superusuario
constexpr const char* SUPERUSER_CODE = "1234";
constexpr const char* SUPERUSER_PASS = "1234";

// Llama esto cuando se detecte el código y password de superusuario
inline void mostrarMenu(TFT_eSPI& tft) {
    menuActivo() = true;
    MenuAdministrador::pintada() = false;
}

// Llama esto en el loop principal si el menú está activo
inline void loopMenu(TFT_eSPI& tft, XPT2046_Touchscreen& ts) {
    // Si la pantalla alfanumérica está activa, no procesar menú
    extern bool PantallaAlfanumerica_pintada();
    if (PantallaAlfanumerica_pintada && PantallaAlfanumerica_pintada()) return;
    if (!MenuAdministrador::pintada()) {
        MenuAdministrador::mostrar(tft);
    }
    TS_Point p = ts.getPoint();
    if (p.z >= 20) {
        static uint32_t ultimoToqueMs = 0;
        uint32_t ahora = millis();
        if (ahora - ultimoToqueMs > 180) {
            ultimoToqueMs = ahora;
            int x = map(p.y, 3723, 378, 0, tft.width() - 1);
            int y = map(p.x, 446, 3832, 0, tft.height() - 1);
            x = constrain(x, 0, tft.width() - 1);
            y = constrain(y, 0, tft.height() - 1);
            // Detectar opción tocada
            int y0 = 80;
            int h = 40;
            if (MenuAdministrador::menuActivo() == -1) {
                for (int i = 0; i < MenuAdministrador::NUM_OPCIONES; ++i) {
                    int yTop = y0 + i * h - 10;
                    int yBot = yTop + 35;
                    if (y >= yTop && y <= yBot) {
                        MenuAdministrador::opcionSeleccionada() = i;
                        MenuAdministrador::mostrar(tft);
                        MenuAdministrador::actualizar(tft, 0); // OK
                        break;
                    }
                }
            } else {
                int total = MenuAdministrador::NUM_SUBOPCIONES + 1; // +1 para Volver
                for (int i = 0; i < total; ++i) {
                    int yTop = y0 + i * h - 10;
                    int yBot = yTop + 35;
                    if (y >= yTop && y <= yBot) {
                        MenuAdministrador::subOpcionSeleccionada() = i;
                        MenuAdministrador::mostrar(tft);
                        MenuAdministrador::actualizar(tft, 0); // OK
                        break;
                    }
                }
            }
        }
    }
}

// Llama esto cuando quieras salir del menú
inline void salirMenu() {
    menuActivo() = false;
}

} // namespace
