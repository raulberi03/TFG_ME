
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "pantallas/PantallaInicio.h"
#include "config/AppConfig.h"
#include "services/RFIDService.h"
#include "services/FingerprintService.h"
#include "controllers/AppController.h"

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(AppConfig::Pins::TOUCH_CS_PIN);
RFIDService rfid(AppConfig::Pins::RC522_SS, AppConfig::Pins::RC522_RST);
FingerprintService fingerprint(Serial2, AppConfig::Pins::R503_RX, AppConfig::Pins::R503_TX);

void setup() {
  // Inicializa perifericos y pantalla principal.
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("[BOOT] ESP32 iniciado");
  Serial.println("[BOOT] Inicializando perifericos...");
  pinMode(AppConfig::Pins::TFT_CS_PIN, OUTPUT); digitalWrite(AppConfig::Pins::TFT_CS_PIN, HIGH);
  pinMode(AppConfig::Pins::TOUCH_CS_PIN, OUTPUT); digitalWrite(AppConfig::Pins::TOUCH_CS_PIN, HIGH);
  pinMode(AppConfig::Pins::RC522_SS, OUTPUT); digitalWrite(AppConfig::Pins::RC522_SS, HIGH);
  SPI.begin(AppConfig::Pins::SPI_SCK, AppConfig::Pins::SPI_MISO, AppConfig::Pins::SPI_MOSI);
  Serial.println("[BOOT] RFID begin()");
  rfid.begin();
  Serial.println("[BOOT] Fingerprint begin()");
  fingerprint.begin();
  Serial.println("[BOOT] Pantalla TFT init()");
  tft.init(); tft.setRotation(2);
  ts.begin();
  PantallaInicio::mostrar(tft);
  delay(500);
  Serial.println("[BOOT] AppController begin()");
  // Arranca el flujo de login y gestion.
  AppController::begin(tft, ts, rfid, fingerprint);
  Serial.println("[BOOT] Setup completado");
}

void loop() {
  // Delegar la logica de UI y lecturas.
  AppController::loop(tft, ts, rfid, fingerprint);
}