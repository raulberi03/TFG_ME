
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "pantallas/PantallaInicio.h"
#include "services/RFIDService.h"
#include "services/RFIDUsuariosService.h"
#include "controllers/AppController.h"

#define PIN_SPI_SCK   18
#define PIN_SPI_MISO  19
#define PIN_SPI_MOSI  23
#define PIN_TFT_CS    5
#define PIN_TOUCH_CS  14
#define PIN_RC522_SS  27
#define PIN_RC522_RST 26

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(PIN_TOUCH_CS);
RFIDService rfid(PIN_RC522_SS, PIN_RC522_RST);
RFIDUsuariosService rfidUsuarios;

void setup() {
  // Inicializa perifericos y pantalla principal.
  Serial.begin(115200);
  delay(300);
  pinMode(PIN_TFT_CS, OUTPUT); digitalWrite(PIN_TFT_CS, HIGH);
  pinMode(PIN_TOUCH_CS, OUTPUT); digitalWrite(PIN_TOUCH_CS, HIGH);
  pinMode(PIN_RC522_SS, OUTPUT); digitalWrite(PIN_RC522_SS, HIGH);
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
  rfid.begin();
  tft.init(); tft.setRotation(2);
  ts.begin();
  PantallaInicio::mostrar(tft);
  delay(500);
  // Arranca el flujo de login y gestion.
  AppController::begin(tft, ts, rfid, rfidUsuarios);
}

void loop() {
  // Delegar la logica de UI y lecturas.
  AppController::loop(tft, ts, rfid, rfidUsuarios);
}