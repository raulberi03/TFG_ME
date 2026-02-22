#pragma once
#include <TFT_eSPI.h>
#include "PantallaBase.h"
#include "FontHelper.h"

// Forward declarations
namespace AppController {
    void pedirSSID(TFT_eSPI& tft);
    void pedirPasswordWifi(TFT_eSPI& tft);
    String obtenerSSID();
    String obtenerPassword();
    void borrarCredencialesWifi();
    void gestionarRFIDAgregar(TFT_eSPI& tft);
    void gestionarRFIDSobrescribir(TFT_eSPI& tft);
    void gestionarRFIDDesvincular(TFT_eSPI& tft);
}

namespace MenuAdministrador {

// Estado del menú
inline bool& pintada() { static bool p = false; return p; }
inline bool& debeVolverAlLogin() { static bool v = false; return v; }
inline int& opcionSeleccionada() { static int o = 0; return o; }

// Opciones principales del menú de administrador
constexpr int NUM_OPCIONES = 4;
namespace { const char* const opciones[NUM_OPCIONES] = {
    "Cambiar Wifi",
    "Gestion RFID",
    "Gestion Huella",
    "Salir"
}; }

// Submenús para cada gestión
constexpr int NUM_SUBOPCIONES_WIFI = 4;
namespace { const char* const subopcionesWifi[NUM_SUBOPCIONES_WIFI] = { "Cambiar SSID", "Cambiar Contraseña", "Eliminar red", "Volver" }; }

constexpr int NUM_SUBOPCIONES_RFID = 3;
namespace { const char* const subopcionesRfid[NUM_SUBOPCIONES_RFID] = { "Agregar", "Sobrescribir", "Desvincular" }; }

constexpr int NUM_SUBOPCIONES_GENERICAS = 1;
namespace { const char* const subopcionesGenericas[NUM_SUBOPCIONES_GENERICAS] = { "Opcion 1" }; }

inline int& subOpcionSeleccionada() { static int s = 0; return s; }
inline int& menuActivo() { static int m = -1; return m; } // -1: menú principal, 0-2: submenú

inline int subopcionesCount() {
    if (menuActivo() == 0) return NUM_SUBOPCIONES_WIFI;
    if (menuActivo() == 1) return NUM_SUBOPCIONES_RFID;
    return NUM_SUBOPCIONES_GENERICAS;
}

inline const char* subopcionTexto(int idx) {
    if (menuActivo() == 0) return subopcionesWifi[idx];
    if (menuActivo() == 1) return subopcionesRfid[idx];
    return subopcionesGenericas[idx];
}

inline void dibujarBoton(TFT_eSPI& tft, int x, int y, int w, int h, const char* texto, bool seleccionado) {
    if (seleccionado) {
        tft.fillRect(x + 1, y + 1, w - 2, h - 2, TFT_WHITE);
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawRect(x, y, w, h, TFT_WHITE);
    FontHelper::drawStringWithSpanish(tft, texto, x + w / 2, y + h / 2, FontHelper::FONT_BOTON);
}

inline void dibujarCampoValor(TFT_eSPI& tft, int x, int y, int w, int h, const char* titulo, const String& valor) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    FontHelper::drawStringWithSpanish(tft, titulo, tft.width() / 2, y - 10, FontHelper::FONT_TEXTO);
    tft.drawRect(x, y, w, h, TFT_WHITE);
    FontHelper::drawStringWithSpanish(tft, valor, tft.width() / 2, y + h / 2, FontHelper::FONT_TEXTO);
}

inline void mostrarWifiMenu(TFT_eSPI& tft) {
    PantallaBase::fondoConBorde(tft);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    FontHelper::drawStringWithSpanish(tft, "WiFi", tft.width() / 2, 30, FontHelper::FONT_TITULO);

    int boxX = 20;
    int boxW = tft.width() - 40;
    int boxH = 26;

    int ssidBoxY = 50;
    dibujarCampoValor(tft, boxX, ssidBoxY, boxW, boxH, "SSID Actual", AppController::obtenerSSID());

    int passBoxY = 95;
    dibujarCampoValor(tft, boxX, passBoxY, boxW, boxH, "Contraseña Actual", AppController::obtenerPassword());

    int btnW = (tft.width() - 30) / 2;
    int btnH = 40;
    int btnGap = 10;
    int btnX1 = 10;
    int btnX2 = btnX1 + btnW + btnGap;
    int btnY2 = tft.height() - btnH - 10;
    int btnY1 = btnY2 - btnH - btnGap;

    dibujarBoton(tft, btnX1, btnY1, btnW, btnH, "Cambiar SSID", subOpcionSeleccionada() == 0);
    dibujarBoton(tft, btnX2, btnY1, btnW, btnH, "Cambiar Clave", subOpcionSeleccionada() == 1);
    dibujarBoton(tft, btnX1, btnY2, btnW, btnH, "Eliminar red", subOpcionSeleccionada() == 2);
    dibujarBoton(tft, btnX2, btnY2, btnW, btnH, "Volver", subOpcionSeleccionada() == 3);

    pintada() = true;
}

// Dibuja el menú principal o submenú
inline void mostrar(TFT_eSPI& tft) {
    if (menuActivo() == -1) {
        PantallaBase::fondoConBorde(tft);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        FontHelper::drawStringWithSpanish(tft, "Menu Administrador", tft.width() / 2, 30, FontHelper::FONT_TITULO);

        int btnX = 10;
        int btnW = tft.width() - 20;
        int btnH = 40;
        int btnGap = 8;
        int startY = 45;

        for (int i = 0; i < NUM_OPCIONES; ++i) {
            int y = startY + i * (btnH + btnGap);
            dibujarBoton(tft, btnX, y, btnW, btnH, opciones[i], i == opcionSeleccionada());
        }
    } else if (menuActivo() == 0) {
        mostrarWifiMenu(tft);
    } else {
        PantallaBase::fondoConBorde(tft);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        // Submenú
        FontHelper::drawStringWithSpanish(tft, opciones[menuActivo()], tft.width()/2, 30, FontHelper::FONT_TITULO);
        int total = subopcionesCount();
        int btnX = 10;
        int btnW = tft.width() - 20;
        int btnH = 40;
        int btnGap = 8;
        int startY = 45;

        for (int i = 0; i < total; ++i) {
            int y = startY + i * (btnH + btnGap);
            dibujarBoton(tft, btnX, y, btnW, btnH, subopcionTexto(i), i == subOpcionSeleccionada());
        }
        // Botón volver
        int y = startY + total * (btnH + btnGap);
        dibujarBoton(tft, btnX, y, btnW, btnH, "Volver", subOpcionSeleccionada() == total);
    }
    pintada() = true;
}

// Selecciona opción del menú principal o submenú
inline void seleccionarOpcion(TFT_eSPI& tft) {
    if (menuActivo() == -1) {
        int sel = opcionSeleccionada();
        if (sel >= 0 && sel <= 2) {
            // Entrar a submenú
            menuActivo() = sel;
            subOpcionSeleccionada() = 0;
            mostrar(tft);
        } else if (sel == 3) {
            // Salir
            menuActivo() = -1;
            opcionSeleccionada() = 0;
            pintada() = false;
            debeVolverAlLogin() = true;
        }
    } else {
        int sel = subOpcionSeleccionada();
        if (menuActivo() == 0) { // Cambiar Wifi
            if (sel == 0) {
                AppController::pedirSSID(tft);
            } else if (sel == 1) {
                AppController::pedirPasswordWifi(tft);
            } else {
                // Volver
                menuActivo() = -1;
                mostrar(tft);
            }
        } else if (menuActivo() == 1) { // Gestion RFID
            if (sel == 0) {
                AppController::gestionarRFIDAgregar(tft);
            } else if (sel == 1) {
                AppController::gestionarRFIDSobrescribir(tft);
            } else if (sel == 2) {
                AppController::gestionarRFIDDesvincular(tft);
            } else {
                menuActivo() = -1;
                mostrar(tft);
            }
        } else {
            // Volver
            menuActivo() = -1;
            mostrar(tft);
        }
    }
}

// Detecta cuál opción fue tocada basándose en coordenadas x, y
inline int detectarOpcionTocada(TFT_eSPI& tft, int x, int y) {
    if (menuActivo() == 0) {
        int btnW = (tft.width() - 30) / 2;
        int btnH = 40;
        int btnGap = 10;
        int btnX1 = 10;
        int btnX2 = btnX1 + btnW + btnGap;
        int btnY2 = tft.height() - btnH - 10;
        int btnY1 = btnY2 - btnH - btnGap;

        if (x >= btnX1 && x <= btnX1 + btnW && y >= btnY1 && y <= btnY1 + btnH) return 0;
        if (x >= btnX2 && x <= btnX2 + btnW && y >= btnY1 && y <= btnY1 + btnH) return 1;
        if (x >= btnX1 && x <= btnX1 + btnW && y >= btnY2 && y <= btnY2 + btnH) return 2;
        if (x >= btnX2 && x <= btnX2 + btnW && y >= btnY2 && y <= btnY2 + btnH) return 3;
        return -1;
    }

    int numOpciones = menuActivo() == -1 ? NUM_OPCIONES : subopcionesCount() + 1;
    int btnH = 40;
    int btnGap = 8;
    int startY = 45;
    int xMin = 10;
    int xMax = tft.width() - 10;

    for (int i = 0; i < numOpciones; ++i) {
        int yMin = startY + i * (btnH + btnGap);
        int yMax = yMin + btnH;
        if (x >= xMin && x <= xMax && y >= yMin && y <= yMax) {
            return i;
        }
    }
    return -1;
}

// Procesa un toque en el menú
inline void procesarToque(TFT_eSPI& tft, int x, int y) {
    int opcionTocada = detectarOpcionTocada(tft, x, y);
    
    if (opcionTocada >= 0) {
        if (menuActivo() == -1) {
            // Menú principal
            opcionSeleccionada() = opcionTocada;
            mostrar(tft);
            delay(100);
            seleccionarOpcion(tft);
        } else if (menuActivo() == 0) {
            subOpcionSeleccionada() = opcionTocada;
            mostrar(tft);
            delay(100);
            int sel = subOpcionSeleccionada();
            if (sel == 0) {
                AppController::pedirSSID(tft);
            } else if (sel == 1) {
                AppController::pedirPasswordWifi(tft);
            } else if (sel == 2) {
                AppController::borrarCredencialesWifi();
                mostrar(tft);
            } else {
                menuActivo() = -1;
                mostrar(tft);
            }
        } else {
            // Submenú
            subOpcionSeleccionada() = opcionTocada;
            mostrar(tft);
            delay(100);
            
            int sel = subOpcionSeleccionada();
            if (menuActivo() == 1) {
                if (sel == 0) {
                    AppController::gestionarRFIDAgregar(tft);
                } else if (sel == 1) {
                    AppController::gestionarRFIDSobrescribir(tft);
                } else if (sel == 2) {
                    AppController::gestionarRFIDDesvincular(tft);
                } else {
                    menuActivo() = -1;
                    mostrar(tft);
                }
            } else {
                menuActivo() = -1;
                mostrar(tft);
            }
        }
    }
}

// Lógica de navegación simple (solo para un botón OK)
// boton: 0=OK, 1=arriba, 2=abajo
inline void actualizar(TFT_eSPI& tft, int boton) {
    if (menuActivo() == -1) {
        if (boton == 0) {
            seleccionarOpcion(tft);
        } else if (boton == 1) {
            opcionSeleccionada() = (opcionSeleccionada() - 1 + NUM_OPCIONES) % NUM_OPCIONES;
            mostrar(tft);
        } else if (boton == 2) {
            opcionSeleccionada() = (opcionSeleccionada() + 1) % NUM_OPCIONES;
            mostrar(tft);
        }
    } else if (menuActivo() == 0) {
        int total = 4;
        if (boton == 0) {
            int sel = subOpcionSeleccionada();
            if (sel == 0) {
                AppController::pedirSSID(tft);
            } else if (sel == 1) {
                AppController::pedirPasswordWifi(tft);
            } else if (sel == 2) {
                AppController::borrarCredencialesWifi();
                mostrar(tft);
            } else {
                menuActivo() = -1;
                mostrar(tft);
            }
        } else if (boton == 1) {
            subOpcionSeleccionada() = (subOpcionSeleccionada() - 1 + total) % total;
            mostrar(tft);
        } else if (boton == 2) {
            subOpcionSeleccionada() = (subOpcionSeleccionada() + 1) % total;
            mostrar(tft);
        }
    } else {
        if (boton == 0) {
            // Seleccionar subopción o volver
            int sel = subOpcionSeleccionada();
            int total = subopcionesCount();
            if (sel < total) {
                seleccionarOpcion(tft);
            } else {
                menuActivo() = -1;
                mostrar(tft);
            }
        } else if (boton == 1) {
            int total = subopcionesCount();
            subOpcionSeleccionada() = (subOpcionSeleccionada() - 1 + total + 1) % (total + 1);
            mostrar(tft);
        } else if (boton == 2) {
            int total = subopcionesCount();
            subOpcionSeleccionada() = (subOpcionSeleccionada() + 1) % (total + 1);
            mostrar(tft);
        }
    }
}

} // namespace MenuAdministrador
