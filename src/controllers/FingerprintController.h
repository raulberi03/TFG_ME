#pragma once
#include <TFT_eSPI.h>
#include "controllers/LoginController.h"

namespace FingerprintController {
    inline void begin() {}

    inline void handleFingerprintLogin(TFT_eSPI& tft, bool valido) {
        LoginController::handleHuellaLogin(tft, valido);
    }
}
