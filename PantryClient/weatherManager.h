#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

struct WeatherData {
    String time = "N/A";           // Time of the weather update
    String date = "N/A";           // Date of the weather update
    float temperature = 0.0f;      // Temperature in Celsius
    String precipitation = "";     // Precipitation description (e.g., "Rain", "Snow", or empty)
    String warning = "";           // Weather warnings (e.g., "Storm Alert" or empty)
};

class WeatherManager {
private:
    WeatherData cachedWeather;  // Holds the latest weather data

public:
    // Returns the cached weather data
    WeatherData getWeatherData() const;  
    void updateWeatherFromMqtt(DynamicJsonDocument& doc);
    
};

#endif // WEATHER_MANAGER_H
