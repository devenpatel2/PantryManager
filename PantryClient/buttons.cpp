#include "buttons.h"
#include "display.h"
#include <Arduino.h>

#include "display.h"
#include "itemManager.h"

// Button pins
#define BUTTON_UP_PIN D3
#define BUTTON_DOWN_PIN D4

bool buttonUpPressed = false;
bool buttonDownPressed = false;

// Reference to the DisplayManager instance
// extern DisplayManager display;

void handleButtons(DisplayManager display, ItemManager itemManager) {
    // Scroll up
    if (digitalRead(BUTTON_UP_PIN) == LOW && !buttonUpPressed) {
        buttonUpPressed = true;

        if (display.canScrollUp()) {
            display.scrollUp();
            display.drawUI(itemManager.getSortedItems());
        }
    } else if (digitalRead(BUTTON_UP_PIN) == HIGH) {
        buttonUpPressed = false;
    }

    // Scroll down
    if (digitalRead(BUTTON_DOWN_PIN) == LOW && !buttonDownPressed) {
        buttonDownPressed = true;

        if (display.canScrollDown(itemManager.getItems().size())) {
            display.scrollDown();
            display.drawUI(itemManager.getSortedItems());
        }
    } else if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
        buttonDownPressed = false;
    }
}
