#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <FastLED.h>

#define NUM_LEDS 10
#define DATA_PIN 6

extern CRGB leds[NUM_LEDS];

void setupLEDs();
void updateLEDs();

#endif