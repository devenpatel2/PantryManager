#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <map>

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

public:
    DisplayManager(uint8_t cs, uint8_t dc, uint8_t rst);

    void init();                           // Initialize the display
    void drawUI(const std::vector<std::pair<String, int>>& items); // Draw the entire UI

    bool canScrollUp();                    // Check if scrolling up is possible
    bool canScrollDown(int totalItems);    // Check if scrolling down is possible	
    void scrollUp();                       // Scroll up
    void scrollDown();                     // Scroll down
};

#endif
