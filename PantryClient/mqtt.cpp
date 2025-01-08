#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "mqtt.h"
#include "display.h"
#include "itemManager.h"
// MQTT Configuration
#define MQTT_PORT 1883
#define MQTT_ITEMS_TOPIC "/pantry/items"
#define MQTT_REQUEST_TOPIC "/pantry/request"
const char* mqtt_server = "pi3-wifi";

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// JSON handling
DynamicJsonDocument doc(512);

// Forward Declaration
extern ItemManager itemManager;
extern DisplayManager displayManager;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0'; // Ensure null-terminated string
  String json = String((char*)payload);
  processMqttMessage(json);
}

void mqttInit() {
  client.setServer(mqtt_server, MQTT_PORT);
  client.setCallback(mqttCallback);
  while (!client.connected()) {
    if (client.connect("NodeMCUClient")) {
      client.subscribe(MQTT_ITEMS_TOPIC);
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

void processMqttMessage(String json) {
  // Deserialize JSON
  Serial.print("Recieved message: ");

  Serial.println(json);
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print("JSON Parse Error: ");
    Serial.println(error.c_str());
    return;
  }

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
