#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>
#include "pantallas/PantallaBase.h"
#include "pantallas/PantallaAlfanumerica.h"
#include "pantallas/MenuAdministrador.h"
#include "pantallas/FontHelper.h"

namespace WiFiController {
    namespace {
        String ssidDeTienda = "";
        String passwordDeTienda = "";
        
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
    inline String obtenerSSID() {
        return ssidDeTienda;
    }
    
    // Retorna la contraseña ingresada
    inline String obtenerPassword() {
        return passwordDeTienda;
    }

    inline void borrarCredenciales() {
        ssidDeTienda = "";
        passwordDeTienda = "";
    }
}
