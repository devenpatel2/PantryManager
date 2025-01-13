#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

void mqttInit();
void mqttLoop();
void processItemMessage(DynamicJsonDocument& doc);

#endif
