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
    int visibleCount = 0;
    for (int i = firstVisibleIndex; i < items.size() && visibleCount < maxVisibleItems; i++) {
        const auto& [name, status] = items[i];

        bool highlighted = (i == currentIndex);
        uint16_t bg = highlighted ? ST7735_YELLOW : ST7735_BLACK;
        tft.fillRect(0, visibleCount * rowHeight, screenWidth, rowHeight, bg);
        tft.setTextColor(highlighted ? ST7735_BLACK : ST7735_CYAN);

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


void DisplayManager::drawWeatherScreen(const WeatherData& w) {
    const uint16_t GRAY = tft.color565(60, 60, 60);
    const uint16_t ORANGE = tft.color565(255, 140, 0);

    tft.fillScreen(ST7735_BLACK);

    // Header: date (small) over time (large), both centered
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    int dateW = w.date.length() * 6;
    tft_print(w.date, (screenWidth - dateW) / 2, 8);

    tft.setTextSize(2);
    tft.setTextColor(ST7735_CYAN);
    int timeW = w.time.length() * 12;
    tft_print(w.time, (screenWidth - timeW) / 2, 22);
    tft.setTextSize(1);

    tft.drawLine(0, 44, screenWidth, 44, GRAY);

    // Weather row: 32x32 icon on the left, big temperature on the right
    drawWeatherIcon(8, 52, w.weather_code);

    tft.setTextSize(2);
    uint16_t tColor = tempColor(w.temperature);
    tft.setTextColor(tColor);
    String tempStr = String(w.temperature, 1);
    int tempX = 50;
    int tempY = 62;
    tft_print(tempStr, tempX, tempY);
    int degX = tempX + tempStr.length() * 12 + 2;
    tft.drawCircle(degX, tempY + 2, 2, tColor);
    tft.setTextSize(1);

    // Centered condition label
    tft.setTextColor(conditionColor(w.weather_code));
    const char* label = conditionLabel(w.weather_code);
    int labelW = strlen(label) * 6;
    tft_print(label, (screenWidth - labelW) / 2, 92);

    tft.drawLine(0, 108, screenWidth, 108, GRAY);

    // Footer: sunrise (yellow up-triangle) and sunset (orange down-triangle)
    int row1 = 118;
    int row2 = 134;

    tft.fillTriangle(8, row1 + 7, 16, row1 + 7, 12, row1 + 1, ST7735_YELLOW);
    tft.setTextColor(ST7735_YELLOW);
    tft_print("Sunrise " + w.sunrise, 22, row1);

    tft.fillTriangle(8, row2 + 1, 16, row2 + 1, 12, row2 + 7, ORANGE);
    tft.setTextColor(ORANGE);
    tft_print("Sunset  " + w.sunset, 22, row2);
}

// ---- Weather icon helpers (all draw inside a 32x32 box anchored at x,y) ----

void DisplayManager::drawWeatherIcon(int x, int y, int code) {
    if (code == 0) {
        drawSunIcon(x, y);
    } else if (code == 1 || code == 2) {
        drawPartlyCloudyIcon(x, y);
    } else if (code == 3) {
        drawCloudIcon(x, y + 6, ST7735_WHITE);
    } else if (code == 45 || code == 48) {
        drawFogIcon(x, y);
    } else if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) {
        drawRainIcon(x, y);
    } else if ((code >= 71 && code <= 77) || code == 85 || code == 86) {
        drawSnowIcon(x, y);
    } else if (code >= 95 && code <= 99) {
        drawThunderIcon(x, y);
    } else {
        drawCloudIcon(x, y + 6, tft.color565(120, 120, 120));
    }
}

void DisplayManager::drawSunIcon(int x, int y) {
    int cx = x + 16, cy = y + 16;
    tft.fillCircle(cx, cy, 6, ST7735_YELLOW);
    tft.drawLine(cx + 9,  cy,      cx + 14, cy,      ST7735_YELLOW);
    tft.drawLine(cx - 9,  cy,      cx - 14, cy,      ST7735_YELLOW);
    tft.drawLine(cx,      cy + 9,  cx,      cy + 14, ST7735_YELLOW);
    tft.drawLine(cx,      cy - 9,  cx,      cy - 14, ST7735_YELLOW);
    tft.drawLine(cx + 6,  cy + 6,  cx + 10, cy + 10, ST7735_YELLOW);
    tft.drawLine(cx - 6,  cy - 6,  cx - 10, cy - 10, ST7735_YELLOW);
    tft.drawLine(cx + 6,  cy - 6,  cx + 10, cy - 10, ST7735_YELLOW);
    tft.drawLine(cx - 6,  cy + 6,  cx - 10, cy + 10, ST7735_YELLOW);
}

// (x, y) is the top-left of the cloud's 28x20 bounding box.
void DisplayManager::drawCloudIcon(int x, int y, uint16_t color) {
    tft.fillCircle(x + 16, y + 7,  7, color);
    tft.fillCircle(x + 10, y + 13, 6, color);
    tft.fillCircle(x + 22, y + 13, 6, color);
    tft.fillRect(x + 10, y + 13, 13, 7, color);
}

void DisplayManager::drawPartlyCloudyIcon(int x, int y) {
    // Small sun in upper-left
    int cx = x + 8, cy = y + 10;
    tft.fillCircle(cx, cy, 4, ST7735_YELLOW);
    tft.drawLine(cx,     cy - 6, cx,     cy - 8, ST7735_YELLOW);
    tft.drawLine(cx - 6, cy,     cx - 8, cy,     ST7735_YELLOW);
    tft.drawLine(cx - 4, cy - 4, cx - 6, cy - 6, ST7735_YELLOW);
    tft.drawLine(cx + 4, cy - 4, cx + 6, cy - 6, ST7735_YELLOW);
    // Cloud overlapping in the lower portion
    drawCloudIcon(x + 2, y + 12, ST7735_WHITE);
}

void DisplayManager::drawRainIcon(int x, int y) {
    uint16_t gray = tft.color565(150, 150, 150);
    drawCloudIcon(x, y + 2, gray);
    // 3 diagonal raindrops below the cloud (cloud bottom ~y+22)
    tft.drawLine(x + 10, y + 25, x + 8,  y + 30, ST7735_BLUE);
    tft.drawLine(x + 16, y + 25, x + 14, y + 30, ST7735_BLUE);
    tft.drawLine(x + 22, y + 25, x + 20, y + 30, ST7735_BLUE);
}

void DisplayManager::drawSnowIcon(int x, int y) {
    uint16_t gray = tft.color565(150, 150, 150);
    drawCloudIcon(x, y + 2, gray);
    int sy = y + 28;
    int sxs[3] = { x + 10, x + 16, x + 22 };
    for (int i = 0; i < 3; i++) {
        int sx = sxs[i];
        tft.drawLine(sx - 2, sy,     sx + 2, sy,     ST7735_WHITE);
        tft.drawLine(sx,     sy - 2, sx,     sy + 2, ST7735_WHITE);
    }
}

void DisplayManager::drawFogIcon(int x, int y) {
    uint16_t gray = tft.color565(150, 150, 150);
    tft.drawLine(x + 4, y + 10, x + 28, y + 10, ST7735_WHITE);
    tft.drawLine(x + 2, y + 16, x + 26, y + 16, gray);
    tft.drawLine(x + 5, y + 22, x + 30, y + 22, ST7735_WHITE);
    tft.drawLine(x + 3, y + 28, x + 27, y + 28, gray);
}

void DisplayManager::drawThunderIcon(int x, int y) {
    uint16_t gray = tft.color565(150, 150, 150);
    drawCloudIcon(x, y + 2, gray);
    // Yellow lightning zigzag (drawn twice with 1px offset to thicken)
    tft.drawLine(x + 18, y + 22, x + 13, y + 28, ST7735_YELLOW);
    tft.drawLine(x + 13, y + 28, x + 17, y + 28, ST7735_YELLOW);
    tft.drawLine(x + 17, y + 28, x + 13, y + 31, ST7735_YELLOW);
    tft.drawLine(x + 19, y + 22, x + 14, y + 28, ST7735_YELLOW);
}

const char* DisplayManager::conditionLabel(int code) {
    if (code == 0) return "Sunny";
    if (code == 1 || code == 2) return "Partly Cloudy";
    if (code == 3) return "Cloudy";
    if (code == 45 || code == 48) return "Fog";
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return "Rain";
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) return "Snow";
    if (code >= 95 && code <= 99) return "Storm";
    return "--";
}

uint16_t DisplayManager::conditionColor(int code) {
    if (code == 0) return ST7735_YELLOW;
    if (code == 1 || code == 2) return ST7735_YELLOW;
    if (code == 3) return ST7735_WHITE;
    if (code == 45 || code == 48) return tft.color565(180, 180, 180);
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return ST7735_BLUE;
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) return ST7735_WHITE;
    if (code >= 95 && code <= 99) return ST7735_YELLOW;
    return tft.color565(150, 150, 150);
}

uint16_t DisplayManager::tempColor(float t) {
    if (t > 25.0f) return ST7735_RED;
    if (t > 15.0f) return tft.color565(255, 140, 0);
    if (t > 0.0f)  return ST7735_CYAN;
    return ST7735_BLUE;
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

int DisplayManager::getCurrentIndex(){
    return currentIndex;
}

void DisplayManager::highlightItemByName(const std::vector<std::pair<String, int>>& items, const String& name) {
    for (int i = 0; i < (int)items.size(); i++) {
        if (items[i].first == name) {
            currentIndex = i;
            if (currentIndex < firstVisibleIndex) {
                firstVisibleIndex = currentIndex;
            } else if (currentIndex >= firstVisibleIndex + maxVisibleItems) {
                firstVisibleIndex = currentIndex - maxVisibleItems + 1;
            }
            return;
        }
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
