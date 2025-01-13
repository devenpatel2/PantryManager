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

  public:
    DisplayManager(uint8_t cs, uint8_t dc, uint8_t rst);

    void init();                           // Initialize the display
    void drawUI(const std::vector<std::pair<String, int>>& items); // Draw the entire UI

    bool canScrollUp();                    // Check if scrolling up is possible
    bool canScrollDown(int totalItems);    // Check if scrolling down is possible	
    void scrollUp();                       // Scroll up
    void scrollDown();                     // Scroll down
    unsigned long getLastInteractionTime();
    void setLastInteractionTime(unsigned long interactionTime);
    ScreenState getCurrentScreen();
    void setCurrentScreen(ScreenState screen);
    void drawWeatherScreen(const WeatherData& weatherData);
    void tft_print(const String& msg, int x, int y);
};

#endif
