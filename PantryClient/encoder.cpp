#include <Arduino.h>
#include <RotaryEncoder.h>
#include "encoder.h"

#define ENCODER_CLK_PIN D3
#define ENCODER_DT_PIN  D4
#define ENCODER_SW_PIN  D0

static RotaryEncoder encoder(ENCODER_CLK_PIN, ENCODER_DT_PIN, RotaryEncoder::LatchMode::FOUR3);
static long lastPosition = 0;
static int lastSwState = HIGH;

void encoderInit() {
    pinMode(ENCODER_SW_PIN, INPUT);
    lastSwState = digitalRead(ENCODER_SW_PIN);
    lastPosition = encoder.getPosition();
}

static void onScrollInput(DisplayManager& display, ItemManager& itemManager) {
    display.setLastInteractionTime(millis());
    if (display.getCurrentScreen() == WEATHER_STATION) {
        display.setCurrentScreen(PANTRY_MANAGER);
    }
    display.drawUI(itemManager.getSortedItems());
}

void handleEncoder(DisplayManager& display, ItemManager& itemManager) {
    encoder.tick();

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
        Serial.println("[encoder] select pressed");
    }
    lastSwState = swState;
}
