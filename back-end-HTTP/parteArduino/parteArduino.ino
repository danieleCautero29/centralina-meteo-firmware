#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// CENTRALINA METEO — LED + LED RGB

// Credenziali WiFi
const char *ssid_AP = ".";
const char *pass_AP = "pastizzo10";

// Pin LED singolo (accendi/spegni)
#define PIN_LED 2

// Pin LED RGB — da collegare su breadboard quando disponibile
#define PIN_R 25
#define PIN_G 26
#define PIN_B 27

// Timing
unsigned long tempoPrecedente = 0;
const int intervallo = 3000;

// URL base Firebase
const String URL_BASE = "https://centralina-meteo-c3703-default-rtdb.europe-west1.firebasedatabase.app";

void setup() {
    Serial.begin(9600);

    // LED singolo
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    // LED RGB — API v3.x: ledcAttach(pin, frequenza, risoluzione)
    ledcAttach(PIN_R, 5000, 8);
    ledcAttach(PIN_G, 5000, 8);
    ledcAttach(PIN_B, 5000, 8);

    // Parte spento
    ledcWrite(PIN_R, 0);
    ledcWrite(PIN_G, 0);
    ledcWrite(PIN_B, 0);

    // Connessione WiFi
    Serial.println("Avvio centralina meteo");
    WiFi.begin(ssid_AP, pass_AP);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connesso!");
    Serial.println(WiFi.localIP());
}

void loop() {
    unsigned long cronometro = millis();

    if (cronometro - tempoPrecedente >= intervallo) {
        tempoPrecedente = cronometro;

        // PARTE 1: INVIA DATI SENSORI (PATCH)
        int valCO2  = random(350, 1000);
        int valT    = random(0, 50);
        int valH    = random(0, 100);
        int valLum  = random(200, 1000);

        String pacchettoDati = "{";
        pacchettoDati += "\"co2\": "         + String(valCO2) + ", ";
        pacchettoDati += "\"temperatura\": " + String(valT)   + ", ";
        pacchettoDati += "\"luminosita\": "  + String(valLum) + ", ";
        pacchettoDati += "\"umidita\": "     + String(valH);
        pacchettoDati += "}";

        Serial.println("Dati inviati: " + pacchettoDati);

        HTTPClient httpPatch;
        httpPatch.begin(URL_BASE + "/centralina.json");
        httpPatch.addHeader("Content-Type", "application/json");
        int codicePatch = httpPatch.sendRequest("PATCH", pacchettoDati);
        if (codicePatch != 200) {
            Serial.println("Errore invio dati: " + String(codicePatch));
        }
        httpPatch.end();

        // PARTE 2: LETTURA STATO LED SINGOLO (GET)
        HTTPClient httpLed;
        httpLed.begin(URL_BASE + "/centralina/led.json");
        int codiceLed = httpLed.GET();

        if (codiceLed == 200) {
            String risposta = httpLed.getString();
            risposta.trim();
            digitalWrite(PIN_LED, risposta == "true" ? HIGH : LOW);
            Serial.println("LED singolo: " + risposta);
        } else {
            Serial.println("Errore lettura LED: " + String(codiceLed));
        }
        httpLed.end();

        // PARTE 3: LETTURA VALORI RGB E IMPOSTA IL LED
        HTTPClient httpRgb;
        httpRgb.begin(URL_BASE + "/centralina/rgb.json");
        int codiceRgb = httpRgb.GET();

        if (codiceRgb == 200) {
            String jsonRgb = httpRgb.getString();

            StaticJsonDocument<64> doc;
            DeserializationError errore = deserializeJson(doc, jsonRgb);

            if (!errore) {
                int r = doc["r"];
                int g = doc["g"];
                int b = doc["b"];

                // Imposta i pin PWM (ha effetto visivo solo con LED RGB su breadboard)
                ledcWrite(PIN_R, r);
                ledcWrite(PIN_G, g);
                ledcWrite(PIN_B, b);

                // Stampa nel serial monitor
                Serial.println("==================");
                Serial.println("RGB aggiornato:");
                Serial.print("  R: "); Serial.println(r);
                Serial.print("  G: "); Serial.println(g);
                Serial.print("  B: "); Serial.println(b);
                Serial.println("==================");
            } else {
                Serial.println("Errore parsing JSON RGB: " + String(errore.c_str()));
            }
        } else {
            Serial.println("Errore lettura RGB: " + String(codiceRgb));
        }
        httpRgb.end();
    }
}
