#include <map>
#include "display.h"
#include "icons.h"

DisplayManager::DisplayManager(uint8_t cs, uint8_t dc, uint8_t rst)
        : tft(cs, dc, rst), currentIndex(0), firstVisibleIndex(0),
        lineY(-1), lastInteractionTime(0), currentScreen(PANTRY_MANAGER) {
    maxVisibleItems = screenHeight / rowHeight;
}

void DisplayManager::init() {
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(0);
    tft.fillScreen(ST7735_BLACK);
    tft.setTextSize(1);
}

void DisplayManager::tft_print(const String& msg, int x, int y){
    tft.setCursor(x, y);
    tft.print(msg.c_str());
}


void DisplayManager::drawUI(const std::vector<std::pair<String, int>>& items) {
    tft.fillScreen(ST7735_BLACK);  // Clear the screen

    int visibleCount = 0;
    for (int i = firstVisibleIndex; i < items.size() && visibleCount < maxVisibleItems; i++) {
        const auto& [name, status] = items[i];

        // Highlight current item
        if (i == currentIndex) {
            tft.fillRect(0, visibleCount * rowHeight, screenWidth, rowHeight, ST7735_YELLOW);
            tft.setTextColor(ST7735_BLACK);
        } else {
            tft.setTextColor(ST7735_CYAN);
        }

        tft.setCursor(5, visibleCount * rowHeight + 5);
        tft.print(name.c_str());

        // Draw Status Icon
        int iconX = screenWidth - 15;
        int iconY = visibleCount * rowHeight + 6;
        switch (status) {
            case 0:  // In Stock
                tft.drawBitmap(iconX, iconY, checkIcon, ICON_SIZE, ICON_SIZE, ST7735_GREEN);
                break;
            case 1:  // Out of Stock
                tft.drawBitmap(iconX, iconY, crossIcon, ICON_SIZE, ICON_SIZE, ST7735_RED);
                break;
            case 2:  // Surplus
                tft.drawBitmap(iconX, iconY, plusIcon, ICON_SIZE, ICON_SIZE, ST7735_BLUE);
                break;
        }

        // Draw separator line
        if (lineY == -1 && status != 1) {
            lineY = visibleCount * rowHeight - 1;
        }
        visibleCount++;
    }

    if (lineY != -1) {
        tft.drawLine(0, lineY, screenWidth, lineY, ST7735_WHITE);
    }
    lineY = -1;
}


void DisplayManager::drawWeatherScreen(const WeatherData& weatherData) {
    // Clear the screen
    String msg;
    tft.fillScreen(ST7735_BLACK);

    // Draw time and date
    tft.setTextColor(ST7735_WHITE);
    msg = "Time: " + String(weatherData.time);
    tft_print(msg, 10, 10);
    msg = "Date: " + String(weatherData.date);
    tft_print(msg, 10, 30);
    // Draw temperature
    tft.setTextColor(ST7735_CYAN);
    msg = "Temp: " + String(weatherData.temperature, 1) + "  C";
    tft_print(msg, 10, 50);
    tft.drawCircle(76, 51, 2, ST7735_WHITE);
    // Draw precipitation if available
    if (weatherData.precipitation.length() > 0) {
        tft.setTextColor(ST7735_BLUE);
        msg = "Precip: " + String(weatherData.precipitation);
        tft_print(msg, 10, 70);
    }

    // Draw warnings if available
    if (weatherData.warning.length() > 0) {
        tft.setTextColor(ST7735_RED);
        msg = "Warning: " + String(weatherData.warning);
        tft_print(msg, 10, 90);
    }
}


bool DisplayManager::canScrollUp() {
    return currentIndex > 0;
}

bool DisplayManager::canScrollDown(int totalItems) {
    return currentIndex < totalItems - 1;
}

void DisplayManager::scrollUp() {
    currentIndex--;
    if (currentIndex < firstVisibleIndex) {
        firstVisibleIndex--;
    }
}

void DisplayManager::scrollDown() {
    currentIndex++;
    if (currentIndex >= firstVisibleIndex + maxVisibleItems) {
        firstVisibleIndex++;
    }
}

unsigned long DisplayManager::getLastInteractionTime(){
    return lastInteractionTime;
}

void DisplayManager::setLastInteractionTime(unsigned long interactionTime){
    lastInteractionTime = interactionTime;
}

ScreenState DisplayManager::getCurrentScreen(){
    return currentScreen;
}

void DisplayManager::setCurrentScreen(ScreenState screen){
    currentScreen = screen;
}
