#pragma once
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Preferences.h>
#include <Adafruit_Fingerprint.h>
#include "config/AppConfig.h"

class FingerprintService {
public:
    using PasoHuellaCallback = void (*)(void* contexto, const char* linea1, const char* linea2);

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

        if (!conectado) {
            Serial.println("[R503] No responde o password incorrecta");
            Serial.println("[R503] Revisar cableado TX/RX, GND y alimentacion del lector");
            return;
        }

        sensor.getParameters();

        Serial.println("[R503] Sensor listo");
        Serial.print("[R503] Packet size del sensor: ");
        Serial.println(sensor.packet_len);
        activarModoLogin();
    }

    void tick() {
        if (!conectado || !ledTemporalActivo) {
            return;
        }

        if (millis() >= ledTemporalHastaMs) {
            ledTemporalActivo = false;
            aplicarColorLed(colorBaseLed);
        }
    }

    bool detectarHuella() {
        if (!conectado) {
            return false;
        }

        const uint8_t estadoImagen = sensor.getImage();
        if (esperandoRetirada && estadoImagen == FINGERPRINT_NOFINGER) {
            esperandoRetirada = false;
            return false;
        }

        if (esperandoRetirada || estadoImagen == FINGERPRINT_NOFINGER) {
            return false;
        }

        if (estadoImagen != FINGERPRINT_OK) {
            Serial.print("[R503] getImage() codigo: ");
            Serial.println(estadoImagen);
            return false;
        }

        Serial.println("[R503] Dedo detectado");
        esperandoRetirada = true;
        return true;
    }

    const String& ultimoMensaje() const {
        return ultimoMensajeServidor;
    }

    void activarModoLogin() {
        colorBaseLed = kColorLogin;
        if (!ledTemporalActivo) {
            aplicarColorLed(colorBaseLed);
        }
    }

    void apagarLed() {
        colorBaseLed = 0;
        ledTemporalActivo = false;
        if (!conectado) {
            return;
        }

        const uint8_t estado = sensor.LEDcontrol(FINGERPRINT_LED_OFF, 0, 0, 0);
        if (estado != FINGERPRINT_OK) {
            Serial.print("[R503] LEDcontrol(OFF) codigo: ");
            Serial.println(estado);
        }
    }

    void activarModoOperacion() {
        colorBaseLed = kColorOperacion;
        if (!ledTemporalActivo) {
            aplicarColorLed(colorBaseLed);
        }
    }

    void indicarExito() {
        mostrarColorTemporal(kColorExito);
    }

    void indicarError() {
        mostrarColorTemporal(kColorError);
    }

    bool loginHuella() {
        if (!conectado) {
            ultimoMensajeServidor = "Sensor de huella no disponible";
            return false;
        }

        ultimoMensajeServidor = "";
        ultimoSlotDetectado = 0;

        const uint8_t estadoConteo = sensor.getTemplateCount();
        if (estadoConteo != FINGERPRINT_OK) {
            ultimoMensajeServidor = "No se pudo leer el lector";
            Serial.print("[R503] getTemplateCount() codigo: ");
            Serial.println(estadoConteo);
            return false;
        }

        if (sensor.templateCount == 0) {
            ultimoMensajeServidor = "No hay huellas registradas";
            return false;
        }

        if (!capturarCaracteristicas(1)) {
            esperandoRetirada = true;
            return false;
        }

        uint8_t estadoBusqueda = sensor.fingerSearch(1);
        if (estadoBusqueda != FINGERPRINT_OK && estadoBusqueda != FINGERPRINT_NOTFOUND) {
            Serial.print("[R503] fingerSearch() codigo: ");
            Serial.println(estadoBusqueda);
            estadoBusqueda = sensor.fingerFastSearch();
        }
        esperandoRetirada = true;
        if (estadoBusqueda != FINGERPRINT_OK) {
            ultimoMensajeServidor = estadoBusqueda == FINGERPRINT_NOTFOUND ? "Huella no registrada" : "No se pudo buscar la huella";
            Serial.print("[R503] busqueda huella codigo: ");
            Serial.println(estadoBusqueda);
            return false;
        }

        ultimoSlotDetectado = sensor.fingerID;
        String codigoUsuario;
        if (!obtenerUsuarioPorSlot(ultimoSlotDetectado, codigoUsuario)) {
            ultimoMensajeServidor = "Huella sin vinculo local";
            Serial.print("[R503] Slot detectado sin usuario local: ");
            Serial.println((int)ultimoSlotDetectado);
            return false;
        }

        Serial.print("[R503] Huella encontrada en slot ");
        Serial.println((int)ultimoSlotDetectado);
        return true;
    }

    bool agregarHuella(const String& codigoUsuario, PasoHuellaCallback callback = nullptr, void* contexto = nullptr) {
        if (!conectado) {
            ultimoMensajeServidor = "Sensor de huella no disponible";
            return false;
        }

        uint16_t slotExistente = 0;
        if (obtenerSlotUsuario(codigoUsuario, slotExistente)) {
            ultimoMensajeServidor = "El usuario ya tiene huella";
            return false;
        }

        uint16_t slotLibre = 0;
        if (!encontrarSlotLibre(slotLibre)) {
            if (ultimoMensajeServidor.length() == 0) {
                ultimoMensajeServidor = "No hay slots libres";
            }
            return false;
        }

        if (!capturarYGuardarModelo(slotLibre, callback, contexto)) {
            return false;
        }

        const bool backendOk = sincronizarVinculoHuella(AppConfig::WiFi::ServidorHuellaAgregarUrl, codigoUsuario, slotLibre);
        if (!backendOk) {
            sensor.deleteModel(slotLibre);
            return false;
        }

        guardarVinculoLocal(codigoUsuario, slotLibre);
        ultimoSlotDetectado = slotLibre;
        return true;
    }

    bool sobrescribirHuella(const String& codigoUsuario, PasoHuellaCallback callback = nullptr, void* contexto = nullptr) {
        if (!conectado) {
            ultimoMensajeServidor = "Sensor de huella no disponible";
            return false;
        }

        uint16_t slot = 0;
        if (!obtenerSlotUsuario(codigoUsuario, slot)) {
            ultimoMensajeServidor = "El usuario no tiene huella";
            return false;
        }

        if (!capturarYGuardarModelo(slot, callback, contexto)) {
            return false;
        }

        const bool backendOk = sincronizarVinculoHuella(AppConfig::WiFi::ServidorHuellaSobrescribirUrl, codigoUsuario, slot);
        if (!backendOk) {
            return false;
        }

        guardarVinculoLocal(codigoUsuario, slot);
        ultimoSlotDetectado = slot;
        return true;
    }

    bool eliminarHuella(const String& codigoUsuario) {
        if (!conectado) {
            ultimoMensajeServidor = "Sensor de huella no disponible";
            return false;
        }

        uint16_t slot = 0;
        if (!eliminarHuellaServidor(codigoUsuario, slot)) {
            return false;
        }

        const uint8_t estadoBorrado = sensor.deleteModel(slot);
        if (estadoBorrado != FINGERPRINT_OK) {
            ultimoMensajeServidor = "No se pudo borrar la huella del lector";
            Serial.print("[R503] deleteModel() codigo: ");
            Serial.println(estadoBorrado);
            return false;
        }

        borrarVinculoLocal(codigoUsuario, slot);
        ultimoSlotDetectado = 0;
        return true;
    }

    bool limpiarLector() {
        if (!conectado) {
            ultimoMensajeServidor = "Sensor de huella no disponible";
            return false;
        }

        const uint8_t estado = sensor.emptyDatabase();
        if (estado != FINGERPRINT_OK) {
            ultimoMensajeServidor = "No se pudo limpiar el lector";
            Serial.print("[R503] emptyDatabase() codigo: ");
            Serial.println(estado);
            return false;
        }

        prefs.begin("huellas", false);
        prefs.clear();
        prefs.end();
        ultimoSlotDetectado = 0;
        ultimoMensajeServidor = "";
        return true;
    }

    uint16_t ultimoSlot() const {
        return ultimoSlotDetectado;
    }

private:
    HardwareSerial& serial;
    Adafruit_Fingerprint sensor;
    Preferences prefs;
    int8_t rx;
    int8_t tx;
    uint32_t baud;
    bool conectado = false;
    bool esperandoRetirada = false;
    uint16_t ultimoSlotDetectado = 0;
    String ultimoMensajeServidor;

    static constexpr uint16_t kTimeoutLecturaMs = 15000;
    static constexpr uint16_t kDelaySondeoMs = 30;
    static constexpr uint16_t kLedResultadoMs = 1800;
    static constexpr uint8_t kColorLogin = FINGERPRINT_LED_BLUE;
    static constexpr uint8_t kColorOperacion = FINGERPRINT_LED_PURPLE;
    static constexpr uint8_t kColorError = FINGERPRINT_LED_RED;
    static constexpr uint8_t kColorExito = 0x04;

    uint8_t colorBaseLed = kColorLogin;
    bool ledTemporalActivo = false;
    uint32_t ledTemporalHastaMs = 0;

    String claveUsuario(const String& codigoUsuario) {
        return String("u_") + codigoUsuario;
    }

    String claveSlot(uint16_t slot) {
        return String("s_") + String(slot);
    }

    bool obtenerSlotUsuario(const String& codigoUsuario, uint16_t& slot) {
        prefs.begin("huellas", true);
        const String valor = prefs.getString(claveUsuario(codigoUsuario).c_str(), "");
        prefs.end();
        if (valor.length() == 0) {
            return false;
        }
        slot = (uint16_t)valor.toInt();
        return slot > 0;
    }

    bool obtenerUsuarioPorSlot(uint16_t slot, String& codigoUsuario) {
        prefs.begin("huellas", true);
        codigoUsuario = prefs.getString(claveSlot(slot).c_str(), "");
        prefs.end();
        return codigoUsuario.length() > 0;
    }

    void guardarVinculoLocal(const String& codigoUsuario, uint16_t slot) {
        prefs.begin("huellas", false);
        prefs.putString(claveUsuario(codigoUsuario).c_str(), String(slot));
        prefs.putString(claveSlot(slot).c_str(), codigoUsuario);
        prefs.end();
    }

    void borrarVinculoLocal(const String& codigoUsuario, uint16_t slot) {
        prefs.begin("huellas", false);
        prefs.remove(claveUsuario(codigoUsuario).c_str());
        prefs.remove(claveSlot(slot).c_str());
        prefs.end();
    }

    bool encontrarSlotLibre(uint16_t& slotLibre) {
        const uint16_t capacidad = sensor.capacity > 0 ? sensor.capacity : 64;
        prefs.begin("huellas", true);
        for (uint16_t slot = 1; slot <= capacidad; slot++) {
            if (!prefs.isKey(claveSlot(slot).c_str())) {
                prefs.end();
                slotLibre = slot;
                return true;
            }
        }
        prefs.end();
        ultimoMensajeServidor = "No hay espacio en el lector";
        return false;
    }

    bool capturarYGuardarModelo(uint16_t slot, PasoHuellaCallback callback, void* contexto) {
        ultimoMensajeServidor = "";

        notificarPaso(callback, contexto, "Coloque el dedo", "1/2");
        Serial.println("[R503] Captura 1/2");
        if (!capturarCaracteristicas(1)) {
            return false;
        }

        notificarPaso(callback, contexto, "Retire el dedo", "");
        Serial.println("[R503] Retire el dedo");
        if (!esperarSinDedo()) {
            ultimoMensajeServidor = "No se retiro el dedo a tiempo";
            return false;
        }

        notificarPaso(callback, contexto, "Coloque el dedo", "2/2");
        Serial.println("[R503] Coloque el mismo dedo otra vez");
        if (!capturarCaracteristicas(2)) {
            return false;
        }

        const uint8_t estadoModelo = sensor.createModel();
        if (estadoModelo != FINGERPRINT_OK) {
            ultimoMensajeServidor = estadoModelo == FINGERPRINT_ENROLLMISMATCH ? "Las dos capturas no coinciden" : "No se pudo crear el modelo";
            Serial.print("[R503] createModel() codigo: ");
            Serial.println(estadoModelo);
            return false;
        }

        const uint8_t estadoGuardado = sensor.storeModel(slot);
        esperandoRetirada = true;
        if (estadoGuardado != FINGERPRINT_OK) {
            ultimoMensajeServidor = "No se pudo guardar la huella en el lector";
            Serial.print("[R503] storeModel() codigo: ");
            Serial.println(estadoGuardado);
            return false;
        }

        Serial.print("[R503] Huella guardada en slot ");
        Serial.println((int)slot);
        return true;
    }

    void notificarPaso(PasoHuellaCallback callback, void* contexto, const char* linea1, const char* linea2) {
        if (callback != nullptr) {
            callback(contexto, linea1, linea2);
        }
    }

    bool capturarCaracteristicas(uint8_t slot) {
        const uint8_t estadoImagen = esperarImagen();
        if (estadoImagen != FINGERPRINT_OK) {
            ultimoMensajeServidor = mensajeCapturaImagen(estadoImagen);
            Serial.print("[R503] getImage() codigo: ");
            Serial.println(estadoImagen);
            return false;
        }

        const uint8_t estadoConversion = sensor.image2Tz(slot);
        if (estadoConversion != FINGERPRINT_OK) {
            ultimoMensajeServidor = mensajeConversion(estadoConversion);
            Serial.print("[R503] image2Tz() codigo: ");
            Serial.println(estadoConversion);
            return false;
        }

        return true;
    }

    uint8_t esperarImagen() {
        const uint32_t inicioMs = millis();
        while ((millis() - inicioMs) < kTimeoutLecturaMs) {
            const uint8_t estado = sensor.getImage();
            if (estado == FINGERPRINT_OK) {
                return estado;
            }
            if (estado != FINGERPRINT_NOFINGER) {
                return estado;
            }
            delay(kDelaySondeoMs);
        }
        return FINGERPRINT_TIMEOUT;
    }

    bool esperarSinDedo() {
        const uint32_t inicioMs = millis();
        while ((millis() - inicioMs) < kTimeoutLecturaMs) {
            if (sensor.getImage() == FINGERPRINT_NOFINGER) {
                return true;
            }
            delay(kDelaySondeoMs);
        }
        return false;
    }

    const char* mensajeCapturaImagen(uint8_t codigo) {
        if (codigo == FINGERPRINT_TIMEOUT) return "Tiempo agotado esperando dedo";
        if (codigo == FINGERPRINT_IMAGEFAIL) return "No se pudo capturar la imagen";
        return "Error capturando la imagen";
    }

    const char* mensajeConversion(uint8_t codigo) {
        if (codigo == FINGERPRINT_IMAGEMESS) return "Imagen de huella poco clara";
        if (codigo == FINGERPRINT_FEATUREFAIL) return "No se pudieron extraer rasgos";
        if (codigo == FINGERPRINT_INVALIDIMAGE) return "Imagen de huella invalida";
        return "Error procesando la huella";
    }

    bool sincronizarVinculoHuella(const char* url, const String& codigoUsuario, uint16_t slot) {
        const int32_t codigoUsuarioInt = (int32_t)codigoUsuario.toInt();
        return enviarPayload(
            url,
            String("{\"CodigoUsuario\":") + String(codigoUsuarioInt) + String(",\"IdHuellaSensor\":") + String(slot) + "}"
        );
    }

    bool eliminarHuellaServidor(const String& codigoUsuario, uint16_t& slot) {
        if (WiFi.status() != WL_CONNECTED) {
            ultimoMensajeServidor = "Sin conexion WiFi";
            return false;
        }

        HTTPClient http;
        http.setConnectTimeout(AppConfig::WiFi::ServidorTimeoutMs);
        http.setTimeout(AppConfig::WiFi::ServidorTimeoutMs);

        if (!http.begin(AppConfig::WiFi::ServidorHuellaEliminarUrl)) {
            ultimoMensajeServidor = "No se pudo conectar al servidor";
            return false;
        }

        http.addHeader("Content-Type", "application/json");
        const String payload = String("{\"CodigoUsuario\":") + String((int32_t)codigoUsuario.toInt()) + "}";
        const int httpCode = http.POST(payload);
        const String respuesta = http.getString();
        http.end();

        if (httpCode < 200 || httpCode >= 300) {
            ultimoMensajeServidor = respuesta;
            return false;
        }

        String respuestaNormalizada = respuesta;
        respuestaNormalizada.trim();
        respuestaNormalizada.toLowerCase();
        if (respuestaNormalizada == "null" || respuestaNormalizada.length() == 0) {
            ultimoMensajeServidor = "No hay huella para eliminar";
            return false;
        }

        if (!extraerIdHuellaSensor(respuesta, slot)) {
            ultimoMensajeServidor = "El servidor no devolvio IdHuellaSensor valido";
            Serial.print("[R503] Respuesta eliminar huella: ");
            Serial.println(respuesta);
            return false;
        }

        ultimoMensajeServidor = respuesta;
        return true;
    }

    bool extraerIdHuellaSensor(String respuesta, uint16_t& slot) {
        respuesta.trim();
        if (respuesta.length() == 0) {
            return false;
        }

        bool soloDigitos = true;
        for (size_t i = 0; i < respuesta.length(); i++) {
            if (!isDigit((unsigned char)respuesta[i])) {
                soloDigitos = false;
                break;
            }
        }

        if (soloDigitos) {
            slot = (uint16_t)respuesta.toInt();
            return slot > 0;
        }

        const String clave = "IdHuellaSensor";
        int idx = respuesta.indexOf(clave);
        if (idx < 0) {
            idx = respuesta.indexOf("idHuellaSensor");
        }
        if (idx < 0) {
            idx = respuesta.indexOf("idHuella");
        }
        if (idx < 0) {
            return false;
        }

        idx = respuesta.indexOf(':', idx);
        if (idx < 0) {
            return false;
        }
        idx++;
        while (idx < respuesta.length() && (respuesta[idx] == ' ' || respuesta[idx] == '"')) {
            idx++;
        }

        String numero = "";
        while (idx < respuesta.length() && isDigit((unsigned char)respuesta[idx])) {
            numero += respuesta[idx++];
        }

        if (numero.length() == 0) {
            return false;
        }

        slot = (uint16_t)numero.toInt();
        return slot > 0;
    }

    void mostrarColorTemporal(uint8_t color) {
        if (!conectado) {
            return;
        }

        ledTemporalActivo = true;
        ledTemporalHastaMs = millis() + kLedResultadoMs;
        aplicarColorLed(color);
    }

    void aplicarColorLed(uint8_t color) {
        if (color == 0) {
            apagarLed();
            return;
        }

        const uint8_t estado = sensor.LEDcontrol(FINGERPRINT_LED_ON, 0, color, 0);
        if (estado != FINGERPRINT_OK) {
            Serial.print("[R503] LEDcontrol() codigo: ");
            Serial.println(estado);
        }
    }

    bool enviarPayload(const char* url, const String& payload) {
        if (WiFi.status() != WL_CONNECTED) {
            ultimoMensajeServidor = "Sin conexion WiFi";
            return false;
        }

        HTTPClient http;
        http.setConnectTimeout(AppConfig::WiFi::ServidorTimeoutMs);
        http.setTimeout(AppConfig::WiFi::ServidorTimeoutMs);

        if (!http.begin(url)) {
            ultimoMensajeServidor = "No se pudo conectar al servidor";
            return false;
        }

        http.addHeader("Content-Type", "application/json");
        const int httpCode = http.POST(payload);
        ultimoMensajeServidor = http.getString();
        http.end();
        return httpCode >= 200 && httpCode < 300;
    }
};
