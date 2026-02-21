#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft;

void fullClear(uint16_t c) {
  // Esto fuerza a cubrir todo el área que la librería cree que existe
  tft.fillScreen(c);
  tft.fillRect(0, 0, tft.width(), tft.height(), c);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  tft.init();
  tft.setRotation(3);

  Serial.printf("tft.width()=%d  tft.height()=%d\n", tft.width(), tft.height());

  // Limpia y dibuja “marco” para ver si llega a bordes
  fullClear(TFT_BLACK);

  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_WHITE);
  tft.drawLine(0, 0, tft.width()-1, tft.height()-1, TFT_RED);
  tft.drawLine(tft.width()-1, 0, 0, tft.height()-1, TFT_GREEN);

  // Texto con fondo (esto evita “restos”)
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 10);
  tft.printf("W=%d H=%d", tft.width(), tft.height());
}

void loop() {}