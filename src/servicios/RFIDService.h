#pragma once
#include <Arduino.h>
#include <MFRC522.h>

class RFIDService {
public:
    RFIDService(uint8_t ssPin, uint8_t rstPin)
        : reader(ssPin, rstPin) {}

    void begin() {
        reader.PCD_Init();
        reader.PCD_DumpVersionToSerial();
    }

    // Devuelve true cuando se detecta una tarjeta y se puede leer el UID.
    bool detectarTarjeta() {
        if (!reader.PICC_IsNewCardPresent()) {
            return false;
        }
        if (!reader.PICC_ReadCardSerial()) {
            return false;
        }
        ultimoUid = uidToHex(reader.uid);
        reader.PICC_HaltA();
        reader.PCD_StopCrypto1();
        ultimoValido = validarContraServidor(ultimoUid);
        return true;
    }

    const String& ultimoUidHex() const {
        return ultimoUid;
    }

    bool ultimoUidValido() const {
        return ultimoValido;
    }

private:
    MFRC522 reader;
    String ultimoUid;
    bool ultimoValido = false;

    // Placeholder: cuando exista servidor real, se valida aqui.
    bool validarContraServidor(const String& uid) {
        (void)uid;
        return true;
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
