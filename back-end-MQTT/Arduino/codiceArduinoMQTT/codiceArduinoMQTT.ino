#include <WiFi.h>
#include <PubSubClient.h>

// CENTRALINA METEO — MQTT (HiveMQ pubblico)

// Credenziali WiFi
const char *ssid_AP = ".";
const char *pass_AP = "pastizzo10";

// Broker MQTT
const char *BROKER    = "broker.hivemq.com";
const int   PORTA     = 1883;
const char *CLIENT_ID = "esp32-centralina-001"; // deve essere unico sul broker

// Topic
const char *TOPIC_SENSORI = "centralina/sensori"; // ESP32 pubblica
const char *TOPIC_LED     = "centralina/led";     // ESP32 si iscrive
const char *TOPIC_RGB     = "centralina/rgb";     // ESP32 si iscrive

// Pin LED singolo
#define PIN_LED 2

// Pin LED RGB
#define PIN_R 25
#define PIN_G 26
#define PIN_B 27

// Timing publish sensori
unsigned long tempoPrecedente = 0;
const int intervallo = 3000;

// Client WiFi e MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// CALLBACK — chiamata automaticamente quando
// arriva un messaggio su un topic sottoscritto
void callback(char *topic, byte *payload, unsigned int lunghezza) {

    // Converte il payload (array di byte) in una stringa leggibile
    String messaggio = "";
    for (int i = 0; i < lunghezza; i++) {
        messaggio += (char)payload[i];
    }

    Serial.println("--- Messaggio ricevuto ---");
    Serial.println("Topic:    " + String(topic));
    Serial.println("Payload:  " + messaggio);

    //  GESTIONE LED SINGOLO 
    if (String(topic) == TOPIC_LED) {
        if (messaggio == "true") {
            digitalWrite(PIN_LED, HIGH);
            Serial.println("LED singolo: ACCESO");
        } else {
            digitalWrite(PIN_LED, LOW);
            Serial.println("LED singolo: SPENTO");
        }
    }

    // --- GESTIONE LED RGB ---
    // Il payload atteso è: {"r":255,"g":100,"b":0}
    // Lo parsiamo manualmente senza ArduinoJson
    if (String(topic) == TOPIC_RGB) {
        int r = estraiValore(messaggio, "\"r\":");
        int g = estraiValore(messaggio, "\"g\":");
        int b = estraiValore(messaggio, "\"b\":");

        ledcWrite(PIN_R, r);
        ledcWrite(PIN_G, g);
        ledcWrite(PIN_B, b);

        Serial.println("RGB aggiornato → R:" + String(r) + " G:" + String(g) + " B:" + String(b));
    }
}

// HELPER — estrae un intero da una stringa JSON
// es: estraiValore("{\"r\":255,\"g\":0}", "\"r\":") → 255
int estraiValore(String json, String chiave) {
    int indice = json.indexOf(chiave);
    if (indice == -1) return 0;
    indice += chiave.length();
    return json.substring(indice).toInt();
}

// CONNESSIONE/RICONNESSIONE MQTT
void connettiBroker() {
    while (!mqttClient.connected()) {
        Serial.print("Connessione al broker MQTT...");

        if (mqttClient.connect(CLIENT_ID)) {
            Serial.println(" connesso!");

            // Sottoscrizione ai topic di controllo
            mqttClient.subscribe(TOPIC_LED);
            mqttClient.subscribe(TOPIC_RGB);
            Serial.println("Iscritto a: " + String(TOPIC_LED));
            Serial.println("Iscritto a: " + String(TOPIC_RGB));

        } else {
            Serial.print(" fallita, codice errore: ");
            Serial.println(mqttClient.state());
            Serial.println("Riprovo tra 3 secondi...");
            delay(3000);
        }
    }
}

void setup() {
    Serial.begin(9600);

    // LED singolo
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    // LED RGB
    ledcAttach(PIN_R, 5000, 8);
    ledcAttach(PIN_G, 5000, 8);
    ledcAttach(PIN_B, 5000, 8);
    ledcWrite(PIN_R, 0);
    ledcWrite(PIN_G, 0);
    ledcWrite(PIN_B, 0);

    // Connessione WiFi
    Serial.println("Avvio centralina meteo MQTT");
    WiFi.begin(ssid_AP, pass_AP);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connesso! IP: " + WiFi.localIP().toString());

    // Configurazione client MQTT
    mqttClient.setServer(BROKER, PORTA);
    mqttClient.setCallback(callback);

    // Prima connessione al broker
    connettiBroker();
}

void loop() {
    // Mantiene viva la connessione MQTT e processa i messaggi in arrivo
    // Se si disconnette, riconnette automaticamente
    if (!mqttClient.connected()) {
        connettiBroker();
    }
    mqttClient.loop(); // <-- FONDAMENTALE: è qui che scatta la callback

    // Ogni 3 secondi pubblica i dati sensori
    unsigned long cronometro = millis();
    if (cronometro - tempoPrecedente >= intervallo) {
        tempoPrecedente = cronometro;

        int valCO2  = random(350, 1000);
        int valT    = random(0, 50);
        int valH    = random(0, 100);
        int valLum  = random(200, 1000);

        // Costruisce il JSON da pubblicare
        String payload = "{";
        payload += "\"co2\":"         + String(valCO2) + ",";
        payload += "\"temperatura\":" + String(valT)   + ",";
        payload += "\"luminosita\":"  + String(valLum) + ",";
        payload += "\"umidita\":"     + String(valH);
        payload += "}";

        mqttClient.publish(TOPIC_SENSORI, payload.c_str());
        Serial.println("Pubblicato su [" + String(TOPIC_SENSORI) + "]: " + payload);
    }
}
