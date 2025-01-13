#include "buttons.h"
#include "display.h"
#include <Arduino.h>

#include "display.h"
#include "itemManager.h"

// Button pins
#define BUTTON_UP_PIN D3
#define BUTTON_DOWN_PIN D4

// Long-press configuration
#define LONG_PRESS_THRESHOLD 500  // Time in ms to detect long press
#define SCROLL_INTERVAL 200       // Time in ms between continuous scrolls

bool buttonUpPressed = false;
bool buttonDownPressed = false;

unsigned long upButtonPressTime = 0;
unsigned long downButtonPressTime = 0;
bool upLongPressActive = false;
bool downLongPressActive = false;

void handleButtons(DisplayManager& display, ItemManager& itemManager) {
    unsigned long currentTime = millis();

    // Handle UP Button
    if (digitalRead(BUTTON_UP_PIN) == LOW) {
        if (!buttonUpPressed) {
            buttonUpPressed = true;
            upButtonPressTime = currentTime;  // Start timing the press

            // Handle short press
            if (display.canScrollUp()) {
                display.scrollUp();
                display.drawUI(itemManager.getSortedItems());
            }
        }

        // Check for long press
        if (!upLongPressActive && (currentTime - upButtonPressTime > LONG_PRESS_THRESHOLD)) {
            upLongPressActive = true;
        }

        // If long press is active, trigger continuous scrolling
        if (upLongPressActive) {
            static unsigned long lastScrollTimeUp = 0;
            if (currentTime - lastScrollTimeUp > SCROLL_INTERVAL) {
                if (display.canScrollUp()) {
                    display.scrollUp();
                    display.drawUI(itemManager.getSortedItems());
                }
                lastScrollTimeUp = currentTime;  // Update last scroll time
            }
        }
    } else {
        // Reset UP button states on release
        buttonUpPressed = false;
        upLongPressActive = false;
    }

    // Handle DOWN Button
    if (digitalRead(BUTTON_DOWN_PIN) == LOW) {
        if (!buttonDownPressed) {
            buttonDownPressed = true;
            downButtonPressTime = currentTime;  // Start timing the press

            // Handle short press
            if (display.canScrollDown(itemManager.getItems().size())) {
                display.scrollDown();
                display.drawUI(itemManager.getSortedItems());
            }
        }

        // Check for long press
        if (!downLongPressActive && (currentTime - downButtonPressTime > LONG_PRESS_THRESHOLD)) {
            downLongPressActive = true;
        }

        // If long press is active, trigger continuous scrolling
        if (downLongPressActive) {
            static unsigned long lastScrollTimeDown = 0;
            if (currentTime - lastScrollTimeDown > SCROLL_INTERVAL) {
                if (display.canScrollDown(itemManager.getItems().size())) {
                    display.scrollDown();
                    display.drawUI(itemManager.getSortedItems());
                }
                lastScrollTimeDown = currentTime;  // Update last scroll time
            }
        }
    } else {
        // Reset DOWN button states on release
        buttonDownPressed = false;
        downLongPressActive = false;
    }

    if (buttonDownPressed || buttonUpPressed) {
        display.setLastInteractionTime(millis());

        if (display.getCurrentScreen() == WEATHER_STATION) {
            display.setCurrentScreen(PANTRY_MANAGER);
            display.drawUI(itemManager.getSortedItems());
        }
    }
}
