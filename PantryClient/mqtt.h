#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>

void mqttInit();
void mqttLoop();
void processMqttMessage(String json);

#endif
