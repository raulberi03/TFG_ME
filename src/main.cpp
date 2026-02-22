
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "pantallas/PantallaBase.h"
#include "pantallas/PantallaInicio.h"
#include "pantallas/PantallaNumerica.h"
#include "pantallas/MenuAdministrador.h"

#define PIN_SPI_SCK   18
#define PIN_SPI_MISO  19
#define PIN_SPI_MOSI  23
#define PIN_TFT_CS    5
#define PIN_TOUCH_CS  14
#define PIN_RC522_SS  27

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(PIN_TOUCH_CS);

String ultimoUsuario = "";

void pedirUsuario();
void pedirPassword();

void onUsuarioOk(TFT_eSPI& tft, const String& usuario) {
  ultimoUsuario = usuario;
  pedirPassword();
}

void onUsuarioClr(TFT_eSPI& tft, String& v) {
  v = "";
  PantallaNumerica::refrescarCajaValor(tft);
}

void onPasswordOk(TFT_eSPI& tft, const String& pass) {
  if (ultimoUsuario == "1234" && pass == "1234") {
    tft.fillScreen(TFT_GREEN);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.drawString("Login OK", tft.width()/2, tft.height()/2);
    delay(1000);
    PantallaNumerica::pintada() = false;  // Resetea el teclado
    MenuAdministrador::mostrar(tft);
  } else {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.drawString("Login FAIL", tft.width()/2, tft.height()/2);
    delay(1200);
    pedirUsuario();
  }
}

void onPasswordClr(TFT_eSPI& tft, String& v) {
  v = "";
  PantallaNumerica::refrescarCajaValor(tft);
}

void pedirUsuario() {
  PantallaNumerica::Config cfg;
  cfg.textoSuperior = "Codigo Usuario";
  cfg.maxLen = 8;
  cfg.onOk = onUsuarioOk;
  cfg.onClr = onUsuarioClr;
  PantallaNumerica::mostrar(tft, cfg);
}

void pedirPassword() {
  PantallaNumerica::Config cfg;
  cfg.textoSuperior = "Password";
  cfg.maxLen = 8;
  cfg.onOk = onPasswordOk;
  cfg.onClr = onPasswordClr;
  PantallaNumerica::mostrar(tft, cfg);
}

void pedirSSID(TFT_eSPI& tft) {
  // Placeholder: muestra un mensaje de que esto será implementado
  PantallaBase::fondoConBorde(tft);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString("Cambiar SSID", tft.width()/2, tft.height()/2 - 20);
  tft.drawString("Proximamente...", tft.width()/2, tft.height()/2 + 20);
  delay(2000);
  MenuAdministrador::mostrar(tft);
}

void pedirPasswordWifi(TFT_eSPI& tft) {
  // Placeholder: muestra un mensaje de que esto será implementado
  PantallaBase::fondoConBorde(tft);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString("Cambiar Password WiFi", tft.width()/2, tft.height()/2 - 20);
  tft.drawString("Proximamente...", tft.width()/2, tft.height()/2 + 20);
  delay(2000);
  MenuAdministrador::mostrar(tft);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(PIN_TFT_CS, OUTPUT); digitalWrite(PIN_TFT_CS, HIGH);
  pinMode(PIN_TOUCH_CS, OUTPUT); digitalWrite(PIN_TOUCH_CS, HIGH);
  pinMode(PIN_RC522_SS, OUTPUT); digitalWrite(PIN_RC522_SS, HIGH);
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
  tft.init(); tft.setRotation(2);
  ts.begin();
  PantallaInicio::mostrar(tft);
  delay(500);
  pedirUsuario();
}

void loop() {
  // Verificar si se debe volver al login desde el menú administrador
  if (MenuAdministrador::debeVolverAlLogin()) {
    MenuAdministrador::debeVolverAlLogin() = false;
    pedirUsuario();
    return;
  }

  // Procesar toques del menú administrador
  if (MenuAdministrador::pintada()) {
    TS_Point p = ts.getPoint();
    if (p.z >= 20) {
      uint32_t ahora = millis();
      static uint32_t ultimoToqueMs = 0;
      if (ahora - ultimoToqueMs > 180) {
        ultimoToqueMs = ahora;
        int x = map(p.y, 3723, 378, 0, tft.width() - 1);
        int y = map(p.x, 446, 3832, 0, tft.height() - 1);
        x = constrain(x, 0, tft.width() - 1);
        y = constrain(y, 0, tft.height() - 1);
        MenuAdministrador::procesarToque(tft, x, y);
      }
    }
    return;
  }

  if (PantallaNumerica::pintada()) {
    TS_Point p = ts.getPoint();
    if (p.z >= 20) {
      uint32_t ahora = millis();
      static uint32_t ultimoToqueMs = 0;
      if (ahora - ultimoToqueMs > 180) {
        ultimoToqueMs = ahora;
        int x = map(p.y, 3723, 378, 0, tft.width() - 1);
        int y = map(p.x, 446, 3832, 0, tft.height() - 1);
        x = constrain(x, 0, tft.width() - 1);
        y = constrain(y, 0, tft.height() - 1);
        int y0 = PantallaNumerica::tecladoY(tft);
        if (y >= y0) {
          int col = x / PantallaNumerica::teclaW(tft);
          int fila = (y - y0) / PantallaNumerica::teclaH(tft);
          if (col >= 0 && col <= 2 && fila >= 0 && fila <= 3) {
            PantallaNumerica::efectoPulsacion(tft, fila, col);
            PantallaNumerica::procesarTecla(tft, PantallaNumerica::labelTecla(fila, col));
          }
        }
      }
    }
  }
}