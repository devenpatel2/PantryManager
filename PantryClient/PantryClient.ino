#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include "mqtt.h"
#include "display.h"
#include "buttons.h"
#include "itemManager.h"
#include "weatherManager.h"
#include "wifi_credentials.h"
// Display Configuration
#define TFT_CS D2
#define TFT_RST D6
#define TFT_DC D1


// Wi-Fi config
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Pin Definitions
#define BUTTON_UP_PIN D3
#define BUTTON_DOWN_PIN D4

const unsigned long INACTIVITY_TIMEOUT = 60000; // 1 minute
ItemManager itemManager;
DisplayManager displayManager(TFT_CS, TFT_DC, TFT_RST);
WeatherManager weatherManager;

void setup() {
  // Initialize Serial
  Serial.begin(9600);
  // Initialize Display
  displayManager.init();

  // Initialize Buttons
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");
  // Setup MQTT
  mqttInit();
}

void loop() {
  // Handle MQTT communication
  mqttLoop();
  // Handle button input
  handleButtons(displayManager, itemManager);
  if (millis() - displayManager.getLastInteractionTime() > INACTIVITY_TIMEOUT && displayManager.getCurrentScreen() == PANTRY_MANAGER) {
    displayManager.setCurrentScreen(WEATHER_STATION);
    displayManager.drawWeatherScreen(weatherManager.getWeatherData());
  }
}
