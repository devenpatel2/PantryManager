#include <Arduino.h>
#include <ArduinoJson.h>
#include "weatherManager.h"

void WeatherManager::updateWeatherFromMqtt(DynamicJsonDocument& doc) {

    // Update cached weather data
    cachedWeather.time = doc["data"]["time"] | "N/A";
    cachedWeather.date = doc["data"]["date"] | "N/A";
    cachedWeather.temperature = doc["data"]["temperature"] | 0.0f;
    cachedWeather.precipitation = doc["data"]["precipitation"] | "";
    cachedWeather.warning = doc["data"]["warning"] | "";
}

WeatherData WeatherManager::getWeatherData() const {
    return cachedWeather;
}
