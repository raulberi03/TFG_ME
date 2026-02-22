#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "pantallas/PantallaBase.h"
#include "pantallas/PantallaAlfanumerica.h"
#include "pantallas/MenuAdministrador.h"
#include "pantallas/FontHelper.h"

namespace WiFiController {
    namespace {
        String ssidDeTienda = "";
        String passwordDeTienda = "";
        Preferences prefs;

        void guardarCredenciales() {
            prefs.begin("wifi", false);
            prefs.putString("ssid", ssidDeTienda);
            prefs.putString("pass", passwordDeTienda);
            prefs.end();
        }

        void mostrarConectando(TFT_eSPI& tft) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Conectando...", tft.width() / 2, tft.height() / 2, FontHelper::FONT_TITULO);
        }

        void mostrarResultadoConexion(TFT_eSPI& tft, bool ok) {
            PantallaBase::fondoConBorde(tft);
            if (ok) {
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                FontHelper::drawStringWithSpanish(tft, "Conexion correcta", tft.width() / 2, tft.height() / 2, FontHelper::FONT_TITULO);
            } else {
                tft.setTextColor(TFT_RED, TFT_BLACK);
                FontHelper::drawStringWithSpanish(tft, "Conexion fallida", tft.width() / 2, tft.height() / 2, FontHelper::FONT_TITULO);
            }
        }

        bool conectarInterno(TFT_eSPI* tft, bool mostrarPantallas) {
            if (ssidDeTienda.length() == 0) {
                if (mostrarPantallas && tft) {
                    mostrarResultadoConexion(*tft, false);
                    delay(1200);
                }
                return false;
            }

            if (mostrarPantallas && tft) {
                mostrarConectando(*tft);
            }

            WiFi.mode(WIFI_STA);
            WiFi.disconnect(true);
            delay(100);
            WiFi.begin(ssidDeTienda.c_str(), passwordDeTienda.c_str());

            const uint32_t timeoutMs = 15000;
            uint32_t inicio = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - inicio) < timeoutMs) {
                delay(200);
            }

            bool ok = (WiFi.status() == WL_CONNECTED);
            if (mostrarPantallas && tft) {
                mostrarResultadoConexion(*tft, ok);
                delay(1200);
            }
            return ok;
        }
        
        void mostrarMensajeSSSID(TFT_eSPI& tft) {
            delay(500);
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "SSID Guardado", tft.width()/2, tft.height()/2, FontHelper::FONT_BOTON);
            delay(1500);
        }
        
        void mostrarMensajePassword(TFT_eSPI& tft) {
            delay(500);
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Contraseña Guardada", tft.width()/2, tft.height()/2, FontHelper::FONT_BOTON);
            delay(1500);
        }
        
        void onSSIDOk(TFT_eSPI& tft, const String& valor) {
            ssidDeTienda = valor;
            guardarCredenciales();
            PantallaAlfanumerica::pintada() = false;
            mostrarMensajeSSSID(tft);
            MenuAdministrador::mostrar(tft);
        }

        void onSSIDBack(TFT_eSPI& tft) {
            PantallaAlfanumerica::pintada() = false;
            MenuAdministrador::mostrar(tft);
        }
        
        void onSSIDClr(TFT_eSPI& tft, String& v) {
            v = "";
            PantallaAlfanumerica::refrescarCajaValor(tft);
        }
        
        void onPasswordOk(TFT_eSPI& tft, const String& valor) {
            passwordDeTienda = valor;
            guardarCredenciales();
            PantallaAlfanumerica::pintada() = false;
            mostrarMensajePassword(tft);
            MenuAdministrador::mostrar(tft);
        }

        void onPasswordBack(TFT_eSPI& tft) {
            PantallaAlfanumerica::pintada() = false;
            MenuAdministrador::mostrar(tft);
        }
        
        void onPasswordClr(TFT_eSPI& tft, String& v) {
            v = "";
            PantallaAlfanumerica::refrescarCajaValor(tft);
        }
    }
    
    // Solicita el SSID del WiFi usando teclado alfanumérico
    inline void pedirSSID(TFT_eSPI& tft) {
        MenuAdministrador::pintada() = false;
        PantallaAlfanumerica::Config cfg;
        cfg.textoSuperior = "SSID WiFi";
        cfg.maxLen = 32;
        cfg.onOk = onSSIDOk;
        cfg.onClr = onSSIDClr;
        cfg.onBack = onSSIDBack;
        PantallaAlfanumerica::mostrar(tft, cfg);
    }

    // Solicita la contraseña del WiFi usando teclado alfanumérico
    inline void pedirPasswordWifi(TFT_eSPI& tft) {
        MenuAdministrador::pintada() = false;
        PantallaAlfanumerica::Config cfg;
        cfg.textoSuperior = "Contraseña WiFi";
        cfg.maxLen = 32;
        cfg.onOk = onPasswordOk;
        cfg.onClr = onPasswordClr;
        cfg.onBack = onPasswordBack;
        PantallaAlfanumerica::mostrar(tft, cfg);
    }
    
    // Retorna el SSID ingresado
    inline const String& obtenerSSID() {
        return ssidDeTienda;
    }
    
    // Retorna la contraseña ingresada
    inline const String& obtenerPassword() {
        return passwordDeTienda;
    }

    inline void borrarCredenciales() {
        ssidDeTienda = "";
        passwordDeTienda = "";
        prefs.begin("wifi", false);
        prefs.remove("ssid");
        prefs.remove("pass");
        prefs.end();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    inline void cargarCredenciales() {
        prefs.begin("wifi", true);
        ssidDeTienda = prefs.getString("ssid", "");
        passwordDeTienda = prefs.getString("pass", "");
        prefs.end();
    }

    inline bool conectarConPantalla(TFT_eSPI& tft) {
        return conectarInterno(&tft, true);
    }

    inline bool conectarSilencioso() {
        return conectarInterno(nullptr, false);
    }
}
