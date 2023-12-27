#include <Arduino.h>
#include "HX711.h"
#include <WiFiManager.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 18;

HX711 scale;

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