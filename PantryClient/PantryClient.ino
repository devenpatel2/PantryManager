#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include "mqtt.h"
#include "display.h"
#include "buttons.h"
#include "itemManager.h"

// Display Configuration
#define TFT_CS D2
#define TFT_RST D6
#define TFT_DC D1


// Wi-Fi config
const char* ssid = "xxxxxxxxxxxx";
const char* password = "xxxxxxxxxxx";

// Pin Definitions
#define BUTTON_UP_PIN D3
#define BUTTON_DOWN_PIN D4
ItemManager itemManager;
DisplayManager displayManager(TFT_CS, TFT_DC, TFT_RST);
void setup() {
  // Initialize Serial
  Serial.begin(9600);
  // Initialize Display
  displayManager.init();

  // Initialize Buttons
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // Setup MQTT
  mqttInit();
}

void loop() {
  // Handle MQTT communication
  mqttLoop();
  // Handle button input
  handleButtons(displayManager, itemManager);
}
