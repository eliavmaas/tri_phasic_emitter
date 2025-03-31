#include "ModeHandler.h"

int mode = 0;

void setupModes() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void handleModeSwitching() {
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(BUTTON_PIN);
    
    if (buttonState == LOW && lastButtonState == HIGH) {
        mode = (mode + 1) % 9;
        delay(200);
    }
    lastButtonState = buttonState;
}