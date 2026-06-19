# centralina-meteo-firmware
parte back-end del progetto
# Centralina Meteo IoT - Firmware (ESP32) 🔌

Questo repository contiene il codice sorgente del firmware sviluppato in C++ (ambiente Arduino) per la scheda **ESP32** della centralina meteo IoT. Il codice si occupa della gestione dei dati dei sensori, della connettività di rete e del controllo degli attuatori (LED integrato e LED RGB).

## 🏷️ Storico delle Versioni (Tag di Git)

Il progetto è stato sviluppato in modo incrementale. Puoi visualizzare e scaricare le diverse tappe architetturali navigando tra i **Tag** di questo repository:

* 📦 **`v1.0-http`**: Prima release del firmware. Sfrutta il protocollo **HTTP** e la libreria `HTTPClient` per inviare i dati dei sensori ed effettuare il polling dello stato dei LED su un database Cloud **Firebase** (Realtime Database).
* 📦 **`v2.0-mqtt` (Versione Corrente)**: Evoluzione del firmware. Sostituisce l'architettura precedente con il protocollo **MQTT** (libreria `PubSubClient`) appoggiandosi al broker pubblico **HiveMQ**. Implementa il paradigma *Publish/Subscribe* azzerando la latenza e abilitando la ricezione dei comandi in tempo reale tramite funzioni di callback.

## 🛠️ Configurazione Hardware (Pinout)

Il codice è configurato per mappare i seguenti pin sulla scheda ESP32:

| Componente | Tipo di Segnale | Pin ESP32 | Descrizione |
| :--- | :--- | :--- | :--- |
| **LED Singolo** | Output Digitale | `PIN 2` | LED integrato nella scheda (Stato ON/OFF) |
| **LED RGB (Rosso)** | Output PWM | `PIN 25` | Controllo intensità canale Red (0-255) |
| **LED RGB (Verde)** | Output PWM | `PIN 26` | Controllo intensità canale Green (0-255) |
| **LED RGB (Blu)** | Output PWM | `PIN 27` | Controllo intensità canale Blue (0-255) |

## 📚 Librerie Richieste (Dipendenze)

Per poter compilare correttamente il codice, assicurati di aver installato tramite il *Gestore Librerie* dell'IDE di Arduino i seguenti pacchetti:

1.  **ArduinoJson** (di Benoit Blanchon): Utilizzata per impacchettare i dati dei sensori in formato JSON prima dell'invio e per decodificare i comandi RGB in arrivo.
2.  **PubSubClient** (di Nick O'Leary): Richiesta specificatamente nella versione `v2.0-mqtt` per gestire la connessione, le sottoscrizioni e le pubblicazioni sul broker MQTT.

## 🚀 Come Compilare ed Eseguire

1. Cambia la visualizzazione del repository sul **Tag** che desideri esaminare (`v1.0-http` o `v2.0-mqtt`).
2. Apri il file `.ino` corrispondente con l'**IDE di Arduino**.
3. All'inizio del codice sorgente, configura le variabili di rete con le tue credenziali locali:
   ```cpp
   const char *ssid_AP = "IL_TUO_WIFI";
   const char *pass_AP = "LA_TUA_PASSWORD";
