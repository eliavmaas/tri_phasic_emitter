#include "SoundProcessing.h"

double vReal[SAMPLES];
double vImag[SAMPLES];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

void setupSoundProcessing() {
    pinMode(AUDIO_PIN, INPUT);
}

void processSound() {
    for (int i = 0; i < SAMPLES; i++) {
        vReal[i] = analogRead(AUDIO_PIN);
        vImag[i] = 0;
        delayMicroseconds(1000);
    }
    FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.complexToMagnitude(vReal, vImag, SAMPLES);
}