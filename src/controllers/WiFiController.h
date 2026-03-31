#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include "pantallas/PantallaBase.h"
#include "pantallas/PantallaAlfanumerica.h"
#include "pantallas/MenuAdministrador.h"
#include "pantallas/FontHelper.h"

namespace WiFiController {
    namespace {
        constexpr char kServidorPingUrl[] = "http://10.140.46.168:5286/api/Conexion/ping";
        constexpr uint16_t kServidorTimeoutMs = 5000;
        String ssidDeTienda = "";
        String passwordDeTienda = "";
        Preferences prefs;

        // Escanea redes con reintentos, vuelca todo por serie y devuelve true si encuentra el SSID objetivo.
        // Llama siempre antes de WiFi.begin() para que el radio haya visto la red.
        bool escanearRedesYVerificar() {
            constexpr int MAX_INTENTOS = 3;
            int n = WIFI_SCAN_FAILED;

            for (int intento = 1; intento <= MAX_INTENTOS; intento++) {
                Serial.print("[WiFi] Escaneando redes (intento ");
                Serial.print(intento);
                Serial.println(")...");
                WiFi.scanDelete();
                n = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);
                if (n >= 0) {
                    break; // Escaneo exitoso
                }
                Serial.println("[WiFi] Escaneo fallido, reintentando...");
                delay(500);
            }

            if (n < 0) {
                Serial.println("[WiFi] Escaneo fallido tras todos los intentos");
                WiFi.scanDelete();
                return false;
            }

            Serial.print("[WiFi] Redes encontradas: ");
            Serial.println(n);
            bool encontrado = false;
            for (int i = 0; i < n; i++) {
                String ssid = WiFi.SSID(i);
                int rssi = WiFi.RSSI(i);
                Serial.print("  [" + String(i) + "] ");
                Serial.print(ssid);
                Serial.print(" (RSSI: ");
                Serial.print(rssi);
                Serial.println(")");
                if (ssid == ssidDeTienda) {
                    encontrado = true;
                }
            }
            WiFi.scanDelete();
            if (!encontrado) {
                Serial.print("[WiFi] SSID '");
                Serial.print(ssidDeTienda);
                Serial.println("' NO encontrado en el escaneo");
            } else {
                Serial.print("[WiFi] SSID '");
                Serial.print(ssidDeTienda);
                Serial.println("' encontrado, conectando...");
            }
            return encontrado;
        }

        const char* wifiStatusToString(wl_status_t status) {
            switch (status) {
                case WL_NO_SHIELD: return "WL_NO_SHIELD";
                case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
                case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
                case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
                case WL_CONNECTED: return "WL_CONNECTED";
                case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
                case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
                case WL_DISCONNECTED: return "WL_DISCONNECTED";
                default: return "WL_UNKNOWN";
            }
        }

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

        void mostrarComprobandoServidor(TFT_eSPI& tft) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, "Intentando servidor", tft.width() / 2, (tft.height() / 2) - 18, FontHelper::FONT_TITULO);
            FontHelper::drawStringWithSpanish(tft, "Esperando respuesta...", tft.width() / 2, (tft.height() / 2) + 18, FontHelper::FONT_TEXTO);
        }

        void mostrarResultadoServidor(TFT_eSPI& tft, bool valorPrueba) {
            PantallaBase::fondoConBorde(tft);
            tft.setTextColor(valorPrueba ? TFT_GREEN : TFT_RED, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, valorPrueba ? "Servidor reachable" : "Fallo servidor", tft.width() / 2, (tft.height() / 2) - 28, FontHelper::FONT_TITULO);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            FontHelper::drawStringWithSpanish(tft, String("Variable prueba: ") + (valorPrueba ? "true" : "false"), tft.width() / 2, (tft.height() / 2) + 14, FontHelper::FONT_TEXTO);
        }

        bool interpretarRespuestaPing(String respuesta) {
            respuesta.trim();
            respuesta.toLowerCase();
            return respuesta == "true";
        }

        bool comprobarServidorInterno(TFT_eSPI* tft, bool mostrarPantallas) {
            bool valorPrueba = false;

            if (mostrarPantallas && tft) {
                mostrarComprobandoServidor(*tft);
            }

            if (WiFi.status() == WL_CONNECTED) {
                HTTPClient http;
                http.setConnectTimeout(kServidorTimeoutMs);
                http.setTimeout(kServidorTimeoutMs);

                Serial.print("[Servidor] GET ");
                Serial.println(kServidorPingUrl);

                if (http.begin(kServidorPingUrl)) {
                    const int httpCode = http.GET();
                    Serial.print("[Servidor] HTTP code: ");
                    Serial.println(httpCode);

                    if (httpCode == 200) {
                        String respuesta = http.getString();
                        Serial.print("[Servidor] Respuesta: ");
                        Serial.println(respuesta);
                        valorPrueba = interpretarRespuestaPing(respuesta);
                    } else if (httpCode > 0) {
                        Serial.println("[Servidor] Respuesta HTTP no valida para considerar exito");
                    } else {
                        Serial.print("[Servidor] Error HTTP: ");
                        Serial.println(http.errorToString(httpCode));
                    }
                    http.end();
                } else {
                    Serial.println("[Servidor] No se pudo inicializar la peticion HTTP");
                }
            } else {
                Serial.println("[Servidor] Sin conexion WiFi, variable prueba=false");
            }

            if (mostrarPantallas && tft) {
                mostrarResultadoServidor(*tft, valorPrueba);
                delay(1800);
            }

            return valorPrueba;
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
            WiFi.persistent(false);
            WiFi.setSleep(false);
            WiFi.setAutoReconnect(false);
            WiFi.disconnect(false, true);
            delay(600); // Tiempo extra para que el radio arranque correctamente

            Serial.println("[WiFi] Iniciando conexion...");
            Serial.print("[WiFi] SSID: ");
            Serial.println(ssidDeTienda);
            Serial.print("[WiFi] Password length: ");
            Serial.println(passwordDeTienda.length());

            // Escanear primero: garantiza que el radio ve la red antes de conectar.
            // Se ignora el resultado: intentamos conectar siempre (puede ser red oculta o scan lento).
            escanearRedesYVerificar();

            WiFi.begin(ssidDeTienda.c_str(), passwordDeTienda.c_str());

            const uint32_t timeoutMs = 15000;
            uint32_t inicio = millis();
            wl_status_t ultimoEstado = WL_IDLE_STATUS;
            while (WiFi.status() != WL_CONNECTED && (millis() - inicio) < timeoutMs) {
                wl_status_t estadoActual = WiFi.status();
                if (estadoActual != ultimoEstado) {
                    Serial.print("[WiFi] Estado: ");
                    Serial.println(wifiStatusToString(estadoActual));
                    ultimoEstado = estadoActual;
                }
                delay(200);
            }

            bool ok = (WiFi.status() == WL_CONNECTED);
            Serial.print("[WiFi] Resultado final: ");
            Serial.println(wifiStatusToString(WiFi.status()));
            if (ok) {
                Serial.print("[WiFi] IP: ");
                Serial.println(WiFi.localIP());
            }
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

    inline bool probarConexionServidorConPantalla(TFT_eSPI& tft) {
        return comprobarServidorInterno(&tft, true);
    }

    inline bool probarConexionServidorSilencioso() {
        return comprobarServidorInterno(nullptr, false);
    }

    // Llama esto desde loop(). Reintenta la conexion cada 30s si no esta conectado y hay credenciales.
    inline void tickReconexion() {
        static uint32_t ultimoIntento = 0;
        constexpr uint32_t intervaloMs = 30000;

        if (WiFi.status() == WL_CONNECTED) {
            ultimoIntento = millis(); // Reinicia el contador cuando ya está conectado
            return;
        }
        if (ssidDeTienda.length() == 0) {
            return;
        }
        uint32_t ahora = millis();
        if (ahora - ultimoIntento < intervaloMs) {
            return;
        }
        ultimoIntento = ahora;
        Serial.println("[WiFi] Reintentando conexion...");
        WiFi.mode(WIFI_STA);
        WiFi.persistent(false);
        WiFi.setSleep(false);
        WiFi.setAutoReconnect(false);
        WiFi.disconnect(false, true);
        delay(600);

        // Escanear para diagnóstico. No bloqueamos WiFi.begin() aunque no encuentre la red:
        // puede ser un escaneo lento o red transitoriamente invisible.
        escanearRedesYVerificar();

        WiFi.begin(ssidDeTienda.c_str(), passwordDeTienda.c_str());
    }
}
