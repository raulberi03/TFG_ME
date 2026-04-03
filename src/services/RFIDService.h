#pragma once
#include <Arduino.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config/AppConfig.h"

struct RFIDService {
    MFRC522 reader;
    String ultimoUid;
    bool ultimoValido = false;
    String ultimoMensajeServidor;

    RFIDService(uint8_t ssPin, uint8_t rstPin) : reader(ssPin, rstPin) {}

    void begin() {
        reader.PCD_Init();
        reader.PCD_DumpVersionToSerial();
    }

    bool detectarTarjeta() {
        digitalWrite(AppConfig::Pins::TFT_CS_PIN, HIGH);
        digitalWrite(AppConfig::Pins::TOUCH_CS_PIN, HIGH);

        if (!reader.PICC_IsNewCardPresent()) {
            return false;
        }
        if (!reader.PICC_ReadCardSerial()) {
            Serial.println("[RFID] Tarjeta detectada pero no se pudo leer el UID");
            return false;
        }

        ultimoUid = uidToHex(reader.uid);
        Serial.print("[RFID] UID leido: ");
        Serial.println(ultimoUid);
        reader.PICC_HaltA();
        reader.PCD_StopCrypto1();
        return true;
    }

    const String& ultimoUidHex() const {
        return ultimoUid;
    }

    bool ultimoUidValido() const {
        return ultimoValido;
    }

    const String& ultimoMensaje() const {
        return ultimoMensajeServidor;
    }

    bool loginTarjeta(const String& idTarjeta) {
        if (WiFi.status() != WL_CONNECTED) {
            ultimoValido = false;
            ultimoMensajeServidor = "Sin conexion WiFi";
            return false;
        }

        const uint32_t inicioMs = millis();
        int httpCode = 0;

        do {
            HTTPClient http;
            http.setConnectTimeout(AppConfig::WiFi::ServidorTimeoutMs);
            http.setTimeout(AppConfig::WiFi::ServidorTimeoutMs);

            if (!http.begin(AppConfig::WiFi::ServidorRfidLoginUrl)) {
                ultimoMensajeServidor = "No se pudo conectar al servidor";
                ultimoValido = false;
                return false;
            }

            http.addHeader("Content-Type", "application/json");
            httpCode = http.POST(String("{\"CodigoTarjeta\":\"") + idTarjeta + "\"}");

            if (httpCode > 0) {
                ultimoMensajeServidor = http.getString();
                http.end();
                break;
            }

            ultimoMensajeServidor = http.errorToString(httpCode);
            http.end();
            delay(250);
        } while (millis() - inicioMs < AppConfig::WiFi::ServidorTimeoutMs);

        ultimoValido = httpCode >= 200 && httpCode < 300;
        return ultimoValido;
    }

    bool agregarTarjeta(const String& idTarjeta, const String& codigoUsuario) {
        if (WiFi.status() != WL_CONNECTED) {
            ultimoMensajeServidor = "Sin conexion WiFi";
            return false;
        }

        int32_t codigoUsuarioInt = (int32_t)codigoUsuario.toInt();

        HTTPClient http;
        http.setConnectTimeout(AppConfig::WiFi::ServidorTimeoutMs);
        http.setTimeout(AppConfig::WiFi::ServidorTimeoutMs);

        if (!http.begin(AppConfig::WiFi::ServidorRfidAgregarUrl)) {
            ultimoMensajeServidor = "No se pudo conectar al servidor";
            return false;
        }

        http.addHeader("Content-Type", "application/json");
        const int httpCode = http.POST(String("{\"CodigoTarjeta\":\"") + idTarjeta + "\",\"CodigoUsuario\":" + String(codigoUsuarioInt) + "}");
        ultimoMensajeServidor = http.getString();
        http.end();
        return httpCode >= 200 && httpCode < 300;
    }

    bool sobrescribirTarjeta(const String& idTarjeta, const String& codigoUsuario) {
        if (WiFi.status() != WL_CONNECTED) {
            ultimoMensajeServidor = "Sin conexion WiFi";
            return false;
        }

        int32_t codigoUsuarioInt = (int32_t)codigoUsuario.toInt();

        HTTPClient http;
        http.setConnectTimeout(AppConfig::WiFi::ServidorTimeoutMs);
        http.setTimeout(AppConfig::WiFi::ServidorTimeoutMs);

        if (!http.begin(AppConfig::WiFi::ServidorRfidSobrescribirUrl)) {
            ultimoMensajeServidor = "No se pudo conectar al servidor";
            return false;
        }

        http.addHeader("Content-Type", "application/json");
        const int httpCode = http.POST(String("{\"CodigoTarjeta\":\"") + idTarjeta + "\",\"CodigoUsuario\":" + String(codigoUsuarioInt) + "}");
        ultimoMensajeServidor = http.getString();
        http.end();
        return httpCode >= 200 && httpCode < 300;
    }

    bool eliminarTarjeta(const String& codigoUsuario) {
        if (WiFi.status() != WL_CONNECTED) {
            ultimoMensajeServidor = "Sin conexion WiFi";
            return false;
        }

        int32_t codigoUsuarioInt = (int32_t)codigoUsuario.toInt();

        HTTPClient http;
        http.setConnectTimeout(AppConfig::WiFi::ServidorTimeoutMs);
        http.setTimeout(AppConfig::WiFi::ServidorTimeoutMs);

        if (!http.begin(AppConfig::WiFi::ServidorRfidEliminarUrl)) {
            ultimoMensajeServidor = "No se pudo conectar al servidor";
            return false;
        }

        http.addHeader("Content-Type", "application/json");
        const int httpCode = http.POST(String("{\"CodigoUsuario\":") + String(codigoUsuarioInt) + "}");
        ultimoMensajeServidor = http.getString();
        http.end();
        return httpCode >= 200 && httpCode < 300;
    }

    String uidToHex(const MFRC522::Uid& uid) {
        String out;
        out.reserve(uid.size * 2);
        for (byte i = 0; i < uid.size; i++) {
            if (uid.uidByte[i] < 0x10) {
                out += '0';
            }
            out += String(uid.uidByte[i], HEX);
        }
        out.toUpperCase();
        return out;
    }
};
