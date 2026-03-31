#pragma once
#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

class FingerprintService {
public:
    FingerprintService(HardwareSerial& serialPort, int8_t rxPin, int8_t txPin, uint32_t baudRate = 57600)
        : serial(serialPort), sensor(&serialPort), rx(rxPin), tx(txPin), baud(baudRate) {}

    void begin() {
        Serial.println("[R503] Iniciando UART del sensor...");
        Serial.print("[R503] RX ESP32 <- sensor TX: GPIO");
        Serial.println(rx);
        Serial.print("[R503] TX ESP32 -> sensor RX: GPIO");
        Serial.println(tx);
        Serial.print("[R503] Baud configurado: ");
        Serial.println(baud);

        serial.begin(baud, SERIAL_8N1, rx, tx);
        delay(500);

        conectado = false;
        for (uint8_t intento = 1; intento <= 3 && !conectado; intento++) {
            Serial.print("[R503] Intento de inicializacion ");
            Serial.println(intento);
            sensor.begin(baud);
            delay(150);
            conectado = sensor.verifyPassword();
            if (!conectado) {
                Serial.println("[R503] verifyPassword() fallo");
                delay(250);
            }
        }

        if (conectado) {
            Serial.println("[R503] Sensor listo");
            encenderLedFijo();
        } else {
            Serial.println("[R503] No responde o password incorrecta");
            Serial.println("[R503] Revisar cableado TX/RX, GND y alimentacion del lector");
        }
    }

    void tick() {
        if (!conectado || !ledTemporalActivo) {
            return;
        }

        if (millis() - ultimoCambioLedMs >= kLedTemporalMs) {
            encenderLedFijo();
            ledTemporalActivo = false;
        }
    }

    bool detectarHuella() {
        if (!conectado) {
            return false;
        }

        uint8_t estadoImagen = sensor.getImage();

        if (esperandoRetirarDedo) {
            if (estadoImagen == FINGERPRINT_NOFINGER) {
                esperandoRetirarDedo = false;
            }
            return false;
        }

        if (estadoImagen == FINGERPRINT_NOFINGER) {
            return false;
        }

        if (estadoImagen != FINGERPRINT_OK) {
            Serial.print("[R503] getImage() codigo: ");
            Serial.println(estadoImagen);
            ultimaCapturaValida = false;
            return false;
        }

        Serial.println("[R503] Dedo detectado");

        uint8_t estadoConversion = sensor.image2Tz(1);
        if (estadoConversion != FINGERPRINT_OK) {
            Serial.print("[R503] image2Tz() codigo: ");
            Serial.println(estadoConversion);
            ultimaCapturaValida = false;
            esperandoRetirarDedo = true;
            return false;
        }

        Serial.println("[R503] Huella capturada correctamente");
        ultimaCapturaValida = true;
        mostrarColorCaptura();
        esperandoRetirarDedo = true;
        return true;
    }

    bool ultimaHuellaValida() const {
        return ultimaCapturaValida;
    }

private:
    HardwareSerial& serial;
    Adafruit_Fingerprint sensor;
    int8_t rx;
    int8_t tx;
    uint32_t baud;
    bool conectado = false;
    bool ultimaCapturaValida = false;
    bool esperandoRetirarDedo = false;
    bool ledTemporalActivo = false;
    uint32_t ultimoCambioLedMs = 0;

    static constexpr uint8_t kColorFijo = FINGERPRINT_LED_BLUE;
    static constexpr uint8_t kColorCaptura = FINGERPRINT_LED_PURPLE;
    static constexpr uint16_t kLedTemporalMs = 450;

    void encenderLedFijo() {
        const uint8_t estado = sensor.LEDcontrol(FINGERPRINT_LED_ON, 0, kColorFijo, 0);
        Serial.print("[R503] LED fijo, codigo: ");
        Serial.println(estado);
    }

    void mostrarColorCaptura() {
        const uint8_t estado = sensor.LEDcontrol(FINGERPRINT_LED_ON, 0, kColorCaptura, 0);
        Serial.print("[R503] LED captura, codigo: ");
        Serial.println(estado);
        ledTemporalActivo = true;
        ultimoCambioLedMs = millis();
    }
};
