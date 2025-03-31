#include "LEDControl.h"

CRGB leds[NUM_LEDS];

void setupLEDs() {
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(75);
}

void updateLEDs() {
    FastLED.show();
}