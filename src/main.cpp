#include <Arduino.h>
#include "HX711.h"
#include <WiFiManager.h>
#include <WebSocketsClient.h>
#include "secrets.h"
#include <ArduinoJson.h>
// Websocket
WebSocketsClient webSocket;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 18;
HX711 scale;

// relays
const unsigned int PUMP1 = 12; // IN2
const unsigned int PUMP2 = 25; // IN6
const unsigned int PUMP3 = 27; // IN4
const unsigned int PUMP4 = 13; // IN1
const unsigned int PUMP5 = 14; // IN3
const unsigned int PUMP6 = 26; // IN5

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void startPump(int index, int weight);
void shutdownPumps();
void order(JsonDocument doc);
int getPumpPin(int pumpNumber);
void handleError(String errorMessage);

bool isInProgress = false;
bool isServiceError = false;

void setup() {
    Serial.begin(115200);

    // Wifi Manager
    WiFiManager wifiManager;
    if (!wifiManager.autoConnect("Bartender")) {
        Serial.println("Connection failed and hit timeout");
        ESP.restart();
    }
    Serial.println("Connecté à : " + WiFi.SSID());
    Serial.println("Adresse IP : " + WiFi.localIP().toString());
    // --------------------


    // Weight sensor
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(1397);
    scale.tare();
    // --------------------

    // Relays
    pinMode(PUMP1, OUTPUT);
    pinMode(PUMP2, OUTPUT);
    pinMode(PUMP3, OUTPUT);
    pinMode(PUMP4, OUTPUT);
    pinMode(PUMP5, OUTPUT);
    pinMode(PUMP6, OUTPUT);
    shutdownPumps();
    // --------------------

    // Websocket
    webSocket.begin(WS_SERVER_ADDRESS, WS_SERVER_PORT, "/");
    webSocket.onEvent(webSocketEvent);
    webSocket.setAuthorization(WS_SERVER_USER, WS_SERVER_PASSWORD);
    webSocket.setReconnectInterval(1000);
    webSocket.enableHeartbeat(15000, 3000, 2);
    Serial.println("Connected to WebSocket");
    // --------------------

}

void loop() {
    webSocket.loop();


    //webSocket.sendTXT("Hello from ESP32");
    // digitalWrite(PUMP1, LOW);
    // delay(10000);
    // digitalWrite(PUMP1, HIGH);
    // digitalWrite(PUMP2, LOW);
    // delay(10000);
    // digitalWrite(PUMP2, HIGH);
    // digitalWrite(PUMP3, LOW);
    // delay(10000);
    // digitalWrite(PUMP3, HIGH);
    // digitalWrite(PUMP4, LOW);
    // delay(10000);
    // digitalWrite(PUMP4, HIGH);
    // digitalWrite(PUMP5, LOW);
    // delay(10000);
    // digitalWrite(PUMP5, HIGH);
    // digitalWrite(PUMP6, LOW);
    // delay(25000);
    // digitalWrite(PUMP6, HIGH);
    // send weight to server
    // long reading = scale.get_units(10);
    // webSocket.sendTXT(String(reading).c_str());
    // webSocket.sendTXT("Hello from ESP32");
  // if (scale.is_ready()) {
  //   scale.set_scale();    
  //   Serial.println("Tare... remove any weights from the scale.");
  //   delay(5000);
  //   scale.tare();
  //   Serial.println("Tare done...");
  //   Serial.print("Place a known weight on the scale...");
  //   delay(5000);
  //   long reading = scale.get_units(10);
  //   Serial.print("Result: ");
  //   Serial.println(reading);
  // } 
  // else {
  //   Serial.println("HX711 not found.");
  // }
  // delay(1000);
}

void order(JsonDocument doc) {
    Serial.println("order started");
    if (isInProgress) {
        handleError("IN_PROGRESS");
        return;
    }
    isInProgress = true;
    JsonObject pumps = doc["pumps"];
    for (JsonPair pump : pumps) {
        String pumpNumber = pump.key().c_str();
        int maxWeight = pump.value().as<int>();
        startPump(pumpNumber.toInt(), maxWeight);
        Serial.println("Pump " + pumpNumber);
        if (isServiceError) {
            Serial.println("Service error");
            isServiceError = false;
            break;
        }
    }
    isInProgress = false;

}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected!");
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected!");
      break;
    case WStype_TEXT:
      Serial.println("WebSocket Message!");
        String jsonStr = String((char*)payload);
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonStr);
        if (error) {
            Serial.print(F("Erreur de parsing JSON: "));
            Serial.println(error.f_str());
            return;
        }
        String type = doc["type"];
        if (type == "ORDER") {
          order(doc);
        }

        Serial.println("type: " + type);
        Serial.println((char*)payload);
      break;
  }
}

void startPump(int index, int targetWeight)
{
    int pumpNumber = 0;
    switch (index) {
        case 1: pumpNumber = PUMP1; break;
        case 2: pumpNumber = PUMP2; break;
        case 3: pumpNumber = PUMP3; break;
        case 4: pumpNumber = PUMP4; break;
        case 5: pumpNumber = PUMP5; break;
        case 6: pumpNumber = PUMP6; break;
        default: pumpNumber = -1;
    }
    Serial.println("Démarrage de la pompe " + String(index) + " avec un poids demander de " + String(targetWeight) + "g");
    int totalTime = 0;
    unsigned long startTime = millis();
    long startWeight = scale.get_units(10);
    Serial.println("Poids de départ : " + String(startWeight) + " g");
    if (startWeight <= 10) {
        Serial.println("Aucun poids détecté pour la pompe " + String(index));
        handleError("NO_GLASS");
        return;
    }
    digitalWrite(pumpNumber, LOW);
    int lastWeight = 0;
    while (true) {
        unsigned long startWeighting = millis();
        long currentWeight = scale.get_units(5);
        
        Serial.println("Poids actuel : " + String(currentWeight) + " g");
        Serial.println("Poids moins le poids de depart : " + String((currentWeight - startWeight)) + " g");

        if ((currentWeight - startWeight) >= targetWeight) {
            Serial.println("Poids cible atteint pour la pompe " + String(index));
            break;
        }

        if ((lastWeight - 50) > currentWeight) {
            Serial.println("Le verre a ete retirer " + String(index));
            handleError("GLASS_WITHDRAWN_" + String(index));
            break;
        }
        lastWeight = currentWeight;
        unsigned long endWeighting = millis();
        Serial.print("Temps écoulé pour la boucle  : ");
        Serial.print(String(endWeighting - startWeighting));
        Serial.println(" ms");
        totalTime += (endWeighting - startWeighting);
        if (totalTime >= 20000) {
            Serial.println("Temps d'attente dépassé pour la pompe " + String(index));
            handleError("TIMEOUT_PUMP_" + String(index));
            break;
        }
    }
    digitalWrite(pumpNumber, HIGH);
}

void shutdownPumps()
{
    digitalWrite(PUMP1, HIGH);
    digitalWrite(PUMP2, HIGH);
    digitalWrite(PUMP3, HIGH);
    digitalWrite(PUMP4, HIGH);
    digitalWrite(PUMP5, HIGH);
    digitalWrite(PUMP6, HIGH);
}

void handleError(String errorMessage) {
    Serial.println("Erreur : " + errorMessage);
    shutdownPumps();
    isInProgress = false;
    isServiceError = true;
    webSocket.sendTXT("{\"type\":\"ERROR\",\"message\":\"" + errorMessage + "\"}");
}