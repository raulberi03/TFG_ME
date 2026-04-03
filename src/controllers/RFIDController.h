#pragma once
#include <TFT_eSPI.h>
#include "services/RFIDService.h"
#include <Arduino.h>
#include "pantallas/PantallaBase.h"
#include "pantallas/PantallaNumerica.h"
#include "pantallas/MenuAdministrador.h"
#include "pantallas/FontHelper.h"

namespace RFIDController {
    namespace {
        RFIDService* g_rfid = nullptr;

        enum class RfidOp { None, Agregar, Sobrescribir, Desvincular };
        RfidOp rfidOpPendiente = RfidOp::None;
        String rfidUsuarioPendiente = "";
        bool rfidEsperandoTarjeta = false;

        void onUsuarioOk(TFT_eSPI& tft, const String& usuarioId);
        void onUsuarioClr(TFT_eSPI& tft, String& v);

        void mostrarMensaje(TFT_eSPI& tft, const char* linea1, const String& linea2, uint16_t color) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(color, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, linea1, tft.width()/2, tft.height()/2 - 20, FontHelper::FONT_BOTON);
            if (linea2.length() > 0) {
                FontHelper::drawStringWithSpanish(tft, linea2, tft.width()/2, tft.height()/2 + 20, FontHelper::FONT_BOTON);
            }
            delay(1800);
        }

        void mostrarPantallaEscaneo(TFT_eSPI& tft) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Escanee tarjeta", tft.width()/2, tft.height()/2, FontHelper::FONT_BOTON);
        }

        void mostrarComprobando(TFT_eSPI& tft) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Comprobando", tft.width()/2, tft.height()/2 - 20, FontHelper::FONT_BOTON);
            FontHelper::drawStringWithSpanish(tft, "espere...", tft.width()/2, tft.height()/2 + 20, FontHelper::FONT_BOTON);
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
                mostrarComprobando(tft);
                bool ok = g_rfid && g_rfid->eliminarTarjeta(rfidUsuarioPendiente);
                String detalle = "";
                if (!ok && g_rfid) {
                    detalle = g_rfid->ultimoMensaje();
                }

                mostrarMensaje(tft, ok ? "RFID eliminado" : "Error servidor RFID", detalle, ok ? TFT_GREEN : TFT_RED);
                MenuAdministrador::mostrar(tft);
                rfidOpPendiente = RfidOp::None;
                rfidEsperandoTarjeta = false;
                return;
            }

            mostrarPantallaEscaneo(tft);
            rfidEsperandoTarjeta = true;
        }

        void onUsuarioClr(TFT_eSPI& tft, String& v) {
            v = "";
            PantallaNumerica::refrescarCajaValor(tft);
        }
    }

    // Inyecta el servicio RFID que habla con la API.
    inline void begin(RFIDService& rfid) {
        g_rfid = &rfid;
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

        mostrarComprobando(tft);

        bool ok = false;
        if (rfidOpPendiente == RfidOp::Agregar) {
            ok = g_rfid && g_rfid->agregarTarjeta(uid, rfidUsuarioPendiente);
        } else if (rfidOpPendiente == RfidOp::Sobrescribir) {
            ok = g_rfid && g_rfid->sobrescribirTarjeta(uid, rfidUsuarioPendiente);
        }

        const char* mensajeOk = "Operacion RFID OK";
        if (rfidOpPendiente == RfidOp::Agregar) {
            mensajeOk = "RFID agregado";
        } else if (rfidOpPendiente == RfidOp::Sobrescribir) {
            mensajeOk = "RFID actualizado";
        } else if (rfidOpPendiente == RfidOp::Desvincular) {
            mensajeOk = "RFID eliminado";
        }

        String detalle = "";
        if (!ok && g_rfid) {
            detalle = g_rfid->ultimoMensaje();
        }

        mostrarMensaje(tft, ok ? mensajeOk : "Error servidor RFID", detalle, ok ? TFT_GREEN : TFT_RED);
        MenuAdministrador::mostrar(tft);
        rfidOpPendiente = RfidOp::None;
        rfidEsperandoTarjeta = false;
        return true;
    }

    inline void mostrarComprobandoTarjeta(TFT_eSPI& tft) {
        mostrarComprobando(tft);
    }
}
