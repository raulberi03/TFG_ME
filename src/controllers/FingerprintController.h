#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>
#include "services/FingerprintService.h"
#include "controllers/LoginController.h"
#include "pantallas/PantallaBase.h"
#include "pantallas/PantallaNumerica.h"
#include "pantallas/MenuAdministrador.h"
#include "pantallas/FontHelper.h"

namespace FingerprintController {
    namespace {
        FingerprintService* g_fingerprint = nullptr;

        enum class HuellaOp { None, Agregar, Sobrescribir, Desvincular, LimpiarTodo };
        HuellaOp huellaOpPendiente = HuellaOp::None;
        String huellaUsuarioPendiente = "";
        bool huellaEsperandoDedo = false;

        void onUsuarioOk(TFT_eSPI& tft, const String& usuarioId);
        void onUsuarioClr(TFT_eSPI& tft, String& valor);

        void mostrarMensaje(TFT_eSPI& tft, const char* linea1, const String& linea2, uint16_t color) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(color, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, linea1, tft.width()/2, tft.height()/2 - 20, FontHelper::FONT_BOTON);
            if (linea2.length() > 0) {
                FontHelper::drawStringWithSpanish(tft, linea2, tft.width()/2, tft.height()/2 + 20, FontHelper::FONT_BOTON);
            }
            delay(1800);
        }

        void mostrarPromptHuella(TFT_eSPI& tft) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Coloque el dedo", tft.width()/2, tft.height()/2 - 18, FontHelper::FONT_BOTON);
            FontHelper::drawStringWithSpanish(tft, "1/2", tft.width()/2, tft.height()/2 + 18, FontHelper::FONT_BOTON);
        }

        void mostrarComprobando(TFT_eSPI& tft) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Autenticando", tft.width()/2, tft.height()/2 - 20, FontHelper::FONT_BOTON);
            FontHelper::drawStringWithSpanish(tft, "espere...", tft.width()/2, tft.height()/2 + 20, FontHelper::FONT_BOTON);
        }

        void mostrarProcesoAdmin(TFT_eSPI& tft) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Procesando huella", tft.width()/2, tft.height()/2 - 20, FontHelper::FONT_BOTON);
            FontHelper::drawStringWithSpanish(tft, "espere...", tft.width()/2, tft.height()/2 + 20, FontHelper::FONT_BOTON);
        }

        void mostrarPasoCaptura(void* contexto, const char* linea1, const char* linea2) {
            TFT_eSPI* tft = static_cast<TFT_eSPI*>(contexto);
            if (tft == nullptr) {
                return;
            }

            PantallaBase::fondoConBorde(*tft);
            tft->setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(*tft, linea1, tft->width()/2, tft->height()/2 - 18, FontHelper::FONT_BOTON);
            if (linea2 != nullptr && strlen(linea2) > 0) {
                FontHelper::drawStringWithSpanish(*tft, linea2, tft->width()/2, tft->height()/2 + 18, FontHelper::FONT_BOTON);
            }
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
            huellaUsuarioPendiente = usuarioId;
            PantallaNumerica::pintada() = false;

            if (huellaOpPendiente == HuellaOp::Desvincular) {
                mostrarComprobando(tft);
                bool ok = g_fingerprint && g_fingerprint->eliminarHuella(huellaUsuarioPendiente);
                if (!ok && g_fingerprint && g_fingerprint->ultimoMensaje().length() > 0) {
                    Serial.print("[R503] Error eliminar huella: ");
                    Serial.println(g_fingerprint->ultimoMensaje());
                }
                if (g_fingerprint) {
                    ok ? g_fingerprint->indicarExito() : g_fingerprint->indicarError();
                    g_fingerprint->activarModoLogin();
                }

                mostrarMensaje(tft, ok ? "Huella eliminada" : "Error servidor", "", ok ? TFT_GREEN : TFT_RED);
                MenuAdministrador::mostrar(tft);
                huellaOpPendiente = HuellaOp::None;
                huellaEsperandoDedo = false;
                return;
            }

            mostrarPromptHuella(tft);
            huellaEsperandoDedo = true;
        }

        void onUsuarioClr(TFT_eSPI& tft, String& valor) {
            valor = "";
            PantallaNumerica::refrescarCajaValor(tft);
        }
    }

    inline void begin(FingerprintService& fingerprint) {
        g_fingerprint = &fingerprint;
    }

    inline void startAdd(TFT_eSPI& tft) {
        huellaOpPendiente = HuellaOp::Agregar;
        if (g_fingerprint) g_fingerprint->activarModoOperacion();
        mostrarPromptId(tft);
    }

    inline void startOverwrite(TFT_eSPI& tft) {
        huellaOpPendiente = HuellaOp::Sobrescribir;
        if (g_fingerprint) g_fingerprint->activarModoOperacion();
        mostrarPromptId(tft);
    }

    inline void startUnlink(TFT_eSPI& tft) {
        huellaOpPendiente = HuellaOp::Desvincular;
        if (g_fingerprint) g_fingerprint->activarModoOperacion();
        mostrarPromptId(tft);
    }

    inline void startClearAll(TFT_eSPI& tft) {
        huellaOpPendiente = HuellaOp::LimpiarTodo;
        if (g_fingerprint) g_fingerprint->activarModoOperacion();
        mostrarComprobando(tft);
        bool ok = g_fingerprint && g_fingerprint->limpiarLector();
        if (!ok && g_fingerprint && g_fingerprint->ultimoMensaje().length() > 0) {
            Serial.print("[R503] Error limpiar lector: ");
            Serial.println(g_fingerprint->ultimoMensaje());
        }
        if (g_fingerprint) {
            ok ? g_fingerprint->indicarExito() : g_fingerprint->indicarError();
            g_fingerprint->activarModoLogin();
        }
        mostrarMensaje(tft, ok ? "Lector vaciado" : "Error lector", "", ok ? TFT_GREEN : TFT_RED);
        MenuAdministrador::mostrar(tft);
        huellaOpPendiente = HuellaOp::None;
        huellaEsperandoDedo = false;
    }

    inline bool handleCapturedFingerprint(TFT_eSPI& tft) {
        if (!huellaEsperandoDedo) {
            return false;
        }

        bool ok = false;
        if (huellaOpPendiente == HuellaOp::Agregar) {
            ok = g_fingerprint && g_fingerprint->agregarHuella(huellaUsuarioPendiente, mostrarPasoCaptura, &tft);
        } else if (huellaOpPendiente == HuellaOp::Sobrescribir) {
            ok = g_fingerprint && g_fingerprint->sobrescribirHuella(huellaUsuarioPendiente, mostrarPasoCaptura, &tft);
        }

        const char* mensajeOk = huellaOpPendiente == HuellaOp::Agregar ? "Huella agregada" : "Huella actualizada";
        if (!ok && g_fingerprint && g_fingerprint->ultimoMensaje().length() > 0) {
            const char* operacion = huellaOpPendiente == HuellaOp::Agregar ? "agregar huella" : "sobrescribir huella";
            Serial.print("[R503] Error ");
            Serial.print(operacion);
            Serial.print(": ");
            Serial.println(g_fingerprint->ultimoMensaje());
        }
        if (g_fingerprint) {
            ok ? g_fingerprint->indicarExito() : g_fingerprint->indicarError();
            g_fingerprint->activarModoLogin();
        }

        mostrarMensaje(tft, ok ? mensajeOk : "Error servidor", "", ok ? TFT_GREEN : TFT_RED);
        MenuAdministrador::mostrar(tft);
        huellaOpPendiente = HuellaOp::None;
        huellaEsperandoDedo = false;
        return true;
    }

    inline void mostrarComprobandoHuella(TFT_eSPI& tft) {
        mostrarComprobando(tft);
    }

    inline void handleFingerprintLogin(TFT_eSPI& tft, bool valido) {
        LoginController::handleHuellaLogin(tft, valido);
    }

    inline bool hayOperacionPendiente() {
        return huellaOpPendiente != HuellaOp::None;
    }
}
