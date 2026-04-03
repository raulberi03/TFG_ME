#pragma once

#include <Arduino.h>

namespace AppConfig {
    namespace Pins {
        constexpr uint8_t SPI_SCK = 18;
        constexpr uint8_t SPI_MISO = 19;
        constexpr uint8_t SPI_MOSI = 23;
        constexpr uint8_t TFT_CS_PIN = 5;
        constexpr uint8_t TOUCH_CS_PIN = 14;
        constexpr uint8_t RC522_SS = 27;
        constexpr uint8_t RC522_RST = 26;
        constexpr uint8_t R503_TX = 17;
        constexpr uint8_t R503_RX = 16;
    }

    namespace Login {
        constexpr char AdminUser[] = "1234";
        constexpr char AdminPass[] = "1234";
    }

    namespace WiFi {
        constexpr char ServidorPingUrl[] = "http://192.168.1.131:5286/api/Conexion/ping";
        constexpr char ServidorRfidLoginUrl[] = "http://192.168.1.131:5286/api/RFID/login-tarjeta";
        constexpr char ServidorRfidAgregarUrl[] = "http://192.168.1.131:5286/api/RFID/agregar-tarjeta";
        constexpr char ServidorRfidSobrescribirUrl[] = "http://192.168.1.131:5286/api/RFID/sobrescribir-tarjeta";
        constexpr char ServidorRfidEliminarUrl[] = "http://192.168.1.131:5286/api/RFID/eliminar-tarjeta";
        constexpr char ServidorHuellaLoginUrl[] = "http://192.168.1.131:5286/api/Huella/login-huella";
        constexpr char ServidorHuellaAgregarUrl[] = "http://192.168.1.131:5286/api/Huella/agregar-huella";
        constexpr char ServidorHuellaSobrescribirUrl[] = "http://192.168.1.131:5286/api/Huella/sobrescribir-huella";
        constexpr char ServidorHuellaEliminarUrl[] = "http://192.168.1.131:5286/api/Huella/eliminar-huella";
        constexpr uint16_t ServidorTimeoutMs = 15000;
    }
}
