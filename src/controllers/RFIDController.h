#pragma once
#include <TFT_eSPI.h>
#include "services/RFIDUsuariosService.h"
#include <Arduino.h>
#include "pantallas/PantallaBase.h"
#include "pantallas/PantallaNumerica.h"
#include "pantallas/MenuAdministrador.h"
#include "pantallas/FontHelper.h"

namespace RFIDController {
    namespace {
        RFIDUsuariosService* g_rfidUsuarios = nullptr;

        enum class RfidOp { None, Agregar, Sobrescribir, Desvincular };
        RfidOp rfidOpPendiente = RfidOp::None;
        String rfidUsuarioPendiente = "";
        bool rfidEsperandoTarjeta = false;

        void onUsuarioOk(TFT_eSPI& tft, const String& usuarioId);
        void onUsuarioClr(TFT_eSPI& tft, String& v);

        void mostrarMensaje(TFT_eSPI& tft, const char* linea1, const char* linea2, uint16_t color) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(color, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, linea1, tft.width()/2, tft.height()/2 - 20, FontHelper::FONT_BOTON);
            if (linea2 && linea2[0] != '\0') {
                FontHelper::drawStringWithSpanish(tft, linea2, tft.width()/2, tft.height()/2 + 20, FontHelper::FONT_BOTON);
            }
            delay(1200);
        }

        void mostrarPantallaEscaneo(TFT_eSPI& tft) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Escanee tarjeta", tft.width()/2, tft.height()/2, FontHelper::FONT_BOTON);
        }

        void mostrarPromptId(TFT_eSPI& tft) {
            MenuAdministrador::pintada() = false;
            PantallaNumerica::Config cfg;
            cfg.textoSuperior = "ID Usuario";
            cfg.maxLen = 8;
            cfg.onOk = onUsuarioOk;
            cfg.onClr = onUsuarioClr;
            PantallaNumerica::mostrar(tft, cfg);
        }

        void onUsuarioOk(TFT_eSPI& tft, const String& usuarioId) {
            rfidUsuarioPendiente = usuarioId;
            PantallaNumerica::pintada() = false;

            if (rfidOpPendiente == RfidOp::Desvincular) {
                bool ok = g_rfidUsuarios && g_rfidUsuarios->desvincular(rfidUsuarioPendiente);
                mostrarMensaje(tft, ok ? "RFID desvinculado" : "No encontrado", "", ok ? TFT_GREEN : TFT_RED);
                MenuAdministrador::mostrar(tft);
                rfidOpPendiente = RfidOp::None;
                return;
            }

            if (rfidOpPendiente == RfidOp::Agregar) {
                if (g_rfidUsuarios && g_rfidUsuarios->tieneUsuario(rfidUsuarioPendiente)) {
                    mostrarMensaje(tft, "Usuario ya tiene", "RFID", TFT_RED);
                    MenuAdministrador::mostrar(tft);
                    rfidOpPendiente = RfidOp::None;
                    return;
                }
            }

            if (rfidOpPendiente == RfidOp::Sobrescribir) {
                if (!g_rfidUsuarios || !g_rfidUsuarios->tieneUsuario(rfidUsuarioPendiente)) {
                    mostrarMensaje(tft, "Usuario sin", "RFID", TFT_RED);
                    MenuAdministrador::mostrar(tft);
                    rfidOpPendiente = RfidOp::None;
                    return;
                }
            }

            mostrarPantallaEscaneo(tft);
            rfidEsperandoTarjeta = true;
        }

        void onUsuarioClr(TFT_eSPI& tft, String& v) {
            v = "";
            PantallaNumerica::refrescarCajaValor(tft);
        }
    }

    // Inyecta el servicio de usuarios RFID.
    inline void begin(RFIDUsuariosService& rfidUsuarios) {
        g_rfidUsuarios = &rfidUsuarios;
    }

    // Inicia los flujos desde el menu de administrador.
    inline void startAdd(TFT_eSPI& tft) {
        rfidOpPendiente = RfidOp::Agregar;
        mostrarPromptId(tft);
    }

    inline void startOverwrite(TFT_eSPI& tft) {
        rfidOpPendiente = RfidOp::Sobrescribir;
        mostrarPromptId(tft);
    }

    inline void startUnlink(TFT_eSPI& tft) {
        rfidOpPendiente = RfidOp::Desvincular;
        mostrarPromptId(tft);
    }

    // Procesa una tarjeta detectada si hay una operacion pendiente.
    // Devuelve true si la tarjeta se consume por el flujo RFID.
    inline bool handleCard(TFT_eSPI& tft, const String& uid) {
        if (!rfidEsperandoTarjeta) {
            return false;
        }

        bool ok = false;
        if (rfidOpPendiente == RfidOp::Agregar) {
            ok = g_rfidUsuarios && g_rfidUsuarios->agregarSiNoTiene(rfidUsuarioPendiente, uid);
        } else if (rfidOpPendiente == RfidOp::Sobrescribir) {
            ok = g_rfidUsuarios && g_rfidUsuarios->sobrescribir(rfidUsuarioPendiente, uid);
        }

        mostrarMensaje(tft, ok ? "RFID vinculado" : "Error al vincular", "", ok ? TFT_GREEN : TFT_RED);
        MenuAdministrador::mostrar(tft);
        rfidOpPendiente = RfidOp::None;
        rfidEsperandoTarjeta = false;
        return true;
    }
}
