#pragma once
#include <WiFi.h>

// Servicio sencillo para conectar a una red WiFi.
class ConfiguracionWiFi {
public:
    // Intenta conectar a la red WiFi especificada.
    static bool conectar(const char* ssid, const char* password, unsigned long timeoutMs = 10000) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeoutMs) {
            delay(200);
        }
        return WiFi.status() == WL_CONNECTED;
    }

    // Devuelve la IP local asignada.
    static String ipLocal() {
        return WiFi.localIP().toString();
    }

    // Desconecta del WiFi.
    static void desconectar() {
        WiFi.disconnect();
    }
};
