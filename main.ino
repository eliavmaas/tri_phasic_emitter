#include "LEDControl.h"
#include "SoundProcessing.h"
#include "ModeHandler.h"

void setup() {
    Serial.begin(115200);
    setupLEDs();
    setupSoundProcessing();
    setupModes();
}

void loop() {
    processSound();
    updateLEDs();
    handleModeSwitching();
}