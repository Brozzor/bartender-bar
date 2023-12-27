#include <Arduino.h>
#include "HX711.h"
#include <WiFiManager.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 18;
HX711 scale;

// relays
const unsigned int IN1 = 13;
const unsigned int IN2 = 26;
const unsigned int IN3 = 14;
const unsigned int IN4 = 25;
const unsigned int IN5 = 12;
const unsigned int IN6 = 27;

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
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(IN5, OUTPUT);
    pinMode(IN6, OUTPUT);
    // TODO : shutdown all relays
    // --------------------

}

void loop() {

  if (scale.is_ready()) {
    //scale.set_scale();    
    Serial.println("Tare... remove any weights from the scale.");
    delay(5000);
    scale.tare();
    Serial.println("Tare done...");
    Serial.print("Place a known weight on the scale...");
    delay(5000);
    long reading = scale.get_units(10);
    Serial.print("Result: ");
    Serial.println(reading);
  } 
  else {
    Serial.println("HX711 not found.");
  }
  delay(1000);
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