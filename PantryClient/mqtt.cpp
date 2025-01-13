#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "mqtt.h"
#include "display.h"
#include "itemManager.h"
#include "weatherManager.h"
// MQTT Configuration
#define MQTT_PORT 1883
#define MQTT_ITEMS_TOPIC "/pantry/items"
#define MQTT_WEATHER_TOPIC "/pantry/weather"
#define MQTT_REQUEST_TOPIC "/pantry/request"
const char* mqtt_server = "pi3-wifi";

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// JSON handling

// Forward Declaration
extern ItemManager itemManager;
extern DisplayManager displayManager;
extern WeatherManager weatherManager;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0'; // Ensure null-terminated string
    String json = String((char*)payload);
    Serial.print("Recieved message: ");
    Serial.println(json);

    DynamicJsonDocument doc(512);
    // Deserialize JSON
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
        return;
    }


    if (strcmp(topic, MQTT_ITEMS_TOPIC) == 0){
        processItemMessage(doc);
    } else if (strcmp(topic, MQTT_WEATHER_TOPIC) == 0){
        weatherManager.updateWeatherFromMqtt(doc);
        if (displayManager.getCurrentScreen() == WEATHER_STATION) {
            displayManager.drawWeatherScreen(weatherManager.getWeatherData());
        }
    }
}

void mqttInit() {
    client.setServer(mqtt_server, MQTT_PORT);
    client.setCallback(mqttCallback);
    while (!client.connected()) {
        if (client.connect("NodeMCUClient")) {
            client.subscribe(MQTT_ITEMS_TOPIC);
            client.subscribe(MQTT_WEATHER_TOPIC);
            Serial.println("Connected to broker");
        } else {
            Serial.println("Failed to connect to broker, retrying...");
            delay(5000);
        }
    }
    Serial.println("Sending request for all items...");
    String payload = "{\"op\":\"request\"}";
    client.publish(MQTT_REQUEST_TOPIC, payload.c_str());
}

void mqttLoop() {
    if (!client.connected()) {
        mqttInit();
    }
    client.loop();
}

void processItemMessage(DynamicJsonDocument& doc) {

    String op = doc["op"];
    String item = doc["item"];
    int status = doc["status"].isNull() ? -1 : doc["status"]; // Handle null status

    if (op == "add") {
        itemManager.addItem(item, status);
    } else if (op == "remove") {
        itemManager.removeItem(item);
    } else if (op == "update") {
        itemManager.updateItem(item, status);
    } else if (op == "bulk") {
        JsonObject items = doc["items"];
        for (JsonPair kv : items) {
            itemManager.addItem(kv.key().c_str(), kv.value().as<int>());
        }
    }
    displayManager.drawUI(itemManager.getSortedItems());
}
