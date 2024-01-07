#include <Arduino.h>
#include "HX711.h"
#include <WiFiManager.h>
#include <WebSocketsClient.h>
#include "secrets.h"

// Websocket
WebSocketsClient webSocket;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 18;
HX711 scale;

// relays
const unsigned int PUMP1 = 12;
const unsigned int PUMP2 = 25;
const unsigned int PUMP3 = 27;
const unsigned int PUMP4 = 13;
const unsigned int PUMP5 = 14;
const unsigned int PUMP6 = 26;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void startPump(int index, int time);
void shutdownPumps();

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


    digitalWrite(PUMP1, LOW);
    delay(10000);
    digitalWrite(PUMP1, HIGH);
    digitalWrite(PUMP2, LOW);
    delay(10000);
    digitalWrite(PUMP2, HIGH);
    digitalWrite(PUMP3, LOW);
    delay(10000);
    digitalWrite(PUMP3, HIGH);
    digitalWrite(PUMP4, LOW);
    delay(10000);
    digitalWrite(PUMP4, HIGH);
    digitalWrite(PUMP5, LOW);
    delay(10000);
    digitalWrite(PUMP5, HIGH);
    digitalWrite(PUMP6, LOW);
    delay(25000);
    digitalWrite(PUMP6, HIGH);
    // send weight to server
    // long reading = scale.get_units(10);
    // webSocket.sendTXT(String(reading).c_str());
    //webSocket.sendTXT("Hello from ESP32");
    //delay(1000);
//   if (scale.is_ready()) {
//     //scale.set_scale();    
//     Serial.println("Tare... remove any weights from the scale.");
//     delay(5000);
//     scale.tare();
//     Serial.println("Tare done...");
//     Serial.print("Place a known weight on the scale...");
//     delay(5000);
//     long reading = scale.get_units(10);
//     Serial.print("Result: ");
//     Serial.println(reading);
//   } 
//   else {
//     Serial.println("HX711 not found.");
//   }
//   delay(1000);
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
        Serial.println((char*)payload);
      break;
  }
}

void startPump(int index, int time)
{
    if (time == 0)
    {
        return;
    }
    digitalWrite(index, LOW);
    delay(time);
    digitalWrite(index, HIGH);
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