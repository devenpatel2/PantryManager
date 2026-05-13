#ifndef ENCODER_H
#define ENCODER_H

#include "display.h"
#include "itemManager.h"

void encoderInit();
void handleEncoder(DisplayManager& display, ItemManager& itemManager);

#endif
