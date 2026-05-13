#include <Arduino.h>
#include <RotaryEncoder.h>
#include "encoder.h"
#include "mqtt.h"

#define ENCODER_CLK_PIN D3
#define ENCODER_DT_PIN  D4
#define ENCODER_SW_PIN  D0

static RotaryEncoder encoder(ENCODER_CLK_PIN, ENCODER_DT_PIN, RotaryEncoder::LatchMode::FOUR3);
static long lastPosition = 0;
static int lastSwState = HIGH;

static IRAM_ATTR void encoderISR() {
    encoder.tick();
}

void encoderInit() {
    pinMode(ENCODER_SW_PIN, INPUT);
    lastSwState = digitalRead(ENCODER_SW_PIN);
    lastPosition = encoder.getPosition();

    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_DT_PIN), encoderISR, CHANGE);
}

static void onScrollInput(DisplayManager& display, ItemManager& itemManager) {
    display.setLastInteractionTime(millis());
    if (display.getCurrentScreen() == WEATHER_STATION) {
        display.setCurrentScreen(PANTRY_MANAGER);
    }
    display.drawUI(itemManager.getSortedItems());
}

void handleEncoder(DisplayManager& display, ItemManager& itemManager) {
    long pos = encoder.getPosition();
    if (pos != lastPosition) {
        long delta = pos - lastPosition;
        lastPosition = pos;

        for (long i = 0; i < delta; i++) {
            if (display.canScrollDown(itemManager.getItems().size())) {
                display.scrollDown();
            }
        }
        for (long i = 0; i < -delta; i++) {
            if (display.canScrollUp()) {
                display.scrollUp();
            }
        }
        onScrollInput(display, itemManager);
    }

    int swState = digitalRead(ENCODER_SW_PIN);
    if (lastSwState == HIGH && swState == LOW) {
        static unsigned long lastPressMs = 0;
        unsigned long now = millis();
        if (now - lastPressMs >= 50) {
            lastPressMs = now;

            if (display.getCurrentScreen() == WEATHER_STATION) {
                display.setCurrentScreen(PANTRY_MANAGER);
                display.setLastInteractionTime(now);
                display.drawUI(itemManager.getSortedItems());
            } else {
                auto items = itemManager.getSortedItems();
                int idx = display.getCurrentIndex();
                if (idx >= 0 && idx < (int)items.size()) {
                    const String& name = items[idx].first;
                    int newStatus = (items[idx].second + 1) % 3;
                    itemManager.updateItem(name, newStatus);
                    display.setLastInteractionTime(now);
                    display.drawUI(itemManager.getSortedItems());
                    publishItemUpdate(name, newStatus);
                }
            }
        }
    }
    lastSwState = swState;
}
