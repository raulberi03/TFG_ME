#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>
#include "config/AppConfig.h"
#include "pantallas/PantallaNumerica.h"
#include "pantallas/MenuAdministrador.h"
#include "pantallas/FontHelper.h"

namespace LoginController {
    namespace {
        const char* kAdminUser = AppConfig::Login::AdminUser;
        const char* kAdminPass = AppConfig::Login::AdminPass;
        String ultimoUsuario = "";

        void pedirUsuario(TFT_eSPI& tft);
        void pedirPassword(TFT_eSPI& tft);
        void mostrarLoginOk(TFT_eSPI& tft, bool abrirMenuAdmin);
        void mostrarLoginFail(TFT_eSPI& tft, const String& detalle = "");
        void mostrarAccesoRfid(TFT_eSPI& tft, bool autorizado);
        void onUsuarioOk(TFT_eSPI& tft, const String& usuario);
        void onUsuarioClr(TFT_eSPI& tft, String& v);
        void onPasswordOk(TFT_eSPI& tft, const String& pass);
        void onPasswordClr(TFT_eSPI& tft, String& v);
        void mostrarLoginHuellaOk(TFT_eSPI& tft);

        void mostrarPromptNumerico(TFT_eSPI& tft, const char* titulo,
                                   PantallaNumerica::OnOkCallback onOk,
                                   PantallaNumerica::OnClrCallback onClr) {
            PantallaNumerica::Config cfg;
            cfg.textoSuperior = titulo;
            cfg.maxLen = 8;
            cfg.onOk = onOk;
            cfg.onClr = onClr;
            PantallaNumerica::mostrar(tft, cfg);
        }

        void resetMenu() {
            MenuAdministrador::pintada() = false;
            MenuAdministrador::menuActivo() = -1;
            MenuAdministrador::opcionSeleccionada() = 0;
            MenuAdministrador::subOpcionSeleccionada() = 0;
        }

        void pedirUsuario(TFT_eSPI& tft) {
            mostrarPromptNumerico(tft, "Codigo Usuario", onUsuarioOk, onUsuarioClr);
        }

        void pedirPassword(TFT_eSPI& tft) {
            mostrarPromptNumerico(tft, "Contraseña", onPasswordOk, onPasswordClr);
        }

        void onUsuarioOk(TFT_eSPI& tft, const String& usuario) {
            ultimoUsuario = usuario;
            pedirPassword(tft);
        }

        void onUsuarioClr(TFT_eSPI& tft, String& v) {
            v = "";
            PantallaNumerica::refrescarCajaValor(tft);
        }

        void onPasswordOk(TFT_eSPI& tft, const String& pass) {
            bool esAdmin = (ultimoUsuario == kAdminUser && pass == kAdminPass);
            mostrarLoginOk(tft, esAdmin);
        }

        void onPasswordClr(TFT_eSPI& tft, String& v) {
            v = "";
            PantallaNumerica::refrescarCajaValor(tft);
        }

        void mostrarLoginOk(TFT_eSPI& tft, bool abrirMenuAdmin) {
            tft.fillScreen(TFT_GREEN);
            tft.setTextColor(TFT_BLACK, TFT_GREEN);
            FontHelper::drawStringWithSpanish(tft, "Login OK", tft.width()/2, tft.height()/2, FontHelper::FONT_TITULO);
            delay(1000);
            PantallaNumerica::pintada() = false;
            if (abrirMenuAdmin) {
                MenuAdministrador::mostrar(tft);
            } else {
                resetMenu();
                pedirUsuario(tft);
            }
        }

        void mostrarLoginFail(TFT_eSPI& tft, const String& detalle) {
            tft.fillScreen(TFT_RED);
            tft.setTextColor(TFT_WHITE, TFT_RED);
            FontHelper::drawStringWithSpanish(tft, "Login FAIL", tft.width()/2, tft.height()/2 - 20, FontHelper::FONT_TITULO);
            if (detalle.length() > 0) {
                FontHelper::drawStringWithSpanish(tft, detalle, tft.width()/2, tft.height()/2 + 24, FontHelper::FONT_TEXTO);
            }
            delay(1800);
            resetMenu();
            pedirUsuario(tft);
        }

        void mostrarLoginHuellaOk(TFT_eSPI& tft) {
            tft.fillScreen(TFT_GREEN);
            tft.setTextColor(TFT_BLACK, TFT_GREEN);
            FontHelper::drawStringWithSpanish(tft, "Login ON", tft.width()/2, tft.height()/2, FontHelper::FONT_TITULO);
            delay(1000);
            resetMenu();
            pedirUsuario(tft);
        }

        void mostrarAccesoRfid(TFT_eSPI& tft, bool autorizado) {
            tft.fillScreen(autorizado ? TFT_GREEN : TFT_RED);
            tft.setTextColor(autorizado ? TFT_BLACK : TFT_WHITE, autorizado ? TFT_GREEN : TFT_RED);
            FontHelper::drawStringWithSpanish(
                tft,
                autorizado ? "Acceso autorizado" : "Acceso rechazado",
                tft.width()/2,
                tft.height()/2,
                FontHelper::FONT_TITULO
            );
            delay(1200);
            resetMenu();
            pedirUsuario(tft);
        }
    }

    // Inicia el flujo de login por teclado.
    inline void begin(TFT_eSPI& tft) {
        pedirUsuario(tft);
    }

    // Procesa un login por RFID (solo muestra OK/FAIL, nunca abre menu).
    inline void handleRfidLogin(TFT_eSPI& tft, bool valido, const String& detalle = "") {
        if (!valido && detalle.length() > 0) {
            Serial.print("[RFID] Login rechazado: ");
            Serial.println(detalle);
        }
        mostrarAccesoRfid(tft, valido);
    }

    inline void handleHuellaLogin(TFT_eSPI& tft, bool valido) {
        if (valido) {
            mostrarLoginHuellaOk(tft);
        } else {
            mostrarLoginFail(tft);
        }
    }

    inline bool permiteLoginHuella() {
        return PantallaNumerica::pintada()
            && MenuAdministrador::menuActivo() == -1
            && strcmp(PantallaNumerica::config().textoSuperior, "Codigo Usuario") == 0;
    }
}
