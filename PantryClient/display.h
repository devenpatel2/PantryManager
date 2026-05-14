#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <map>
#include "weatherManager.h"

enum ScreenState { PANTRY_MANAGER, WEATHER_STATION };

class DisplayManager {
  private:
    Adafruit_ST7735 tft;

    // Display constants
    const int screenWidth = 128;
    const int screenHeight = 160;
    const int rowHeight = 20;


    // State variables for scrolling
    int currentIndex;
    int firstVisibleIndex;
    int maxVisibleItems;
    int lineY;
    unsigned long lastInteractionTime;
    ScreenState currentScreen;

    // Weather screen helpers
    void drawWeatherIcon(int x, int y, int wmoCode);
    void drawSunIcon(int x, int y);
    void drawCloudIcon(int x, int y, uint16_t color);
    void drawPartlyCloudyIcon(int x, int y);
    void drawRainIcon(int x, int y);
    void drawSnowIcon(int x, int y);
    void drawFogIcon(int x, int y);
    void drawThunderIcon(int x, int y);
    const char* conditionLabel(int wmoCode);
    uint16_t conditionColor(int wmoCode);
    uint16_t tempColor(float t);

  public:
    DisplayManager(uint8_t cs, uint8_t dc, uint8_t rst);

    void init();                           // Initialize the display
    void drawUI(const std::vector<std::pair<String, int>>& items); // Draw the entire UI

    bool canScrollUp();                    // Check if scrolling up is possible
    bool canScrollDown(int totalItems);    // Check if scrolling down is possible	
    void scrollUp();                       // Scroll up
    void scrollDown();                     // Scroll down
    int getCurrentIndex();                 // Highlighted item index (in current sorted view)
    void highlightItemByName(const std::vector<std::pair<String, int>>& items, const String& name);
    unsigned long getLastInteractionTime();
    void setLastInteractionTime(unsigned long interactionTime);
    ScreenState getCurrentScreen();
    void setCurrentScreen(ScreenState screen);
    void drawWeatherScreen(const WeatherData& weatherData);
    void tft_print(const String& msg, int x, int y);
};

#endif
