#include <Arduino.h>
#include <ArduinoJson.h>
#include "weatherManager.h"

void WeatherManager::updateWeatherFromMqtt(DynamicJsonDocument& doc) {

    // Update cached weather data
    cachedWeather.time = doc["data"]["time"] | "N/A";
    cachedWeather.date = doc["data"]["date"] | "N/A";
    cachedWeather.temperature = doc["data"]["temperature"] | 0.0f;
    cachedWeather.weather_code = doc["data"]["weather_code"] | -1;
    cachedWeather.precipitation = doc["data"]["precipitation"] | "";
    cachedWeather.warning = doc["data"]["warning"] | "";
    cachedWeather.sunrise = doc["data"]["sunrise"] | "--:--";
    cachedWeather.sunset = doc["data"]["sunset"] | "--:--";
}

WeatherData WeatherManager::getWeatherData() const {
    return cachedWeather;
}
