#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "pantallas/PantallaInicio.h"
#include "pantallas/PantallaTeclado.h"

// ===== Pines según tu esquema =====
#define PIN_SPI_SCK   18
#define PIN_SPI_MISO  19
#define PIN_SPI_MOSI  23

#define PIN_TFT_CS    5
#define PIN_TOUCH_CS  14
#define PIN_RC522_SS  27

// ===== Objetos globales =====
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(PIN_TOUCH_CS);   // SIN IRQ (más estable)

void setup() {
  Serial.begin(115200);
  delay(300);

  // ===== Asegurar que ningún dispositivo SPI queda seleccionado =====
  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);

  pinMode(PIN_TOUCH_CS, OUTPUT);
  digitalWrite(PIN_TOUCH_CS, HIGH);

  pinMode(PIN_RC522_SS, OUTPUT);
  digitalWrite(PIN_RC522_SS, HIGH);

  // ===== Inicializar SPI compartido =====
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);

  // ===== Inicializar pantalla =====
  tft.init();
  tft.setRotation(2);

  // ===== Inicializar táctil =====
  ts.begin();

  // Mostrar splash
  PantallaInicio::mostrar(tft);

  // Si quieres saltar splash:
  // PantallaTeclado::mostrar(tft);
}

void loop() {
  PantallaTeclado::actualizar(tft, ts);
}