#pragma once
#include <TFT_eSPI.h>
#include "PantallaBase.h"

// Forward declarations
void pedirSSID(TFT_eSPI& tft);
void pedirPasswordWifi(TFT_eSPI& tft);

namespace MenuAdministrador {

// Estado del menú
inline bool& pintada() { static bool p = false; return p; }
inline bool& debeVolverAlLogin() { static bool v = false; return v; }
inline int& opcionSeleccionada() { static int o = 0; return o; }


// Opciones principales del menú de administrador
constexpr int NUM_OPCIONES = 5;
const char* opciones[NUM_OPCIONES] = {
    "Cambiar Wifi",
    "Gestion Usuarios",
    "Gestion RFID",
    "Gestion Huella",
    "Salir"
};

// Submenús para cada gestión
constexpr int NUM_SUBOPCIONES = 2;
const char* subopciones[NUM_SUBOPCIONES] = { "SSID", "Contraseña" };
inline int& subOpcionSeleccionada() { static int s = 0; return s; }
inline int& menuActivo() { static int m = -1; return m; } // -1: menú principal, 0-3: submenú

// Dibuja el menú de administrador
// Dibuja el menú principal o submenú
inline void mostrar(TFT_eSPI& tft) {
    PantallaBase::fondoConBorde(tft);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    if (menuActivo() == -1) {
        tft.drawString("Menu Administrador", tft.width()/2, 30);
        for (int i = 0; i < NUM_OPCIONES; ++i) {
            int y = 80 + i * 40;
            // Dibuja borde y fondo
            tft.drawRect(30, y-10, tft.width()-60, 35, TFT_WHITE);
            if (i == opcionSeleccionada()) {
                tft.fillRect(31, y-9, tft.width()-62, 33, TFT_WHITE);
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            tft.drawString(opciones[i], tft.width()/2, y);
        }
    } else {
        // Submenú
        tft.drawString(opciones[menuActivo()], tft.width()/2, 30);
        for (int i = 0; i < NUM_SUBOPCIONES; ++i) {
            int y = 80 + i * 40;
            tft.drawRect(30, y-10, tft.width()-60, 35, TFT_WHITE);
            if (i == subOpcionSeleccionada()) {
                tft.fillRect(31, y-9, tft.width()-62, 33, TFT_WHITE);
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            tft.drawString(subopciones[i], tft.width()/2, y);
        }
        // Botón volver
        int y = 80 + NUM_SUBOPCIONES * 40;
        tft.drawRect(30, y-10, tft.width()-60, 35, TFT_WHITE);
        if (subOpcionSeleccionada() == MenuAdministrador::NUM_SUBOPCIONES) {
            tft.fillRect(31, y-9, tft.width()-62, 33, TFT_WHITE);
            tft.setTextColor(TFT_BLACK, TFT_WHITE);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        tft.drawString("Volver", tft.width()/2, y);
    }
    pintada() = true;
}


// Forward declaration para evitar error de declaración
inline void mostrarPantallaFutura(TFT_eSPI& tft, const char* opcion);

// Procesa la pulsación de una opción
// Selecciona opción del menú principal o submenú
inline void seleccionarOpcion(TFT_eSPI& tft) {
    if (menuActivo() == -1) {
        int sel = opcionSeleccionada();
        if (sel >= 0 && sel <= 3) {
            // Entrar a submenú
            menuActivo() = sel;
            subOpcionSeleccionada() = 0;
            mostrar(tft);
        } else if (sel == 4) {
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
                pedirSSID(tft);
            } else if (sel == 1) {
                pedirPasswordWifi(tft);
            } else {
                // Volver
                menuActivo() = -1;
                mostrar(tft);
            }
        } else if (sel == 0 || sel == 1) {
            mostrarPantallaFutura(tft, subopciones[sel]);
        } else {
            // Volver
            menuActivo() = -1;
            mostrar(tft);
        }
    }
}

// Muestra pantalla de "futura implementación"
inline void mostrarPantallaFutura(TFT_eSPI& tft, const char* opcion) {
    PantallaBase::fondoConBorde(tft);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString("Futura implementacion", tft.width()/2, tft.height()/2 - 20);
    tft.drawString(opcion, tft.width()/2, tft.height()/2 + 20);
    delay(2000);
    mostrar(tft); // Volver al menú
}

// Detecta cuál opción fue tocada basándose en coordenadas x, y
inline int detectarOpcionTocada(TFT_eSPI& tft, int x, int y) {
    int numOpciones = menuActivo() == -1 ? NUM_OPCIONES : NUM_SUBOPCIONES + 1;
    
    for (int i = 0; i < numOpciones; ++i) {
        int yOpcion = 80 + i * 40;
        int yMin = yOpcion - 10;
        int yMax = yOpcion + 25;
        int xMin = 30;
        int xMax = tft.width() - 30;
        
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
        } else {
            // Submenú
            subOpcionSeleccionada() = opcionTocada;
            mostrar(tft);
            delay(100);
            
            int sel = subOpcionSeleccionada();
            if (menuActivo() == 0) {
                if (sel == 0) {
                    pedirSSID(tft);
                } else if (sel == 1) {
                    pedirPasswordWifi(tft);
                } else {
                    menuActivo() = -1;
                    mostrar(tft);
                }
            } else if (sel == 0 || sel == 1) {
                mostrarPantallaFutura(tft, subopciones[sel]);
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
    } else {
        if (boton == 0) {
            // Seleccionar subopción o volver
            int sel = subOpcionSeleccionada();
            if (sel == 0 || sel == 1) {
                seleccionarOpcion(tft);
            } else {
                // Volver
                menuActivo() = -1;
                mostrar(tft);
            }
        } else if (boton == 1) {
            subOpcionSeleccionada() = (subOpcionSeleccionada() - 1 + NUM_SUBOPCIONES + 1) % (NUM_SUBOPCIONES + 1);
            mostrar(tft);
        } else if (boton == 2) {
            subOpcionSeleccionada() = (subOpcionSeleccionada() + 1) % (NUM_SUBOPCIONES + 1);
            mostrar(tft);
        }
    }
}

} // namespace
